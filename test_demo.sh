echo "file fs/notify/fanotify/*  +p" > /sys/kernel/debug/dynamic_debug/control
mkdir -p a/b/c/d/e/f/g/
mkdir -p /tmp/b
mount -o bind a/b /tmp/b
echo 3 > /proc/sys/vm/drop_caches
./fanotify_demo a/ &
sleep 1
touch a/0
chmod +x a/0
mkdir a/dir
sleep 1
rm a/0
rmdir a/dir
sleep 1
touch /tmp/b/c/d/e/f/0
chmod +x /tmp/b/c/d/e/f/0
sleep 1
rm /tmp/b/c/d/e/f/0
rmdir /tmp/b/c/d/e/f/g/

