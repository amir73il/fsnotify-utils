#!/bin/sh -x

WD=$1

if test -z "$WD"; then
	WD=a
fi

#SLEEP='sleep 1'

#echo "file fs/notify/fanotify/*  +p" > /sys/kernel/debug/dynamic_debug/control
rm -rf a
mkdir -p a/b/c/d/e/f/g/
mkdir -p /tmp/g
mount -o bind a/b/c/d/e/f/g /tmp/g
#echo 3 > /proc/sys/vm/drop_caches
#./inotify_demo $WD &
./fanotify_demo $WD &

echo Hit any key to start events...
read a

$SLEEP
touch a/1
mknod a/0 c 0 0
chmod +x a/0
mkdir a/dir0
$SLEEP
mv a/0 a/1
mv a/dir0 a/dir1
$SLEEP
rm a/1
rmdir a/dir1
sleep 1
touch a/b/c/d/e/f/g/0
chmod +x a/b/c/d/e/f/g/0
$SLEEP
mv a/b/c/d/e/f/g/0 a/b/c/d/e/f/g/1
rm a/b/c/d/e/f/g/1
$SLEEP
mv a/b/c/d/e/f/g a/b/c/d/e/f/G
rmdir a/b/c/d/e/f/G
