all: client_RW server_RW

CFLAGS=-O4

#CFLAGS=-ggdb -O0 -fno-omit-frame-pointer

# Both easily saturate network.
client_RW: client.c
	gcc -DUSE_READ_WRITE $(CFLAGS) -o $@ $^

client_SR: client.c
	gcc -DUSE_SEND_RECV $(CFLAGS) -o $@ $^

server_RW: server.c
	gcc -DUSE_READ_WRITE $(CFLAGS) -o $@ $^

server_SR: server.c
	gcc -DUSE_SEND_RECV $(CFLAGS) -o $@ $^


cp_to_disk: ./server_RW ./client_RW
# create tmp dir
	mkdir ./mt
	sudo mount ../../../min-initrd/min-initrd.d/root ./mt
	sleep 1
	sudo cp ./server_RW ./mt
#cp in the shared obj
	sleep 1
	sudo umount ./mt

	sudo mount ../../../min-initrd/min-initrd.d/root2 ./mt
	sleep 1
	sudo cp ./client_RW ./mt
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
	- rm -rf client_*
	- rm -rf server_*
