#!/system/bin/sh

# Assign all CPUs for foreground (J6 and J6 Eco have different number of cores)
cat /sys/devices/system/cpu/present > /dev/cpuset/foreground/cpus
cat /sys/devices/system/cpu/present > /dev/cpuset/foreground/boost/cpus
