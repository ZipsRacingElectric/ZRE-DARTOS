#!/bin/bash
sfdisk --force --delete /dev/mmcblk0 2
echo 'start=212992, type=83' | sfdisk --force -N 2 /dev/mmcblk0
sleep 4s
partx /dev/mmcblk0
resize2fs /dev/mmcblk0p2

systemctl disable resize_fs

reboot now