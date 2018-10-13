#!/bin/bash

usage() {
	echo "$0 <revision..range|changes..file> [author]"
	exit 1
}

RANGE=$1
export AUTHOR=${2:-torvalds}

[[ "$RANGE" =~ ".." ]] || usage

VFSFILES="fs/*.* include/linux/fs.h"
VFSDOCS="Documentation/filesystems/vfs.txt Documentation/filesystems/porting Documentation/filesystems/Locking"
if [ -f $RANGE ]; then
	LOGFILE=$RANGE.diff
else
	LOGFILE=vfs-changelog-$RANGE-$AUTHOR.diff
fi
TMPPREFIX="/tmp/vfs-" changelog.sh $RANGE "$VFSFILES $VFSDOCS" | tee $LOGFILE
