#!/bin/sh
# install tests to kvm test machine

which kvm-cp || exit 1

kvm-cp $*

echo "sudo kvm-xfstests shell"
echo "cd /results"
echo "./test_demo.sh"
echo "./fanotify_example a &"
