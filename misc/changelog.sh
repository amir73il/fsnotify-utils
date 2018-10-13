#!/bin/bash

usage() {
	echo "$0 <revision..range|changes..file> <path>..."
	exit 1
}

RANGE=$1
shift
FILES="$*"

[[ "$RANGE" =~ ".." ]] && [ -n "$FILES" ] || usage

if [ -f $RANGE ]; then
	TMPFILE=$RANGE
else
	TMPPREFIX=${TMPPREFIX:-/tmp/}
	AUTHOR=${AUTHOR:-torvalds}
	TMPFILE=${TMPPREFIX}changes-$RANGE-$AUTHOR
	git log --ancestry-path --oneline --author $AUTHOR $RANGE -- $FILES > $TMPFILE
fi
[ -s $TMPFILE ] || exit 0

MERGESFILE=${TMPFILE}.merges
COMMITSFILE=${TMPFILE}.commits
rm -f $MERGESFILE $COMMITSFILE

cat $TMPFILE | while read a b ; do
	fsdiff=$( git diff --name-status $a^..$a -- $FILES)
	[ -n "$fsdiff" ] || continue
	echo -n "+++ "
	git log -1 $a
	git log -1 --oneline $a >> $MERGESFILE
	git diff --full-diff --name-status $a^..$a -- $FILES
	echo "---"
	git log --oneline --full-diff --name-status --no-merges $a^..$a -- $FILES
	git log --oneline --no-merges $a^..$a -- $FILES >> $COMMITSFILE
	echo "--"
done

merges=$(cat $MERGESFILE | wc -l)
commits=$(cat $COMMITSFILE | wc -l)
echo $commits commits in $merges merged branches
