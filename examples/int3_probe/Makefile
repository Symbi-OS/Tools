all: int3_probe

#CFLAGS=-O4 -ggdb -static

CFLAGS=-ggdb -O0 -static #-fno-omit-frame-pointer

int3_probe: int3_probe.c #pgtblmgr.cc pgtblmgr.h
	gcc $(CFLAGS) -o int3_probe int3_probe.c ../../include/libsym.a ../../include/kallsymlib/libkallsym.a


cp_to_disk: ./int3_probe System.map
# create tmp dir
	mkdir ./mt
	sudo mount ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sleep 1
	sudo cp $^ ./mt
#cp in the shared obj
	sleep 1
	sudo umount ./mt
	rm -d ./mt

QUEUES = 4
VECTORS = 10
run_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep ip=192.168.19.130:::255.255.255.0::eth0:none isolcpus=0" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio -netdev tap,id=vlan1,ifname=tap0,script=no,downscript=no,vhost=on,queues=$(QUEUES) -device virtio-net-pci,mq=on,vectors=$(VECTORS),netdev=vlan1,mac=02:00:00:04:00:29

run_debug_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep ip=192.168.19.130:::255.255.255.0::eth0:none isolcpus=0" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 2048 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio -s -netdev tap,id=vlan1,ifname=tap0,script=no,downscript=no,vhost=on,queues=$(QUEUES) -device virtio-net-pci,mq=on,vectors=$(VECTORS),netdev=vlan1,mac=02:00:00:04:00:29


clean:
	- rm -rf int3_probe pgtblmgr.o *.o
