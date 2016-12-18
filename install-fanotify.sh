sudo mount /mnt/results/
sudo cp $* /mnt/results/
sudo umount /mnt/results/

echo sudo kvm-xfstests shell
echo cd /results
echo ./test_demo.sh
