:~$ cat /etc/init.d/S50reboot

#!/bin/sh

sleep 5
i=$RANDOM
j=$(( $i % 2))

if [ $j -eq 0 ] ; then
        mount /dev/mmcblk0p1 /mnt

        count=`cat /mnt/cnt`
        echo "KEXEC rebootng to BE count = $count"
        echo $RANDOM > /mnt/"$count""_BE"

        count=$(( $count + 1 ))
        echo "$count">/mnt/cnt

        kexec -l /mnt/vmlinux_BE.strip --command-line="console=ttyS0,115200 earlyprintk=uart8 250-32bit,0x1c020000 debug swiotlb=65536 log_buf_len=2M"
        umount /mnt
        kexec -e
else
        mount /dev/mmcblk0p1 /mnt

        count=`cat /mnt/cnt`
        echo "KEXEC rebooting to LE count = $count"
        echo $RANDOM > /mnt/"$count""_LE"

        count=$(( $count + 1 ))
        echo "$count">/mnt/cnt

        kexec -l /mnt/vmlinux_LE.strip --command-line="console=ttyS0,115200 earlyprintk=uart8 250-32bit,0x1c020000 debug swiotlb=65536 log_buf_len=2M"
        umount /mnt
        kexec -e
fi

exit $?
