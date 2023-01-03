CC=gcc
CFLAGS= -O3 -ggdb -Wall -Wextra -mno-red-zone -lpthread -lrt
export CC CFLAGS

#Check for kernel version
UNAME=$(shell uname -r)
USER=$(shell whoami)

ifeq ($(TYPE), static)
	SYMLIB_LINK=$(SYMLIB_STATIC)
endif

RUN_ITERATIONS := 10000
export RUN_ITERATIONS

all: server server_killer independent_client ipc_shortcut.so

# GET TARGET INFO

SYS_INFO="sys_info/"

info:
	mkdir -p $(SYS_INFO)

info_cmdline:
	cat /proc/cmdline > $(SYS_INFO)cmdline.info

info_ifconfig:
	ifconfig > $(SYS_INFO)ifconfig.info

info_nproc:
	nproc > $(SYS_INFO)nproc.info

info__uptime:
	uptime > $(SYS_INFO)uptime.info

info_cpu:
	cat /proc/cpuinfo > $(SYS_INFO)cpu.info

info_uname:
	uname -srm > $(SYS_INFO)uname.info

info_ps:
	echo "NPROC: " > $(SYS_INFO)ps.info
	ps aux | wc -l; ps aux >> $(SYS_INFO)ps.info

expe_info: info info_cmdline info_ifconfig info_nproc info__uptime info_cpu info_uname info_ps

server: server.c ipc.c
	$(CC) $(CFLAGS) $^ -o $@

independent_client: client.c
	$(CC) $(CFLAGS) $^ -o $@ -D INDEPENDENT_CLIENT

server_killer: server_killer.c ipc.c
	$(CC) $(CFLAGS) $^ -o $@

ipc_shortcut.so: ipc_shortcut.c ipc.c
	$(CC) $(CCFLAGS) -shared -fPIC $^ -o $@

run_independent_client: independent_client
	taskset -c 1 ./independent_client $(RUN_ITERATIONS)

run_client: independent_client
	taskset -c 1 bash -c 'LD_PRELOAD=./ipc_shortcut.so ./independent_client $(RUN_ITERATIONS)'

run_2client: independent_client
	taskset -c 1 bash -c 'LD_PRELOAD=./ipc_shortcut.so ./independent_client $(RUN_ITERATIONS)' &
	taskset -c 2 bash -c 'LD_PRELOAD=./ipc_shortcut.so ./independent_client $(RUN_ITERATIONS)'

run_server: server
	taskset -c 0 ./server $(THREADS)

run_server_elevated: server
	taskset -c 0 bash -c 'shortcut.sh -be -s "write->ksys_write" -- ./server $(THREADS)'

kill_server: server_killer
	./server_killer $(THREADS)

copy_binaries_to_artifacts: server server_killer ipc_shortcut.so
	\cp server server_killer ipc_shortcut.so /home/root/Symbi-OS/artifacts/ipc_interposer/ 

clean:
	rm -rf server server_killer independent_client ipc_shortcut.so run_log scalability.log sys_info/
	rm -f /dev/shm/sym_server_shm
