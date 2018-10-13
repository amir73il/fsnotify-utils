version=$1

error() {
	echo $1
	exit 1
}

[ -n "$version" ] || error "usage: $0 <kernel version>"

uuid="XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
entry="gnulinux-advanced-${uuid}>gnulinux-${version}-${uuid}"

GRUBCFG=/boot/grub/grub.cfg
GRUBENV=/boot/grub/grubenv

grep -wF "${version}-${uuid}" $GRUBCFG || error "entry for ${version} not found in $GRUBCFG"

#grub-set-default "$entry"
grub-reboot "$entry"
cat $GRUBENV
