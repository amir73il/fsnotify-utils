dirs="bin dev etc lib lib64 sbin usr var vtmp results proc sys tmp"

USER=${1:-fsgqa}
GROUP=$USER
ROOT=/home/$USER/root

mkdir -p $ROOT
chown $USER:$GROUP $ROOT
mount --bind $ROOT $ROOT
mount --make-private $ROOT

for dir in $dirs; do
	mkdir -p $ROOT/$dir
	chown $USER:$GROUP $ROOT/$dir
	mount -o bind /$dir $ROOT/$dir
done

su - $USER -c "unshare --mount --user --fork --map-root-user /usr/sbin/chroot $ROOT sh"

for dir in $dirs; do
	umount $ROOT/$dir
done
umount $ROOT

