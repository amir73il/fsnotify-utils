#!/bin/sh -x

WD=${1:-"."}
cd $WD

#echo "file fs/notify/fanotify/*  +p" > /sys/kernel/debug/dynamic_debug/control
rm -rf a
mkdir -p a/b/c/d/e/f/g/
touch a/b/c/0 a/b/c/1 a/b/c/d/e/f/g/0

if [ $(id -u) = 0 ]; then
	# FAN_MARK_FILESYSTEM and open_by_handle_at(2)
	MODE='--global'
else
	# FAN_UNPRIVILEGED recursive watches
	MODE='--recursive'
fi

# Sleep 2 seconds while generating events and then process events
EVENTS="-w"
inotifywatch $MODE $EVENTS --timeout -2 $WD &

sleep 1
#SLEEP='sleep 1'
$SLEEP
t="Create files and dirs..."
touch a/0 a/1 a/2 a/3
mkdir a/dir0 a/dir1 a/dir2
$SLEEP
t="Rename files and dirs..."
mv a/0 a/3
mv a/dir0 a/dir3
$SLEEP
t="Delete files and dirs..."
rm a/1
rmdir a/dir1
$SLEEP
t="Modify files and dirs..."
chmod +x a/b/c/d
echo >> a/b/c/0
$SLEEP
t="Move files and dirs..."
mv a/b/c/1 a/b/c/d/e/f/g/1
mv a/b/c/d/e/f/g a/b/c/d/e/G
$SLEEP
