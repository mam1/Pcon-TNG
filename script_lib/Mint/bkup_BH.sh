#!/bin/bash
echo -n $(date) >> /media/logs/bkup-log
echo backup started >> /media/logs/bkup-log

mount -lv /dev/sdc1  /mnt/BHbackup
rsync   -avH     /media/BlackHole/ /mnt/BHbackup/ >> /media/logs/bkup-log
umount /mnt/BHbackup

echo -n $(date) >> /media/logs/bkup-log
echo backup finished >> /media/logs/bkup-log