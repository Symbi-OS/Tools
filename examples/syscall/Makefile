CC=gcc
CC_FLAGS=-static -O0 -ggdb -mno-red-zone
CC_FLAGS_DYNAMIC=-O0 -ggdb -mno-red-zone
.PHONY=all

MY_CC_FLAGS=-O0 -ggdb -mno-red-zone #-Wall #-Wfatal-errors
MY_STATIC_FLAG=-static

MY_INCLUDES=-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include

sym_sys: sym_sys.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../../include/sym_lib_syscall.c

run:
	taskset -c 0 ./sym_sys

run_virt:
	sudo qemu-system-x86_64 -smp 4 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda mitigations=off isolcpus=0 nosmep nosmap" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio

run_virt_db:
	sudo qemu-system-x86_64 -s -S -smp 4 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda mitigations=off isolcpus=0 nosmep nosmap" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio


cp_to_disk: ./sym_sys
# create tmp dir
	mkdir ./mt
	sudo mount -o loop=/dev/loop0 ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sudo cp sym_sys ./mt
	sleep 1
	sudo umount ./mt
	rm -d ./mt

clean:
	-rm sym_sys
