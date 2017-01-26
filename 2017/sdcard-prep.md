### Installing the SD card with Raspian using f2fs

SD cards are not well suited for regular file system, and many brands will
just break after short usage.  See the list of
[Working SD cards](http://elinux.org/RPi_SD_cards)
for other people's experience.  To optimise our changes, Rasperian can be
installed on an f2fs (flash-friendly file system) as follows:

Format the SD card and copy Raspbian onto it,

    parted /dev/sdX
    mklabel msdos
    mkpart primary fat32 0% 200M
    mkpart primary 200M 100%
    quit
    mkfs.vfat -n BOOT -F 32 /dev/sdX1
    mkfs.f2fs /dev/sdX2

    mkdir /mnt/source-boot
    mkdir /mnt/source
    mkdir /mnt/target-boot
    mkdir /mnt/target

    kpartx -a /image-dir/2017-01-11-raspbian-jessie-lite.img
    mount /dev/mapper/loop0p1 /mnt/source-boot
    mount /dev/mapper/loop0p2 /mnt/source
    mount /dev/sdX1 /mnt/target-boot
    mount /dev/sdX2 /mnt/target
    rsync -a /mnt/source-boot/ /mnt/target-boot/
    rsync -a /mnt/source/ /mnt/target/
    umount /mnt/source-boot
    umount /mnt/source
    kpartx -d /image-dir/2015-11-21-raspbian-jessie.img

but leaving `/mnt/target-boot` and `/mnt/target` mounted.  Then update the
file systems in the kernel and system settings:

    sed -i 's/\<ext4\>/f2fs/g' /mnt/target-boot/cmdline.txt
    sed -i 's/\<ext4\>/f2fs/g' /mnt/target/etc/fstab

Optionally prepare for ssh login:

* To `/mnt/target/etc/wpa_supplicant/wpa_supplicant.conf` add:

    network={
        ssid="neic2017"
        psk="XXXXXXXX"
    }

* Remove the default password for the `pi` user from
  `/mnt/target/etc/shadow`, or add your own hash.
* Temporarily add `systemctl start ssh` to `/mnt/target/etc/rc.local` for
  the first boot.
* Add your ssh key to `/mnt/target/root/.ssh/authorized_keys` for the first
  login.  You may create a proper account later.

Finally,

    umount /mnt/target-boot
    umount /mnt/target


### Also Note

* For USB boot `boot_delay=5` or similar may be needed in `cmdline.txt`.
