#!/bin/bash
#
# mkdirs.sh <dir> <depth> [jobs depth]
#
# creates directory tree recursively
#
# example:
# mkdir.sh D 5 2
#
# Creates 1000000 leaf directories of the path pattern D/x/x/x/x/x/x
# Where x is 0..9
# By executing 100 paralel jobs (e.g. for each node D/x/x) and every
# paralel job spawns 3 sub jobs (e.g. for D/x/x/a test/x/x/a/b D/x/x/a/b/c),
# so the output of command "ps |grep mkdirs" has ~400 lines and the values
# of a in the patern (0..9) are the best indication of the entire progress.
# Every job creates exactly the 10 direct subdirs under the node of the job.

if [ $# -lt 2 ]; then
	echo "usage: $0 <dir> <depth> [jobs depth]"
	exit 1
fi

function error()
{
	echo "$1"
	exit 1
}

DIR="$1"
test -d "$DIR" || error "bad path: $DIR"

DEPTH="$2"
test "$DEPTH" -ge 0 || error "bad depth: $DEPTH"

JOBS="$3"
test -z "$JOBS" || test "$JOBS" -ge 0 || error "bad jobs depth: $JOBS"

mkdir -p "$DIR"/{0..9} || error "mkdir $DIR/0..9"

test "$DEPTH" -gt 0 || exit 0

# be verbose only when reaching jobs depth (and not deeper)
test "$JOBS" = 0 && echo "mkdirs $DIR..."

for d in "$DIR"/{0..9}; do
	if test -n "$JOBS" && test "$JOBS" -gt 0; then
		$0 $d $(( $DEPTH - 1 )) $(( $JOBS - 1 )) &
	else
		$0 $d $(( $DEPTH - 1 ))
	fi
done
