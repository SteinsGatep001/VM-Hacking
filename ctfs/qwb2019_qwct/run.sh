#!/bin/sh
#SYS_DIR=/media/psf/Workspace/System/Qemu/Nevm
SYS_DIR=/shared/CTFs/Qemu/Nevm

./qemu-system-x86_64 -initrd $SYS_DIR/mrootfs.img \
    -nographic -kernel $SYS_DIR/vmlinuz-4.8.0-52-generic -append "priority=low console=ttyS0" \
    -L $SYS_DIR/pc-bios \
    -device e1000,netdev=mynet0 \
    -netdev user,id=mynet0,net=177.168.76.0/24,dhcpstart=177.168.76.10 \
    -device qwb \
    -nographic


    # -netdev user,id=net0,hostfwd=tcp::33333-:22 \
    #-device qwb \
    #-s \
    #-monitor /dev/null
    # -netdev user,id=mynet0,net=192.168.76.0/24,dhcpstart=192.168.76.9 \
    # on guest
    #-fsdev local,id=test_dev,path=/root/shared,security_model=none \
    #-device virtio-9p-pci,fsdev=test_dev,mount_tag=test_mount \
    # mount -t 9p -o trans=virtio test_mount /tmp/shared/ -oversion=9p2000.L,posixacl,cache=loose

