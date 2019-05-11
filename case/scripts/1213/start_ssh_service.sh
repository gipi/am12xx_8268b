
#!/usr/bin/sh
chrootPath="/chroot/dropbear"
echo "Stop SSH dropbear service"
pidof dropbear|xargs kill -9

echo "Start SSH dropbear service"
mkdir /dev/pts
mount -t devpts none /dev/pts
busybox telnetd -F &

#chroot setting
#create chroot folder
mkdir -p ${chrootPath}
cd ${chrootPath}
mkdir -p dev/pts proc etc/dropbear lib usr/lib var/run var/log

#generate SSH key
if [ ! -f /etc/dropbear_dss_host_key ] ; then
	echo Generating DSS Key...
	dropbeargen -t rsa -f /etc/dropbear/dropbear_dss_host_key
fi
if [ ! -f /etc/dropbear_rsa_host_key ] ; then
	echo Generating RSA Key...
	dropbeargen -t rsa -f /etc/dropbear/dropbear_rsa_host_key
fi
cp /etc/dropbear/dropbear_dss_host_key /chroot/dropbear/etc/dropbear/
cp /etc/dropbear/dropbear_rsa_host_key /chroot/dropbear/etc/dropbear/
cp /am7x/bin/dropbear /chroot/dropbear/etc/dropbear/

#copy nessary lib for ssh chroot
cd /chroot/dropbear
cp /lib/libcrypt.so.1 lib/
cp /lib/libutil.so.1 lib/
cp /am7x/lib/libz.so.1 lib/
cp /lib/libc.so.6 lib/
cp /lib/ld.so.1 lib/
cp /etc/resolv.conf etc/
touch var/log/lastlog
touch var/run/utmp
touch var/log/wtmp

#chroot device setting
mknod dev/urandom c 1 9
chmod 0666 dev/urandom
mknod dev/ptmx c 5 2
chmod 0666 dev/ptmx
mknod dev/tty c 5 0
chmod 0666 dev/tty

mount -o bind /dev/pts dev/pts/
mount -o bind /proc proc/

#add ssh user
#grep ^xactions /etc/passwd > etc/passwd
#grep ^xactions /etc/group > etc/group
#grep ^xactions /etc/shadow > etc/shadow
#mkdir -p home/xactions
#chown xactions.xactions home/xactions

#start chroot ssh service
chroot /chroot/dropbear/ \
/etc/dropbear/dropbear \
-d /etc/dropbear/dropbear_dss_host_key \
-r /etc/dropbear/dropbear_rsa_host_key \
-m -w -g

#/am7x/bin/dropbear
cd ${chrootPath}/etc
dropbear -F &
cd /
