#!/bin/sh -x

WD=$1

if test -z "$WD"; then
	WD=a
fi

#SLEEP='sleep 1'

echo "file fs/notify/fanotify/*  +p" > /sys/kernel/debug/dynamic_debug/control
mkdir -p a/b/c/d/e/f/g/
mkdir -p /tmp/b
mount -o bind a/b /tmp/b
#echo 3 > /proc/sys/vm/drop_caches
./fanotify_demo $WD &

echo Hit any key to start events...
read a

$SLEEP
touch a/0
chmod +x a/0
mkdir a/dir0
$SLEEP
mv a/0 a/1
mv a/dir0 a/dir1
$SLEEP
rm a/1
rmdir a/dir1
sleep 1
touch /tmp/b/c/d/e/f/g/0
chmod +x /tmp/b/c/d/e/f/g/0
$SLEEP
mv /tmp/b/c/d/e/f/g/0 /tmp/b/c/d/e/f/g/1
rm /tmp/b/c/d/e/f/g/1
$SLEEP
mv /tmp/b/c/d/e/f/g /tmp/b/c/d/e/f/G
rmdir /tmp/b/c/d/e/f/G
