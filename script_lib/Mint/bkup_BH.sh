#!/bin/bash
export PATH=$PATH:/usr/local/myscripts

echo -n $(date) >> /media/logs/bkup-log.log
echo 'BlackHole backup started' >> /media/logs/bkup-log.log

mount -lv /dev/sdc1  /mnt/BlackHole_bkup
rsync   -avH  --delete  /media/BlackHole/ /mnt/BlackHole_bkup/ >> /media/logs/bkup-log.log
umount /mnt/BlackHole_bkup

echo -n $(date) >> /media/logs/bkup-log.log
echo 'Abyss backup started' >> /media/logs/bkup-log.log

mount -lv /dev/sdc1  /mnt/Abyss_bkup
rsync   -avH  --delete  /media/Abyss/ /mnt/Abyss_bkup/ >> /media/logs/bkup-log.log
umount /mnt/Abyss_bkup

echo -n $(date) >> /media/logs/bkup-log.log
echo ' backup finished' >> /media/logs/bkup-log.log
echo '**********************************************' >> /media/logs/bkup-log.log