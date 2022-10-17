#!/bin/bash -x

cp -a /vtmp/libssh* /lib/x86_64-linux-gnu/
cp -a /vtmp/libcurl* /lib/x86_64-linux-gnu/
cp -a /vtmp/libgumbo* /lib/x86_64-linux-gnu/
cp -a /vtmp/libubsan* /lib/x86_64-linux-gnu/
cp -a /vtmp/ca-certificates.crt /etc/ssl/certs/

MNT=/mnt/www
umount /vdf 2>/dev/null
mount /dev/vdf /vdf
# Uncomment to reset cache on startup
#rm -rf /vdf/cache/*
mkdir -p /vdf/cache/data $MNT
umount /vdf/cache/data $MNT 2>/dev/null
ln -sf /vdf/cache/data/linux $MNT/

echo nameserver 8.8.8.8 > /etc/resolv.conf
echo "file fs/namei.c -p" > /sys/kernel/debug/dynamic_debug/control
echo "file fs/notify/fsnotify.c -p" > /sys/kernel/debug/dynamic_debug/control
echo "file fs/notify/fanotify/* -p" > /sys/kernel/debug/dynamic_debug/control

echo "time tar tvfz $MNT/linux/kernel/firmware/linux-firmware-20220815.tar.gz |head"

cd /vtmp
# Uncomment to use filesystem mark
#MNT=-
./httpdirfs -d --fanotify --dl-seg-size 1 --cache-location /vdf/cache https://cdn.kernel.org/pub/ $MNT &
