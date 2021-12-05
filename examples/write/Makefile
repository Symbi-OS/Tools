CC_FLAGS_STATIC=-O0 -ggdb -mno-red-zone -static
SYM_INCLUDE=-I../../include/headers -I../../include
SYM_LIB=../../include/libsym.a
KALLSYM_INCLUDE=-I../../include/kallsymlib/
KALLSYM_LIB=../../include/kallsymlib/libkallsym.a

all: write_syscall_loop_static write_syscall_static

write_syscall_static: write_syscall_static.c
	$(CC) $(CC_FLAGS_STATIC) -o $@ $^ $(SYM_INCLUDE) $(SYM_LIB)

write_syscall_loop_static: write_syscall_loop.c $(SYM_LIB) $(KALLSYM_LIB)
	$(CC) $(CC_FLAGS_STATIC) -DSTATIC_BUILD -o $@ $^ $(SYM_INCLUDE) $(KALLSYM_INCLUDE)

cp_to_disk: ./write_syscall_static ./write_syscall_loop_static
# create tmp dir
	mkdir ./mt
	sudo mount -o loop=/dev/loop0 ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sleep 1
	sudo cp $^ ./mt
#cp in the shared obj
	sleep 1
	sudo umount ./mt
	rm -d ./mt

run_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 1024 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio

run_debug_virt:
	sudo qemu-system-x86_64 -smp 1 -enable-kvm -kernel /boot/vmlinuz-5.14.0-symbiote+ -nographic --append "console=ttyS0 root=/dev/sda nopti mitigations=off nosmap nosmep" -hda ~tommyunger/ukl/min-initrd/min-initrd.d/root -m 2048 -initrd ~tommyunger/ukl/min-initrd/min-initrd.d/initrd -nodefaults -nographic -serial stdio -s

clean:
	-rm -rf write_syscall_static
	-rm -rf write_syscall_loop_static
	-rm -rf *.o
