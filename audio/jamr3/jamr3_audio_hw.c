/*
 * Copyright (C) 2015 Texas Instruments
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "jamr3_audio_hw"
//#define LOG_NDEBUG 0
//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#include <audio_utils/resampler.h>
#include <audio_route/audio_route.h>
#include <system/audio.h>
#include <hardware/hardware.h>
#include <hardware/audio.h>
#include <hardware/audio_effect.h>

#include <tinyalsa/asoundlib.h>

#define UNUSED(x) (void)(x)

/* yet another definition of ARRAY_SIZE macro) */
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))

/*
 * additional space in resampler buffer allowing for extra samples to be returned
 * by speex resampler when sample rates ratio is not an integer
 */
#define RESAMPLER_HEADROOM_FRAMES   10

/* buffer_remix: functor for doing in-place buffer manipulations.
 *
 * NB. When remix_func is called, the memory at `buf` must be at least
 * as large as frames * sample_size * MAX(in_chans, out_chans).
 */
struct buffer_remix {
    void (*remix_func)(struct buffer_remix *data, void *buf, size_t frames);
    size_t sample_size; /* size of one audio sample, in bytes */
    size_t in_chans;    /* number of input channels */
    size_t out_chans;   /* number of output channels */
};

struct j6_audio_device {
    struct audio_hw_device device;
    struct j6_stream_in *in;
    struct audio_route *route;
    audio_devices_t in_device;
    pthread_mutex_t lock;
    unsigned int card;
    unsigned int in_port;
    bool mic_mute;
};

struct j6_stream_in {
    struct audio_stream_in stream;
    struct j6_audio_device *dev;
    struct pcm_config config;
    struct pcm *pcm;
    struct buffer_remix *remix; /* adapt hw chan count to client */
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t frames_in;
    size_t hw_frame_size;
    unsigned int requested_rate;
    unsigned int requested_channels;
    int read_status;
    pthread_mutex_t lock;
    bool standby;
    int read; /* total frames read, cleared when entering standby */
};

static const char *supported_cards[] = {
    "DRA7xx-JAMR3",
};

#define MAX_CARD_COUNT                  10

#define SUPPORTED_IN_DEVICES            AUDIO_DEVICE_IN_LINE

#define CAPTURE_SAMPLE_RATE             44100
#define CAPTURE_PERIOD_SIZE             256
#define CAPTURE_PERIOD_COUNT            4
#define CAPTURE_BUFFER_SIZE             (CAPTURE_PERIOD_SIZE * CAPTURE_PERIOD_COUNT)

#define MIXER_XML_PATH                  "/system/etc/jamr3_mixer_paths.xml"

struct pcm_config pcm_config_capture = {
    .channels        = 2,
    .rate            = CAPTURE_SAMPLE_RATE,
    .format          = PCM_FORMAT_S16_LE,
    .period_size     = CAPTURE_PERIOD_SIZE,
    .period_count    = CAPTURE_PERIOD_COUNT,
    .start_threshold = 1,
    .stop_threshold  = CAPTURE_BUFFER_SIZE,
};

static int find_card_index(const char *supported_cards[], int num_supported)
{
    struct mixer *mixer;
    const char *name;
    int card = 0;
    int found = 0;
    int i;

    do {
        /* returns an error after last valid card */
        mixer = mixer_open(card);
        if (!mixer)
            break;

        name = mixer_get_name(mixer);

        for (i = 0; i < num_supported; ++i) {
            if (supported_cards[i] && !strcmp(name, supported_cards[i])) {
                ALOGV("Supported card '%s' found at %d", name, card);
                found = 1;
                break;
            }
        }

        mixer_close(mixer);
    } while (!found && (card++ < MAX_CARD_COUNT));

    /* Use default card number if not found */
    if (!found)
        card = 2;

    return card;
}

/* must be called with device lock held */
static void select_input_device(struct j6_audio_device *adev)
{
    if (adev->in_device & ~SUPPORTED_IN_DEVICES)
        ALOGW("select_input_device() device not supported, will use default device");
}

static size_t get_input_buffer_size(uint32_t sample_rate, int format, int channel_count)
{
    size_t size;

    /*
     * take resampling into account and return the closest majoring
     * multiple of 16 frames, as audioflinger expects audio buffers to
     * be a multiple of 16 frames
     */
    size = (pcm_config_capture.period_size * sample_rate) / pcm_config_capture.rate;
    size = ((size + 15) / 16) * 16;

    return size * channel_count * audio_bytes_per_sample(format);
}

/*
 * Implementation of buffer_remix::remix_func that removes
 * channels in place without doing any other processing.  The
 * extra channels are truncated.
 */
static void remove_channels_from_buf(struct buffer_remix *data, void *buf, size_t frames)
{
    size_t samp_size, in_frame, out_frame;
    size_t N, c;
    char *s, *d;

    ALOGVV("remove_channels_from_buf() remix=%p buf=%p frames=%u",
           data, buf, frames);

    if (frames == 0)
        return;

    samp_size = data->sample_size;
    in_frame = data->in_chans * samp_size;
    out_frame = data->out_chans * samp_size;

    if (out_frame >= in_frame) {
        ALOGE("BUG: remove_channels_from_buf() can not add channels to a buffer.\n");
        return;
    }

    N = frames - 1;
    d = (char*)buf + out_frame;
    s = (char*)buf + in_frame;

    /* take the first several channels and truncate the rest */
    while (N--) {
        for (c = 0; c < out_frame; ++c)
            d[c] = s[c];
        d += out_frame;
        s += in_frame;
    }
}

static int setup_stereo_to_mono_input_remix(struct j6_stream_in *in)
{
    ALOGV("setup_stereo_to_mono_input_remix() stream=%p", in);

    struct buffer_remix *br = (struct buffer_remix *)calloc(1, sizeof(struct buffer_remix));
    if (!br)
        return -ENOMEM;

    br->remix_func = remove_channels_from_buf;
    br->sample_size = sizeof(int16_t);
    br->in_chans = 2;
    br->out_chans = 1;
    in->remix = br;

    return 0;
}

/*
 * Implementation of buffer_remix::remix_func that duplicates the first
 * channel into the rest of channels in the frame without doing any other
 * processing
 */
static void duplicate_channels_from_mono(struct buffer_remix *data, void *buf, size_t frames)
{
    int samp_size, in_frame, out_frame;
    int N, c;
    char *s, *d;

    ALOGVV("duplicate_channels_from_mono() remix=%p buf=%p frames=%u",
           data, buf, frames);

    if (frames == 0)
        return;

    samp_size = data->sample_size;
    in_frame = data->in_chans * samp_size;
    out_frame = data->out_chans * samp_size;

    if (in_frame >= out_frame) {
        ALOGE("BUG: duplicate_channels_from_mono() can not drop channels\n");
        return;
    }

    N = frames - 1;
    d = (char*)buf + N * out_frame;
    s = (char*)buf + N * in_frame;

    /* duplicate first channel into the rest of channels in the frame */
    while (N-- >= 0) {
        for (c = 0; c < out_frame; ++c)
            d[c] = s[c % in_frame];
        d -= out_frame;
        s -= in_frame;
    }
}

/** audio_stream_in implementation **/
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    const struct j6_stream_in *in = (const struct j6_stream_in *)(stream);

    ALOGVV("in_get_sample_rate() stream=%p rate=%u", stream, in->requested_rate);

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    ALOGV("in_set_sample_rate() stream=%p rate=%u", stream, rate);

    return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    const struct j6_stream_in *in = (const struct j6_stream_in *)(stream);

    size_t bytes = get_input_buffer_size(in->requested_rate,
                                         AUDIO_FORMAT_PCM_16_BIT,
                                         in->requested_channels);

    ALOGVV("in_get_buffer_size() stream=%p bytes=%u", in, bytes);

    return bytes;
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream)
{
    const struct j6_stream_in *in = (const struct j6_stream_in *)(stream);
    audio_channel_mask_t channels = audio_channel_in_mask_from_count(in->requested_channels);

    ALOGVV("in_get_channels() stream=%p channels=%u", in, in->requested_channels);

    return channels;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    audio_format_t format = AUDIO_FORMAT_PCM_16_BIT;

    UNUSED(stream);
    ALOGVV("in_set_format() stream=%p format=0x%08x (%u bits/sample)",
           stream, format, audio_bytes_per_sample(format) << 3);

    return format;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    UNUSED(stream);
    ALOGV("in_set_format() stream=%p format=0x%08x (%u bits/sample)",
          stream, format, audio_bytes_per_sample(format) << 3);

    if (format != AUDIO_FORMAT_PCM_16_BIT) {
        return -ENOSYS;
    } else {
        return 0;
    }
}

/* must be called with locks held */
static void do_in_standby(struct j6_stream_in *in)
{
    struct j6_audio_device *adev = in->dev;

    if (!in->standby) {
        ALOGI("do_in_standby() close card %u port %u", adev->card, adev->in_port);
        pcm_close(in->pcm);
        in->pcm = NULL;
        in->standby = true;
    }
}

static int in_standby(struct audio_stream *stream)
{
    struct j6_stream_in *in = (struct j6_stream_in *)(stream);
    struct j6_audio_device *adev = in->dev;

    ALOGV("in_standby() stream=%p", in);
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    do_in_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&adev->lock);

    return 0;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    UNUSED(stream);
    UNUSED(fd);

    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct j6_stream_in *in = (struct j6_stream_in *)(stream);
    struct j6_audio_device *adev = in->dev;
    struct str_parms *parms;
    char value[32];
    int ret;
    uint32_t val = 0;

    ALOGV("in_set_parameters() stream=%p parameter='%s'", stream, kvpairs);

    parms = str_parms_create_str(kvpairs);

    /* Nothing to do for AUDIO_PARAMETER_STREAM_INPUT_SOURCE, so it's ignored */

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING, value, sizeof(value));
    if (ret >= 0) {
        val = atoi(value);
        pthread_mutex_lock(&adev->lock);
        pthread_mutex_lock(&in->lock);
        if (val != 0) {
            if ((adev->in_device & AUDIO_DEVICE_IN_ALL) != val)
                do_in_standby(in);

            /* set the active input device */
            adev->in_device = val;
            select_input_device(adev);
        }
        pthread_mutex_unlock(&in->lock);
        pthread_mutex_unlock(&adev->lock);
    }

    return 0;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    UNUSED(stream);
    UNUSED(keys);

    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    UNUSED(stream);
    UNUSED(gain);

    return 0;
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                           struct resampler_buffer* buffer)
{
    struct j6_stream_in *in;
    struct buffer_remix *remix;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct j6_stream_in *)((char *)buffer_provider -
                                 offsetof(struct j6_stream_in, buf_provider));

    if (in->pcm == NULL) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
        return -ENODEV;
    }

    if (in->frames_in == 0) {
        in->read_status = pcm_read(in->pcm,
                                   (void*)in->buffer,
                                   buffer->frame_count * in->hw_frame_size);
        if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d", in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->frames_in = buffer->frame_count;

        remix = in->remix;
        if (remix)
            remix->remix_func(remix, in->buffer, in->frames_in);
    }

    buffer->frame_count = (buffer->frame_count > in->frames_in) ?
                                in->frames_in : buffer->frame_count;
    buffer->i16 = in->buffer;

    return in->read_status;
}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer)
{
    struct j6_stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct j6_stream_in *)((char *)buffer_provider -
                                 offsetof(struct j6_stream_in, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/*
 * read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified
 */
static ssize_t read_frames(struct j6_stream_in *in, void *buffer, ssize_t frames)
{
    const struct audio_stream_in *s = (const struct audio_stream_in *)in;
    ssize_t frames_wr = 0;
    size_t frame_size;

    ALOGVV("read_frames() stream=%p frames=%u", in, frames);

    if (in->remix)
        frame_size = audio_stream_in_frame_size(s);
    else
        frame_size = in->hw_frame_size;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;

        in->resampler->resample_from_provider(in->resampler,
                    (int16_t *)((char *)buffer + frames_wr * frame_size),
                    &frames_rd);
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }

    return frames_wr;
}

static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
    const struct audio_stream_in *s = (const struct audio_stream_in *)stream;
    struct j6_stream_in *in = (struct j6_stream_in *)(stream);
    struct j6_audio_device *adev = in->dev;
    const size_t frame_size = audio_stream_in_frame_size(stream);
    const size_t frames = bytes / frame_size;
    uint32_t rate = in_get_sample_rate(&stream->common);
    uint32_t read_usecs = frames * 1000000 / rate;
    int ret;

    ALOGVV("in_read() stream=%p buffer=%p size=%u/%u time=%u usecs",
           stream, buffer, frames, rate, read_usecs);

    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);

    if (in->standby) {
        select_input_device(adev);

        ALOGI("in_read() open card %u port %u", adev->card, adev->in_port);
        in->pcm = pcm_open(adev->card, adev->in_port,
                           PCM_IN | PCM_MONOTONIC,
                           &in->config);
        if (!pcm_is_ready(in->pcm)) {
            ALOGE("in_read() failed to open pcm in: %s", pcm_get_error(in->pcm));
            pcm_close(in->pcm);
            in->pcm = NULL;
            usleep(read_usecs); /* limits the rate of error messages */
            pthread_mutex_unlock(&in->lock);
            pthread_mutex_unlock(&adev->lock);
            return -ENODEV;
        }

        /* if no supported sample rate is available, use the resampler */
        if (in->resampler) {
            in->resampler->reset(in->resampler);
            in->frames_in = 0;
        }

        in->standby = false;
        in->read = 0;
    }

    pthread_mutex_unlock(&adev->lock);

    if (in->resampler || in->remix)
        ret = read_frames(in, buffer, frames);
    else
        ret = pcm_read(in->pcm, buffer, bytes);

    if (ret < 0) {
        ALOGE("in_read() failed to read audio data %d", ret);
        usleep(read_usecs); /* limits the rate of error messages */
        memset(buffer, 0, bytes);
    } else {
        in->read += frames;
        if (adev->mic_mute)
            memset(buffer, 0, bytes);
    }

    pthread_mutex_unlock(&in->lock);

    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    UNUSED(stream);
    ALOGVV("in_get_input_frames_lost() stream=%p frames=%u", stream, 0);

    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    UNUSED(stream);
    UNUSED(effect);

    return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    UNUSED(stream);
    UNUSED(effect);

    return 0;
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out,
                                   const char *address)
{
    UNUSED(dev);
    UNUSED(handle);
    UNUSED(devices);
    UNUSED(flags);
    UNUSED(config);
    UNUSED(stream_out);
    UNUSED(address);

    ALOGE("adev_open_output_stream() output stream is not supported");

    return -ENOSYS;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    UNUSED(dev);
    UNUSED(stream);

    ALOGE("adev_close_output_stream() output stream is not supported");
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    UNUSED(dev);
    UNUSED(kvpairs);

    return -ENOSYS;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
    UNUSED(dev);
    UNUSED(keys);

    return strdup("");;
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    UNUSED(dev);

    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int adev_get_master_volume(struct audio_hw_device *dev, float *volume)
{
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int adev_set_master_mute(struct audio_hw_device *dev, bool muted)
{
    UNUSED(dev);
    UNUSED(muted);

    return -ENOSYS;
}

static int adev_get_master_mute(struct audio_hw_device *dev, bool *muted)
{
    UNUSED(dev);
    UNUSED(muted);

    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    UNUSED(dev);
    UNUSED(mode);

    ALOGV("adev_set_mode() mode=0x%08x", mode);

    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    struct j6_audio_device *adev = (struct j6_audio_device *)dev;

    ALOGV("adev_set_mic_mute() state=%s", state ? "mute" : "unmute");
    adev->mic_mute = state;

    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    const struct j6_audio_device *adev = (const struct j6_audio_device *)dev;

    *state = adev->mic_mute;
    ALOGV("adev_get_mic_mute() state=%s", *state ? "mute" : "unmute");

    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    UNUSED(dev);

    size_t bytes = get_input_buffer_size(config->sample_rate,
                                        config->format,
                                        popcount(config->channel_mask));

    ALOGVV("adev_in_get_buffer_size() bytes=%u", bytes);

    return bytes;
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in,
                                  audio_input_flags_t flags,
                                  const char *address,
                                  audio_source_t source)
{
    struct j6_audio_device *adev = (struct j6_audio_device *)dev;
    struct j6_stream_in *in;
    int ret;

    UNUSED(handle);
    UNUSED(devices);
    UNUSED(flags);
    UNUSED(address);
    UNUSED(source);

    in = (struct j6_stream_in *)calloc(1, sizeof(struct j6_stream_in));
    if (!in)
        return -ENOMEM;

    ALOGI("adev_open_input_stream() stream=%p rate=%u channels=%u format=0x%08x",
          in, config->sample_rate, popcount(config->channel_mask), config->format);

    pthread_mutex_init(&in->lock, NULL);

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    in->dev = adev;
    in->standby = true;
    in->config = pcm_config_capture;
    in->requested_rate = config->sample_rate;
    in->requested_channels = popcount(config->channel_mask);
    in->hw_frame_size = in->config.channels * sizeof(int16_t);
    in->remix = NULL;
    in->resampler = NULL;
    in->buffer = NULL;
    adev->in = in;

    /* in-place stereo-to-mono remix since capture stream is stereo */
    if (in->requested_channels == 1) {
        ALOGV("adev_open_input_stream() stereo-to-mono remix needed");

        /*
         * buffer size is already enough to allow stereo-to-mono remix
         * and resample if needed
         */
        in->buffer = malloc(2 * in->config.period_size * in->hw_frame_size);
        if (!in->buffer) {
            ret = -ENOMEM;
            goto err1;
        }

        ret = setup_stereo_to_mono_input_remix(in);
        if (ret) {
            ALOGE("adev_open_input_stream() failed to setup remix %d", ret);
            goto err2;
        }
    }

    if (in->requested_rate != in->config.rate) {
        ALOGV("adev_open_input_stream() resample needed, req=%uHz got=%uHz",
              in->requested_rate, in->config.rate);

        in->buf_provider.get_next_buffer = get_next_buffer;
        in->buf_provider.release_buffer = release_buffer;
        ret = create_resampler(in->config.rate,
                               in->requested_rate,
                               in->requested_channels,
                               RESAMPLER_QUALITY_DEFAULT,
                               &in->buf_provider,
                               &in->resampler);
        if (ret) {
            ALOGE("adev_open_input_stream() failed to create resampler %d", ret);
            goto err3;
        }
    }

    *stream_in = &in->stream;

    return 0;

 err3:
    free(in->remix);
 err2:
    free(in->buffer);
 err1:
    free(in);
    return ret;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    struct j6_audio_device *adev = (struct j6_audio_device *)dev;
    struct j6_stream_in *in = (struct j6_stream_in *)(stream);

    ALOGV("adev_close_input_stream() stream=%p", stream);

    in_standby(&stream->common);

    if (in->resampler)
        release_resampler(in->resampler);
    in->resampler = NULL;

    if (in->remix)
        free(in->remix);
    in->remix = NULL;

    in->dev = NULL;
    adev->in = NULL;

    free(in->buffer);
    free(in);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    UNUSED(device);
    UNUSED(fd);

    return 0;
}

static int adev_close(hw_device_t *device)
{
    struct j6_audio_device *adev = (struct j6_audio_device *)device;

    ALOGI("adev_close()");

    audio_route_free(adev->route);
    free(device);

    return 0;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    struct j6_audio_device *adev;

    ALOGI("adev_open() %s", name);

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = (struct j6_audio_device*)calloc(1, sizeof(struct j6_audio_device));
    if (!adev)
        return -ENOMEM;

    pthread_mutex_init(&adev->lock, NULL);

    adev->device.common.tag = HARDWARE_DEVICE_TAG;
    adev->device.common.version = AUDIO_DEVICE_API_VERSION_2_0;
    adev->device.common.module = (struct hw_module_t *) module;
    adev->device.common.close = adev_close;

    adev->device.init_check = adev_init_check;
    adev->device.set_voice_volume = adev_set_voice_volume;
    adev->device.set_master_volume = adev_set_master_volume;
    adev->device.get_master_volume = adev_get_master_volume;
    adev->device.set_master_mute = adev_set_master_mute;
    adev->device.get_master_mute = adev_get_master_mute;
    adev->device.set_mode = adev_set_mode;
    adev->device.set_mic_mute = adev_set_mic_mute;
    adev->device.get_mic_mute = adev_get_mic_mute;
    adev->device.set_parameters = adev_set_parameters;
    adev->device.get_parameters = adev_get_parameters;
    adev->device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->device.open_output_stream = adev_open_output_stream;
    adev->device.close_output_stream = adev_close_output_stream;
    adev->device.open_input_stream = adev_open_input_stream;
    adev->device.close_input_stream = adev_close_input_stream;
    adev->device.dump = adev_dump;

    adev->in_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
    adev->card = find_card_index(supported_cards,
                                 ARRAY_SIZE(supported_cards));
    adev->in_port = 0;
    ALOGI("JAMR3 card is hw:%d\n", adev->card);

    adev->mic_mute = false;

    adev->route = audio_route_init(adev->card, MIXER_XML_PATH);
    if (!adev->route) {
        ALOGE("Unable to initialize audio routes");
        free(adev);
        return -EINVAL;
    }

    *device = &adev->device.common;

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "Jacinto6 JAMR3 Audio HAL",
        .author = "Texas Instruments Inc.",
        .methods = &hal_module_methods,
    },
};
