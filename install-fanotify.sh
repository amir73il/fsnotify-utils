sudo mount /mnt/results/
sudo cp inotify_demo /mnt/results/
sudo cp fanotify_demo /mnt/results/
sudo cp fanotify_bug /mnt/results/
sudo cp test_demo.sh /mnt/results/
sudo umount /mnt/results/

echo kvm-xfstests shell
echo cd /results
echo ./test_demo.sh
