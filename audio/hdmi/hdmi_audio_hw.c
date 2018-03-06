/* -*- mode: C; c-file-style: "stroustrup"; indent-tabs-mode: nil; -*- */
/*
 * Copyright (C) 2012 Texas Instruments
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

#define LOG_TAG "hdmi_audio_hw"
/* #define LOG_NDEBUG 0 */
/* #define LOG_TRACE_FUNCTION */

#ifndef LOG_TRACE_FUNCTION
#define TRACE() ((void)0)
#define TRACEM(fmt, ...) ((void)0)
#else
#define tfmt(x) x
#define TRACE() (ALOGV("%s() %s:%d", __func__, __FILE__, __LINE__))
#define TRACEM(fmt, ...) (ALOGV("%s() " tfmt(fmt) " %s:%d", __func__, ##__VA_ARGS__, __FILE__, __LINE__))
#endif

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

#include <cutils/log.h>
#include <cutils/str_parms.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#ifdef PRIMARY_HDMI_AUDIO_HAL
#include <audio_utils/resampler.h>
#include <audio_route/audio_route.h>
#endif

#include <tinyalsa/asoundlib.h>

#include <OMX_Audio.h>

#include "hdmi_audio_hal.h"

#define UNUSED(x) (void)(x)

/* yet another definition of ARRAY_SIZE macro) */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define MAX_CARD_COUNT 10
#define HDMI_PCM_DEV 0
#define HDMI_SAMPLING_RATE 44100
#define HDMI_PERIOD_SIZE 1920
#define HDMI_PERIOD_COUNT 4
#define HDMI_MAX_CHANNELS 8

typedef struct _hdmi_device {
    audio_hw_device_t device;
    int map[HDMI_MAX_CHANNELS];
    bool CEAMap;
#ifdef PRIMARY_HDMI_AUDIO_HAL
    struct audio_route *route;
    struct audio_route *jamr_route;
    unsigned int card;
    unsigned int jamr_card;
    bool has_media;
    bool has_jamr;
    bool mic_mute;
#endif
} hdmi_device_t;

int cea_channel_map[HDMI_MAX_CHANNELS] = {OMX_AUDIO_ChannelLF,OMX_AUDIO_ChannelRF,OMX_AUDIO_ChannelLFE,
        OMX_AUDIO_ChannelCF,OMX_AUDIO_ChannelLS,OMX_AUDIO_ChannelRS,
        OMX_AUDIO_ChannelLR,OMX_AUDIO_ChannelRR};  /*Using OMX_AUDIO_CHANNELTYPE mapping*/

typedef struct _hdmi_out {
    audio_stream_out_t stream_out;
    hdmi_device_t *dev;
    struct pcm_config config;
    struct pcm *pcm;
    audio_config_t android_config;
    int up;
    void *buffcpy;
} hdmi_out_t;

#ifdef PRIMARY_HDMI_AUDIO_HAL
/* buffer_remix: functor for doing in-place buffer manipulations */
typedef struct buffer_remix {
    void (*remix_func)(struct buffer_remix *data, void *buf, size_t frames);
    size_t sample_size; /* size of one audio sample, in bytes */
    size_t in_chans;    /* number of input channels */
    size_t out_chans;   /* number of output channels */
} buffer_remix_t;

typedef struct _audio_in {
    audio_stream_in_t stream;
    hdmi_device_t *dev;
    struct pcm_config config;
    struct pcm *pcm;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    buffer_remix_t *remix; /* adapt hw chan count to client */
    int16_t *buffer;
    size_t frames_in;
    size_t hw_frame_size;
    unsigned int requested_rate;
    unsigned int requested_channels;
    unsigned int card;
    unsigned int port;
    int read_status;
    bool standby;
    pthread_mutex_t lock;
} audio_in_t;

#define MIXER_XML_PATH                  "/vendor/etc/mixer_paths.xml"
#define JAMR_MIXER_XML_PATH             "/vendor/etc/jamr3_mixer_paths.xml"

#define CAPTURE_SAMPLE_RATE             44100
#define CAPTURE_PERIOD_SIZE             1920
#define CAPTURE_PERIOD_COUNT            4
#define CAPTURE_BUFFER_SIZE             (CAPTURE_PERIOD_SIZE * CAPTURE_PERIOD_COUNT)

struct pcm_config pcm_config_capture = {
    .channels        = 2,
    .rate            = CAPTURE_SAMPLE_RATE,
    .format          = PCM_FORMAT_S16_LE,
    .period_size     = CAPTURE_PERIOD_SIZE,
    .period_count    = CAPTURE_PERIOD_COUNT,
    .start_threshold = 1,
    .stop_threshold  = CAPTURE_BUFFER_SIZE,
};

static const char *supported_media_cards[] = {
    "dra7evm",
    "VayuEVM",
    "DRA7xx-EVM",
};

static const char *supported_jamr_cards[] = {
    "DRA7xx-JAMR3",
};
#endif

static const char *supported_hdmi_cards[] = {
    "HDMI",
    "hdmi",
};

#define S16_SIZE sizeof(int16_t)


/*****************************************************************
 * UTILITY FUNCTIONS
 *****************************************************************
 */

#ifdef PRIMARY_HDMI_AUDIO_HAL

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
 * Implementation of buffer_remix::remix_func that removes channels in place
 * without doing any other processing.  The extra channels are dropped.
 */
static void remove_channels_from_buf(buffer_remix_t *data, void *buf, size_t frames)
{
    size_t samp_size, in_frame, out_frame;
    size_t N, c;
    char *s, *d;

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

static int setup_stereo_to_mono_input_remix(audio_in_t *in)
{
    ALOGV("setup_stereo_to_mono_input_remix() stream=%p", in);

    buffer_remix_t *br = (buffer_remix_t *)calloc(1, sizeof(buffer_remix_t));
    if (!br)
        return -ENOMEM;

    br->remix_func = remove_channels_from_buf;
    br->sample_size = sizeof(int16_t);
    br->in_chans = 2;
    br->out_chans = 1;
    in->remix = br;

    return 0;
}

/*****************************************************************
 * AUDIO STREAM IN DEFINITION
 *****************************************************************
 */

static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    const audio_in_t *in = (const audio_in_t *)(stream);

    ALOGV("in_get_sample_rate() stream=%p rate=%u", stream, in->requested_rate);

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    ALOGV("in_set_sample_rate() stream=%p rate=%u", stream, rate);

    return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    const audio_in_t *in = (const audio_in_t *)(stream);

    size_t bytes = get_input_buffer_size(in->requested_rate,
                                         AUDIO_FORMAT_PCM_16_BIT,
                                         in->requested_channels);

    ALOGV("in_get_buffer_size() stream=%p bytes=%u", in, bytes);

    return bytes;
}

static audio_channel_mask_t in_get_channels(const struct audio_stream *stream)
{
    const audio_in_t *in = (const audio_in_t *)(stream);
    audio_channel_mask_t channels = audio_channel_in_mask_from_count(in->requested_channels);

    ALOGV("in_get_channels() stream=%p channels=%u", in, in->requested_channels);

    return channels;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    audio_format_t format = AUDIO_FORMAT_PCM_16_BIT;

    UNUSED(stream);
    ALOGV("in_set_format() stream=%p format=0x%08x (%u bits/sample)",
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
static void do_in_standby(audio_in_t *in)
{
    if (!in->standby) {
        ALOGI("do_in_standby() close card %u port %u", in->card, in->port);
        pcm_close(in->pcm);
        in->pcm = NULL;
        in->standby = true;
    }
}

static int in_standby(struct audio_stream *stream)
{
    audio_in_t *in = (audio_in_t *)(stream);

    pthread_mutex_lock(&in->lock);
    do_in_standby(in);
    pthread_mutex_unlock(&in->lock);

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
    ALOGV("in_set_parameters() stream=%p parameter='%s'", stream, kvpairs);

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
    audio_in_t *in;
    buffer_remix_t *remix;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (audio_in_t *)((char *)buffer_provider - offsetof(audio_in_t, buf_provider));
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
    audio_in_t *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (audio_in_t *)((char *)buffer_provider - offsetof(audio_in_t, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/*
 * read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified
 */
static ssize_t read_frames(audio_in_t *in, void *buffer, ssize_t frames)
{
    const struct audio_stream_in *s = (const struct audio_stream_in *)in;
    ssize_t frames_wr = 0;
    size_t frame_size;

    TRACEM("stream=%p buffer=%p frames=%d", s, buffer, frames);

    if (in->remix)
        frame_size = audio_stream_in_frame_size(s);
    else
        frame_size = in->hw_frame_size;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;

        if  (in->resampler) {
            in->resampler->resample_from_provider(in->resampler,
                    (int16_t *)((char *)buffer + frames_wr * frame_size),
                    &frames_rd);
        } else {
            struct resampler_buffer buf = {
                    { .raw = NULL, },
                    .frame_count = frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw) {
                memcpy((char *)buffer + frames_wr * frame_size,
                       buf.raw,
                       buf.frame_count * frame_size);
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }

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
    audio_in_t *in = (audio_in_t *)stream;
    hdmi_device_t *adev = (hdmi_device_t *)in->dev;
    const size_t frame_size = audio_stream_in_frame_size(stream);
    const size_t frames = bytes / frame_size;
    uint32_t rate = in_get_sample_rate(&stream->common);
    uint32_t read_usecs = frames * 1000000 / rate;
    int ret;

    TRACEM("in_read() stream=%p buffer=%p size=%u/%u time=%u usecs",
           stream, buffer, frames, rate, read_usecs);

    pthread_mutex_lock(&in->lock);

    if (in->standby) {
        ALOGI("in_read() open card %u port %u", in->card, in->port);
        in->pcm = pcm_open(in->card, in->port,
                           PCM_IN | PCM_MONOTONIC,
                           &in->config);
        if (!pcm_is_ready(in->pcm)) {
            ALOGE("in_read() failed to open pcm in: %s", pcm_get_error(in->pcm));
            pcm_close(in->pcm);
            in->pcm = NULL;
            usleep(read_usecs); /* limits the rate of error messages */
            pthread_mutex_unlock(&in->lock);
            return -ENODEV;
        }

        /* if no supported sample rate is available, use the resampler */
        if (in->resampler) {
            in->resampler->reset(in->resampler);
            in->frames_in = 0;
        }

        in->standby = false;
    }

    if (in->resampler || in->remix)
        ret = read_frames(in, buffer, frames);
    else
        ret = pcm_read(in->pcm, buffer, bytes);

    if (ret < 0) {
        ALOGE("in_read() failed to read audio data %d", ret);
        usleep(read_usecs); /* limits the rate of error messages */
        memset(buffer, 0, bytes);
    } else if (adev->mic_mute) {
        memset(buffer, 0, bytes);
    }

    pthread_mutex_unlock(&in->lock);

    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    UNUSED(stream);

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

audio_stream_in_t audio_stream_in_descriptor = {
    .common = {
        .get_sample_rate = in_get_sample_rate,
        .set_sample_rate = in_set_sample_rate,
        .get_buffer_size = in_get_buffer_size,
        .get_channels = in_get_channels,
        .get_format = in_get_format,
        .set_format = in_set_format,
        .standby = in_standby,
        .dump = in_dump,
        .set_parameters = in_set_parameters,
        .get_parameters = in_get_parameters,
        .add_audio_effect = in_add_audio_effect,
        .remove_audio_effect = in_remove_audio_effect,
    },
    .read = in_read,
    .set_gain = in_set_gain,
    .get_input_frames_lost = in_get_input_frames_lost,
};

#endif


/*****************************************************************
 * AUDIO STREAM OUT (hdmi_out_*) DEFINITION
 *****************************************************************
 */

uint32_t hdmi_out_get_sample_rate(const struct audio_stream *stream)
{
    hdmi_out_t *out = (hdmi_out_t*)stream;
    struct pcm_config *config = &out->config;
    TRACEM("stream=%p returning %d", stream, config->rate);
    return config->rate;
}

/* DEPRECATED API */
int hdmi_out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    TRACE();
    UNUSED(stream);
    UNUSED(rate);

    return -EINVAL;
}

/* Returns bytes for ONE PERIOD */
size_t hdmi_out_get_buffer_size(const struct audio_stream *stream)
{
    const struct audio_stream_out *s = (const struct audio_stream_out *)stream;
    hdmi_out_t *out = (hdmi_out_t*)stream;
    struct pcm_config *config = &out->config;
    size_t ans;

    ans = audio_stream_out_frame_size(s) * config->period_size;

    TRACEM("stream=%p returning %u", stream, ans);

    return ans;
}

audio_channel_mask_t hdmi_out_get_channels(const struct audio_stream *stream)
{
    hdmi_out_t *out = (hdmi_out_t*)stream;
    TRACEM("stream=%p returning %x", stream, out->android_config.channel_mask);
    return out->android_config.channel_mask;
}

audio_format_t hdmi_out_get_format(const struct audio_stream *stream)
{
    hdmi_out_t *out = (hdmi_out_t*)stream;
    TRACEM("stream=%p returning %x", stream, out->android_config.format);
    return out->android_config.format;
}

/* DEPRECATED API */
int hdmi_out_set_format(struct audio_stream *stream, audio_format_t format)
{
    TRACE();
    UNUSED(stream);
    UNUSED(format);

    return -EINVAL;
}

int hdmi_out_standby(struct audio_stream *stream)
{
    hdmi_out_t *out = (hdmi_out_t*)stream;

    TRACEM("stream=%p", stream);

    if (out->up && out->pcm) {
        out->up = 0;
        pcm_close(out->pcm);
        out->pcm = 0;
    }

    return 0;
}

int hdmi_out_dump(const struct audio_stream *stream, int fd)
{
    TRACE();
    UNUSED(stream);
    UNUSED(fd);

    return 0;
}

audio_devices_t hdmi_out_get_device(const struct audio_stream *stream)
{
    TRACEM("stream=%p", stream);
    UNUSED(stream);

#ifdef PRIMARY_HDMI_AUDIO_HAL
    return (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_DEFAULT);
#else
    return AUDIO_DEVICE_OUT_AUX_DIGITAL;
#endif
}

/* DEPRECATED API */
int hdmi_out_set_device(struct audio_stream *stream, audio_devices_t device)
{
    TRACE();
    UNUSED(stream);
    UNUSED(device);

    return 0;
}

int hdmi_out_set_parameters(struct audio_stream *stream, const char *kv_pairs)
{
    TRACEM("stream=%p kv_pairs='%s'", stream, kv_pairs);
    UNUSED(stream);
    UNUSED(kv_pairs);

    return 0;
}

#define MASK_CEA_QUAD     ( CEA_SPKR_FLFR | CEA_SPKR_RLRR )
#define MASK_CEA_SURROUND ( CEA_SPKR_FLFR | CEA_SPKR_FC | CEA_SPKR_RC )
#define MASK_CEA_5POINT1  ( CEA_SPKR_FLFR | CEA_SPKR_FC | CEA_SPKR_LFE | CEA_SPKR_RLRR )
#define MASK_CEA_7POINT1  ( CEA_SPKR_FLFR | CEA_SPKR_FC | CEA_SPKR_LFE | CEA_SPKR_RLRR | CEA_SPKR_RLCRRC )
#define SUPPORTS_ARR(spkalloc, profile) (((spkalloc) & (profile)) == (profile))

char * hdmi_out_get_parameters(const struct audio_stream *stream,
			 const char *keys)
{
    struct str_parms *query = str_parms_create_str(keys);
    char *str;
    char value[256];
    struct str_parms *reply = str_parms_create();
    int status;
    hdmi_audio_caps_t caps;

    TRACEM("stream=%p keys='%s'", stream, keys);
    UNUSED(stream);

    if (hdmi_query_audio_caps(&caps)) {
        ALOGE("Unable to get the HDMI audio capabilities");
        str = calloc(1, 1);
        goto end;
    }

    status = str_parms_get_str(query, AUDIO_PARAMETER_STREAM_SUP_CHANNELS,
                                value, sizeof(value));
    if (status >= 0) {
        unsigned sa = caps.speaker_alloc;
        bool first = true;

        /* STEREO is intentionally skipped.  This code is only
         * executed for the 'DIRECT' interface, and we don't
         * want stereo on a DIRECT thread.
         */
        value[0] = '\0';
        if (SUPPORTS_ARR(sa, MASK_CEA_QUAD)) {
            if (!first) {
                strcat(value, "|");
            }
            first = false;
            strcat(value, "AUDIO_CHANNEL_OUT_QUAD");
        }
        if (SUPPORTS_ARR(sa, MASK_CEA_SURROUND)) {
            if (!first) {
                strcat(value, "|");
            }
            first = false;
            strcat(value, "AUDIO_CHANNEL_OUT_SURROUND");
        }
        if (SUPPORTS_ARR(sa, MASK_CEA_5POINT1)) {
            if (!first) {
                strcat(value, "|");
            }
            first = false;
            strcat(value, "AUDIO_CHANNEL_OUT_5POINT1");
        }
        if (SUPPORTS_ARR(sa, MASK_CEA_7POINT1)) {
            if (!first) {
                strcat(value, "|");
            }
            first = false;
            strcat(value, "AUDIO_CHANNEL_OUT_7POINT1");
        }
        str_parms_add_str(reply, AUDIO_PARAMETER_STREAM_SUP_CHANNELS, value);
        str = strdup(str_parms_to_str(reply));
    } else {
        str = strdup(keys);
    }

    ALOGV("%s() reply: '%s'", __func__, str);

end:
    str_parms_destroy(query);
    str_parms_destroy(reply);
    return str;
}
int hdmi_out_add_audio_effect(const struct audio_stream *stream,
			effect_handle_t effect)
{
    TRACE();
    UNUSED(stream);
    UNUSED(effect);

    return 0;
}
int hdmi_out_remove_audio_effect(const struct audio_stream *stream,
			   effect_handle_t effect)
{
    TRACE();
    UNUSED(stream);
    UNUSED(effect);

    return 0;
}

/* returns milliseconds */
uint32_t hdmi_out_get_latency(const struct audio_stream_out *stream)
{
    uint32_t latency;
    hdmi_out_t *out = (hdmi_out_t*)stream;
    struct pcm_config *config = &out->config;

    TRACEM("stream=%p", stream);

    return (1000 * config->period_size * config->period_count) / config->rate;
}

int hdmi_out_set_volume(struct audio_stream_out *stream, float left, float right)
{
    TRACE();
    UNUSED(stream);
    UNUSED(left);
    UNUSED(right);

    return -ENOSYS;
}

static bool find_card_index(const char *supported_cards[],
                            int num_supported,
                            unsigned int *index)
{
    struct mixer *mixer;
    const char *name;
    bool found = false;
    int card = 0;
    int i;

    do {
        /* returns an error after last valid card */
        mixer = mixer_open(card);
        if (!mixer)
            break;

        name = mixer_get_name(mixer);

        for (i = 0; i < num_supported; ++i) {
            if (supported_cards[i] && strstr(name, supported_cards[i])) {
                TRACEM("Supported card '%s' found at %d", name, card);
                found = true;
                *index = card;
                break;
            }
        }

        mixer_close(mixer);
    } while (!found && (card++ < MAX_CARD_COUNT));

    return found;
}

static int hdmi_out_open_pcm(hdmi_out_t *out)
{
    bool found;
    unsigned int card;
    int dev = HDMI_PCM_DEV;
    int ret;

    TRACEM("out=%p", out);

    found = find_card_index(supported_hdmi_cards,
                            ARRAY_SIZE(supported_hdmi_cards),
                            &card);
    if (!found) {
        ALOGE("HDMI card not available");
        return -ENODEV;
    }

    /* out->up must be 0 (down) */
    if (out->up) {
        ALOGE("Trying to open a PCM that's already up. "
             "This will probably deadlock... so aborting");
        return 0;
    }

    out->pcm = pcm_open(card, dev, PCM_OUT, &out->config);

    if(out->pcm && pcm_is_ready(out->pcm)) {
        out->up = 1;
        ret = 0;
    } else {
        ALOGE("cannot open HDMI pcm card %d dev %d error: %s",
              card, dev, pcm_get_error(out->pcm));
        pcm_close(out->pcm);
        out->pcm = 0;
        out->up = 0;
        ret = 1;
    }

    return ret;
}

void channel_remap(struct audio_stream_out *stream, const void *buffer,
                    size_t bytes)
{
        hdmi_out_t *out = (hdmi_out_t*)stream;
        hdmi_device_t *adev = out->dev;
        int x, y, frames;
        int16_t *buf = (int16_t *)buffer;
        int16_t *tmp_buf = (int16_t *)out->buffcpy;

        frames = bytes / audio_stream_out_frame_size(stream);
        while (frames--){
            for(y = 0; y < (int)out->config.channels; y++){
                for(x = 0; x < (int)out->config.channels; x++){
                    if (cea_channel_map[y] == adev->map[x]){
                        tmp_buf[y] = buf[x];
                        break;
                    }
                }
            }
            tmp_buf += (int)out->config.channels;
            buf += (int)out->config.channels;
        }
}

ssize_t hdmi_out_write(struct audio_stream_out *stream, const void* buffer,
		 size_t bytes)
{
    hdmi_out_t *out = (hdmi_out_t*)stream;
    hdmi_device_t *adev = out->dev;
    ssize_t ret;

    TRACEM("stream=%p buffer=%p bytes=%d", stream, buffer, bytes);

    if (!out->up) {
        if(hdmi_out_open_pcm(out)) {
            ret = -ENOSYS;
	    goto exit;
        }
    }

    if (out->config.channels > 2 && !adev->CEAMap){
        channel_remap(stream, buffer, bytes);
        ret = pcm_write(out->pcm, out->buffcpy, bytes);
    } else {
       ret = pcm_write(out->pcm, buffer, bytes);
    }
exit:
    if (ret != 0) {
        ALOGE("Error writing to HDMI pcm: %s",
              out->pcm ? pcm_get_error(out->pcm) : "failed to open PCM device");
        hdmi_out_standby((struct audio_stream*)stream);
	unsigned int usecs = bytes * 1000000 /
                        audio_stream_out_frame_size(stream) /
                        hdmi_out_get_sample_rate((struct audio_stream*)stream);
	if (usecs >= 1000000L) {
	    usecs = 999999L;
	}
	usleep(usecs);
    }

    return bytes;
}


int hdmi_out_get_render_position(const struct audio_stream_out *stream,
			   uint32_t *dsp_frames)
{
    TRACE();
    UNUSED(stream);
    UNUSED(dsp_frames);

    return -EINVAL;
}

int hdmi_out_get_next_write_timestamp(const struct audio_stream_out *stream,
				int64_t *timestamp)
{
    TRACE();
    UNUSED(stream);
    UNUSED(timestamp);

    return -EINVAL;
}



audio_stream_out_t hdmi_stream_out_descriptor = {
    .common = {
        .get_sample_rate = hdmi_out_get_sample_rate,
        .set_sample_rate = hdmi_out_set_sample_rate,
        .get_buffer_size = hdmi_out_get_buffer_size,
        .get_channels = hdmi_out_get_channels,
        .get_format = hdmi_out_get_format,
        .set_format = hdmi_out_set_format,
        .standby = hdmi_out_standby,
        .dump = hdmi_out_dump,
        .get_device = hdmi_out_get_device,
        .set_device = hdmi_out_set_device,
        .set_parameters = hdmi_out_set_parameters,
        .get_parameters = hdmi_out_get_parameters,
        .add_audio_effect = hdmi_out_add_audio_effect,
        .remove_audio_effect = hdmi_out_remove_audio_effect,
    },
    .get_latency = hdmi_out_get_latency,
    .set_volume = hdmi_out_set_volume,
    .write = hdmi_out_write,
    .get_render_position = hdmi_out_get_render_position,
    .get_next_write_timestamp = hdmi_out_get_next_write_timestamp,
};

/*****************************************************************
 * AUDIO DEVICE (hdmi_adev_*) DEFINITION
 *****************************************************************
 */

static int hdmi_adev_close(struct hw_device_t *device)
{
    TRACE();

#ifdef PRIMARY_HDMI_AUDIO_HAL
    hdmi_device_t *adev = (hdmi_device_t *)device;

    if (adev->has_media) {
        audio_route_free(adev->route);
        adev->route = NULL;
    }

    if (adev->has_jamr) {
        audio_route_free(adev->jamr_route);
        adev->jamr_route = NULL;
    }
#else
    UNUSED(device);
#endif

    return 0;
}

static uint32_t hdmi_adev_get_supported_devices(const audio_hw_device_t *dev)
{
    TRACE();
    UNUSED(dev);

#ifdef PRIMARY_HDMI_AUDIO_HAL
    return (AUDIO_DEVICE_OUT_SPEAKER | AUDIO_DEVICE_OUT_DEFAULT);
#else
    return AUDIO_DEVICE_OUT_AUX_DIGITAL;
#endif
}

static int hdmi_adev_init_check(const audio_hw_device_t *dev)
{
    TRACE();
    UNUSED(dev);

    return 0;
}

static int hdmi_adev_set_voice_volume(audio_hw_device_t *dev, float volume)
{
    TRACE();
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int hdmi_adev_set_master_volume(audio_hw_device_t *dev, float volume)
{
    TRACE();
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int hdmi_adev_get_master_volume(audio_hw_device_t *dev, float *volume)
{
    TRACE();
    UNUSED(dev);
    UNUSED(volume);

    return -ENOSYS;
}

static int hdmi_adev_set_mode(audio_hw_device_t *dev, audio_mode_t mode)
{
    TRACE();
    UNUSED(dev);
    UNUSED(mode);

    return 0;
}

static int hdmi_adev_set_mic_mute(audio_hw_device_t *dev, bool state)
{
    TRACE();

#ifdef PRIMARY_HDMI_AUDIO_HAL
    hdmi_device_t *adev = (hdmi_device_t *)dev;

    ALOGV("adev_set_mic_mute() state=%s", state ? "mute" : "unmute");
    adev->mic_mute = state;

    return 0;
#else
    UNUSED(dev);
    UNUSED(state);

    return -ENOSYS;
#endif
}

static int hdmi_adev_get_mic_mute(const audio_hw_device_t *dev, bool *state)
{
    TRACE();

#ifdef PRIMARY_HDMI_AUDIO_HAL
    const hdmi_device_t *adev = (const hdmi_device_t *)dev;

    *state = adev->mic_mute;
    ALOGV("adev_get_mic_mute() state=%s", *state ? "mute" : "unmute");

    return 0;
#else
    UNUSED(dev);
    UNUSED(state);

    return -ENOSYS;
#endif
}

static int hdmi_adev_set_parameters(audio_hw_device_t *dev, const char *kv_pairs)
{
    TRACEM("dev=%p kv_pairss='%s'", dev, kv_pairs);

    struct str_parms *params;
    char *str;
    char value[HDMI_MAX_CHANNELS];
    int ret, x, val, numMatch = 0;
    hdmi_device_t *adev = (hdmi_device_t *)dev;

    params = str_parms_create_str(kv_pairs);
    //Handle maximum of 8 channels
    ret = str_parms_get_str(params, "channel_map", value, HDMI_MAX_CHANNELS);
    if (ret >= 0) {
        val = strtol(value, NULL, 10);
        for(x = 0; x < HDMI_MAX_CHANNELS; x++) {
            adev->map[x] = (val & (0xF << x*4)) >> x*4;
            if (adev->map[x] == cea_channel_map[x])
                numMatch += 1;
        }
        if (numMatch >= 5)
            adev->CEAMap = true;
        else
            adev->CEAMap = false;
    }
    return 0;
}

static char* hdmi_adev_get_parameters(const audio_hw_device_t *dev,
                                      const char *keys)
{
    TRACEM("dev=%p keys='%s'", dev, keys);
    UNUSED(dev);
    UNUSED(keys);

    return NULL;
}

static size_t hdmi_adev_get_input_buffer_size(const audio_hw_device_t *dev,
                                              const struct audio_config *config)
{
    TRACE();
    UNUSED(dev);
    UNUSED(config);

    return 0;
}

#define DUMP_FLAG(flags, flag) {                \
        if ((flags) & (flag)) {                 \
            ALOGV("set:   " #flag);             \
        } else {                                \
            ALOGV("unset: " #flag);             \
        }                                       \
    }

static int hdmi_adev_open_output_stream(audio_hw_device_t *dev,
                                        audio_io_handle_t handle,
                                        audio_devices_t devices,
                                        audio_output_flags_t flags,
                                        struct audio_config *config,
                                        struct audio_stream_out **stream_out,
                                        const char *address)
{
    hdmi_out_t *out = 0;
    struct pcm_config *pcm_config = 0;
    struct audio_config *a_config = 0;
    hdmi_audio_caps_t caps;
    unsigned int sa;

    TRACE();
    UNUSED(handle);
    UNUSED(devices);
    UNUSED(flags);
    UNUSED(address);

    out = calloc(1, sizeof(hdmi_out_t));
    if (!out) {
        return -ENOMEM;
    }

    out->dev = (hdmi_device_t *)dev;
    memcpy(&out->stream_out, &hdmi_stream_out_descriptor,
           sizeof(audio_stream_out_t));
    memcpy(&out->android_config, config, sizeof(audio_config_t));

    pcm_config = &out->config;
    a_config = &out->android_config;

#if defined(LOG_NDEBUG) && (LOG_NDEBUG == 0)
    /* Analyze flags */
    if (flags) {
        DUMP_FLAG(flags, AUDIO_OUTPUT_FLAG_DIRECT)
        DUMP_FLAG(flags, AUDIO_OUTPUT_FLAG_PRIMARY)
        DUMP_FLAG(flags, AUDIO_OUTPUT_FLAG_FAST)
        DUMP_FLAG(flags, AUDIO_OUTPUT_FLAG_DEEP_BUFFER)
    } else {
        ALOGV("flags == AUDIO_OUTPUT_FLAG_NONE (0)");
    }
#endif /* defined(LOG_NDEBUG) && (LOG_NDEBUG == 0) */
    /* Initialize the PCM Configuration */
    pcm_config->period_size = HDMI_PERIOD_SIZE;
    pcm_config->period_count = HDMI_PERIOD_COUNT;

    if (a_config->sample_rate) {
        pcm_config->rate = config->sample_rate;
    } else {
        pcm_config->rate = HDMI_SAMPLING_RATE;
        a_config->sample_rate = HDMI_SAMPLING_RATE;
    }

    switch (a_config->format) {
    case AUDIO_FORMAT_DEFAULT:
        config->format = AUDIO_FORMAT_PCM_16_BIT;
        a_config->format = config->format;
        /* fall through */
    case AUDIO_FORMAT_PCM_16_BIT:
        pcm_config->format = PCM_FORMAT_S16_LE;
        break;
    default:
        ALOGE("HDMI rejecting format %x", config->format);
        goto fail;
    }

    a_config->channel_mask = config->channel_mask;
    switch (config->channel_mask) {
    case AUDIO_CHANNEL_OUT_STEREO:
        pcm_config->channels = 2;
        break;
    case AUDIO_CHANNEL_OUT_QUAD:
        pcm_config->channels = 4;
        break;
    case AUDIO_CHANNEL_OUT_5POINT1:
        pcm_config->channels = 6;
        break;
    case AUDIO_CHANNEL_OUT_7POINT1:
        pcm_config->channels = 8;
        break;
    default:
        if (!hdmi_query_audio_caps(&caps)) {
            sa = caps.speaker_alloc;
            if (SUPPORTS_ARR(sa, MASK_CEA_7POINT1))
                config->channel_mask = AUDIO_CHANNEL_OUT_7POINT1;
            else if (SUPPORTS_ARR(sa, MASK_CEA_5POINT1))
                config->channel_mask = AUDIO_CHANNEL_OUT_5POINT1;
            else if (SUPPORTS_ARR(sa, MASK_CEA_SURROUND))
                config->channel_mask = AUDIO_CHANNEL_OUT_SURROUND;
            else if (SUPPORTS_ARR(sa, MASK_CEA_QUAD))
                config->channel_mask = AUDIO_CHANNEL_OUT_QUAD;
            else
                config->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
        } else {
            ALOGE("Unable to get HDMI audio caps, set a default channel_mask %x -> 8",
                  config->channel_mask);
            config->channel_mask = AUDIO_CHANNEL_OUT_7POINT1;
        }

        a_config->channel_mask = config->channel_mask;
        pcm_config->channels = popcount(config->channel_mask);
        ALOGI("Dynamic channel count from EDID set to %d", pcm_config->channels);
    }

    //Allocating buffer for at most 8 channels
    out->buffcpy = malloc(pcm_config->period_size * sizeof(int16_t) * HDMI_MAX_CHANNELS);
    if (!out->buffcpy){
        ALOGE("Could not allocate memory");
        goto fail;
    }

    ALOGV("stream = %p", out);
    *stream_out = &out->stream_out;

    return 0;

fail:
    free(out);
    return -ENOSYS;
}

static void hdmi_adev_close_output_stream(audio_hw_device_t *dev,
                                          struct audio_stream_out* stream_out)
{
    hdmi_out_t *out = (hdmi_out_t*)stream_out;

    TRACEM("dev=%p stream_out=%p", dev, stream_out);
    UNUSED(dev);

    stream_out->common.standby((audio_stream_t*)stream_out);
    free(out->buffcpy);
    out->buffcpy = NULL;
    free(stream_out);
}

static int hdmi_adev_open_input_stream(audio_hw_device_t *dev,
                                       audio_io_handle_t handle,
                                       audio_devices_t devices,
                                       struct audio_config *config,
                                       struct audio_stream_in **stream_in,
                                       audio_input_flags_t flags,
                                       const char *address,
                                       audio_source_t source)
{
    TRACE();

#ifdef PRIMARY_HDMI_AUDIO_HAL
    hdmi_device_t *adev = (hdmi_device_t *)dev;
    audio_in_t *in;
    int buffer_size;
    int ret;

    UNUSED(handle);
    UNUSED(flags);
    UNUSED(address);
    UNUSED(source);

    if ((devices == AUDIO_DEVICE_IN_BUILTIN_MIC) && !adev->has_media) {
        ALOGE("hdmi_adev_open_input_stream() mic not available");
        return -ENODEV;
    }

    if ((devices == AUDIO_DEVICE_IN_LINE) && !adev->has_jamr) {
        ALOGE("hdmi_adev_open_input_stream() line-in not available");
        return -ENODEV;
    }

    in = (audio_in_t *)calloc(1, sizeof(audio_in_t));
    if (!in)
        return -ENOMEM;

    ALOGV("hdmi_adev_open_input_stream() stream=%p rate=%u channels=%u format=0x%08x",
          in, config->sample_rate, popcount(config->channel_mask), config->format);

    pthread_mutex_init(&in->lock, NULL);

    memcpy(&in->stream, &audio_stream_in_descriptor, sizeof(audio_stream_in_t));

    in->dev = adev;
    in->standby = true;
    in->config = pcm_config_capture;
    in->requested_rate = config->sample_rate;
    in->requested_channels = popcount(config->channel_mask);
    in->hw_frame_size = in->config.channels * sizeof(int16_t);
    in->remix = NULL;
    in->resampler = NULL;
    in->buffer = NULL;
    in->card = (devices == AUDIO_DEVICE_IN_LINE) ? adev->jamr_card : adev->card;
    in->port = 0;

    /* in-place stereo-to-mono remix since capture stream is stereo */
    if (in->requested_channels == 1) {
        ALOGV("adev_open_input_stream() stereo-to-mono remix needed");
        ret = setup_stereo_to_mono_input_remix(in);
        if (ret) {
            ALOGE("adev_open_input_stream() failed to setup remix %d", ret);
            goto err1;
        }
    }

    if (in->requested_rate != in->config.rate) {
        ALOGV("hdmi_adev_open_input_stream() resample needed, req=%uHz got=%uHz",
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
            ALOGE("hdmi_adev_open_input_stream() failed to create resampler %d", ret);
            goto err2;
        }
    }

    /*
     * The buffer size needs to be enough to allow stereo-to-mono remix
     * and resample if needed
     */
    if (in->resampler || in->remix) {
        buffer_size = in->config.period_size * in->hw_frame_size;
        if (in->resampler)
            buffer_size *= 2;
        if (in->remix)
            buffer_size *= 2;

        in->buffer = malloc(buffer_size);
        if (!in->buffer) {
            ret = -ENOMEM;
            goto err3;
        }
    }

    *stream_in = &in->stream;

    return 0;

err3:
    release_resampler(in->resampler);
err2:
    free(in->remix);
err1:
    free(in);
    return ret;
#else
    UNUSED(dev);
    UNUSED(handle);
    UNUSED(devices);
    UNUSED(config);
    UNUSED(stream_in);
    UNUSED(flags);
    UNUSED(address);
    UNUSED(source);

    return -ENOSYS;
#endif
}

static void hdmi_adev_close_input_stream(audio_hw_device_t *dev,
                                         struct audio_stream_in *stream_in)
{
    TRACE();

#ifdef PRIMARY_HDMI_AUDIO_HAL
    hdmi_device_t *adev = (hdmi_device_t *)dev;
    audio_in_t *in = (audio_in_t *)stream_in;

    ALOGV("hdmi_adev_close_input_stream() stream=%p", stream_in);

    in_standby(&stream_in->common);

    if (in->resampler)
        release_resampler(in->resampler);

    if (in->remix)
        free(in->remix);

    free(in->buffer);
    free(in);
#else
    UNUSED(dev);
    UNUSED(stream_in);
#endif
}

static int hdmi_adev_dump(const audio_hw_device_t *dev, int fd)
{
    TRACE();
    UNUSED(dev);
    UNUSED(fd);

    return 0;
}

static hdmi_device_t hdmi_adev = {
    .device = {
        .common = {
            .tag = HARDWARE_DEVICE_TAG,
            .version = AUDIO_DEVICE_API_VERSION_2_0,
            .module = NULL,
            .close = hdmi_adev_close,
        },
        .get_supported_devices = hdmi_adev_get_supported_devices,
        .init_check = hdmi_adev_init_check,
        .set_voice_volume = hdmi_adev_set_voice_volume,
        .set_master_volume = hdmi_adev_set_master_volume,
        .get_master_volume = hdmi_adev_get_master_volume,
        .set_mode = hdmi_adev_set_mode,
        .set_mic_mute = hdmi_adev_set_mic_mute,
        .get_mic_mute = hdmi_adev_get_mic_mute,
        .set_parameters = hdmi_adev_set_parameters,
        .get_parameters = hdmi_adev_get_parameters,
        .get_input_buffer_size = hdmi_adev_get_input_buffer_size,
        .open_output_stream = hdmi_adev_open_output_stream,
        .close_output_stream = hdmi_adev_close_output_stream,
        .open_input_stream =  hdmi_adev_open_input_stream,
        .close_input_stream = hdmi_adev_close_input_stream,
        .dump = hdmi_adev_dump,
    },

    // Don't reorder channels until a valid CEA mapping has been
    // explicitly set. IOW, assume default channel mapping is
    // fine until some other mapping is requested.
    .CEAMap = true,
    .map = {0},
};

static int hdmi_adev_open(const hw_module_t* module,
                          const char* name,
                          hw_device_t** device)
{
    TRACE();

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    hdmi_adev.device.common.module = (struct hw_module_t *) module;
    *device = &hdmi_adev.device.common;

#ifdef PRIMARY_HDMI_AUDIO_HAL
    hdmi_adev.has_media = find_card_index(supported_media_cards,
                                          ARRAY_SIZE(supported_media_cards),
                                          &hdmi_adev.card);

    hdmi_adev.has_jamr = find_card_index(supported_jamr_cards,
                                         ARRAY_SIZE(supported_jamr_cards),
                                         &hdmi_adev.jamr_card);

    if (hdmi_adev.has_media) {
        hdmi_adev.route = audio_route_init(hdmi_adev.card, MIXER_XML_PATH);
        if (!hdmi_adev.route) {
            ALOGE("Unable to initialize audio routes");
            return -ENODEV;
        }
    } else {
        ALOGW("Media card not detected, microphone won't be available");
    }

    if (hdmi_adev.has_jamr) {
        hdmi_adev.jamr_route = audio_route_init(hdmi_adev.jamr_card, JAMR_MIXER_XML_PATH);
        if (!hdmi_adev.jamr_route) {
            ALOGE("Unable to initialize JAMR audio routes");
            if (hdmi_adev.has_media) {
                audio_route_free(hdmi_adev.route);
                hdmi_adev.route = NULL;
            }
            return -ENODEV;
        }
    } else {
        ALOGW("JAMR card not detected, line-in won't be available");
    }
#endif

    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = hdmi_adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "OMAP HDMI audio HW HAL",
        .author = "Texas Instruments",
        .methods = &hal_module_methods,
    },
};
