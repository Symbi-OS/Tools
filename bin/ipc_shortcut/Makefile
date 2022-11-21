CC := gcc
CCFLAGS := -O3 -Wall

USER=$(shell whoami)
INCLUDE_DIR := /home/$(USER)/Symbi-OS/LinuxPrototypes/shmem_ipc

ipc_shortcut.so: ipc_shortcut.c $(INCLUDE_DIR)/ipc.c
	$(CC) $(CCFLAGS) -shared -fPIC -I$(INCLUDE_DIR) $^ -o $@

clean:
	rm -rf ipc_shortcut.so
