#!/bin/sh -x
echo "file fs/notify/fanotify/*  +p" > /sys/kernel/debug/dynamic_debug/control
mkdir -p a/b/c/d/e/f/g/
mkdir -p /tmp/b
mount -o bind a/b /tmp/b
echo 3 > /proc/sys/vm/drop_caches
./fanotify_demo a/ &
sleep 1
touch a/0
chmod +x a/0
mkdir a/dir0
sleep 1
mv a/0 a/1
mv a/dir0 a/dir1
sleep 1
rm a/1
rmdir a/dir1
sleep 1
touch /tmp/b/c/d/e/f/0
chmod +x /tmp/b/c/d/e/f/0
sleep 1
mv /tmp/b/c/d/e/f/0 /tmp/b/c/d/e/f/1
mv /tmp/b/c/d/e/f/g /tmp/b/c/d/e/f/G
sleep 1
rm /tmp/b/c/d/e/f/1
rmdir /tmp/b/c/d/e/f/G

