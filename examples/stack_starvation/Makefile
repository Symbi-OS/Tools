CC=gcc
CC_FLAGS=-ggdb -Werror -static #-mno-red-zone
INCLUDE_DIR=../../include/

VECTORS = 10
QUEUES = 4
NETWORK = -netdev tap,id=vlan1,ifname=tap0,script=no,downscript=no,vhost=on,queues=$(QUEUES) -device virtio-net-pci,mq=on,vectors=$(VECTORS),netdev=vlan1,mac=02:00:00:04:00:29

stack_starvation: stack_starvation.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../../include/libsym.a

run:
	taskset -c 0 ./stack_starvation

# run_virt:
# 	sudo qemu-system-x86_64 -smp 4 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nosmep nosmap mitigations=off isolcpus=0" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio

#KVM=-enable-kvm
run_virt:
	sudo qemu-system-x86_64 -smp 4 $(KVM) -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio

run_debug_virt:
	sudo qemu-system-x86_64 -smp 1 $(KVM) -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 2048 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio -s

run_virt_net:
	sudo qemu-system-x86_64 -smp 4 $(KVM) -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio $(NETWORK)

cp_to_disk: ./stack_starvation
# create tmp dir
	mkdir ./mt
	sudo mount -o loop=/dev/loop0 ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sleep 1
	sudo cp $< ./mt
	sleep 1
	sudo umount ./mt
	rm -d ./mt

clean:
	-rm -rf stack_starvation
