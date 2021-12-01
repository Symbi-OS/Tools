CC_FLAGS_DYNAMIC=-O0 -ggdb -mno-red-zone -static

write_syscall_static: write_syscall_static.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ ../../include/sym_lib_syscall.c

cp_to_disk: ./write_syscall_static
# create tmp dir
	mkdir ./mt
	sudo mount -o loop=/dev/loop0 ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sleep 1
	sudo cp $< ./mt
#cp in the shared obj
	sudo cp ../../include/libelevate_cons.so ./mt
	sleep 1
	sudo umount ./mt
	rm -d ./mt

run_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio

run_debug_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 2048 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio -s

clean:
	-rm -rf write_syscall_static
