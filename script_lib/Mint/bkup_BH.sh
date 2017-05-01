#!/bin/bash
export PATH=$PATH:/usr/local/myscripts

echo -n $(date) >> /media/logs/bkup-log.log
echo ' backup started' >> /media/logs/bkup-log.log

mount -lv /dev/sdc1  /mnt/BlackHole_bkup
rsync   -avH     /media/BlackHole/ /mnt/BlackHole_bkup/ >> /media/logs/bkup-log.log
umount /mnt/BlackHole_bkup

echo -n $(date) >> /media/logs/bkup-log.log
echo ' backup finished' >> /media/logs/bkup-log.log
echo '**********************************************' >> /media/logs/bkup-log.log