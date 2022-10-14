CC=gcc
CFLAGS= -O0 -ggdb -Wall -Wextra -static -mno-red-zone

SYMLIB_DIR=../../Symlib
SYMLIB=$(SYMLIB_DIR)/build/libsym.a
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include

KALLSYMDIR=../../kallsymlib/
KALLSYMLIB=../../kallsymlib/libkallsym.a

VM_DISK=../../min-initrd/min-initrd.d/root

DEPENDENCY_LIBS=$(KALLSYMLIB) $(SYMLIB)

# TODO: Add all executables here.
# Stack Starvation DIR
SSD=stack_starvation
ELO=elevate_lower
TFD=text_fault
I3D=int3_probe
DBP=db_probe
P3D=print
GPD=getppid
POLLD=sym_poll
ECO=echo_server
CLK=clock
GSM=gs_mitigate

# Tools
IDT=../bin/


EXECS=$(SSD)/stack_starvation $(TFD)/text_fault $(I3D)/int3_probe $(I3D)/trivial_int3 $(P3D)/print_cr3_static \
$(GPD)/getppid $(POLLD)/poller $(ECO)/client_RW $(ECO)/server_RW $(IDT)/idt_tool $(IDT)/cr_tool $(IDT)/vm_tool $(IDT)/snapshot_tool $(CLK)/clock \
	$(ELO)/elevate_lower $(GSM)/gs_mitigate $(DBP)/db_probe $(DBP)/db_probe_local $(DBP)/test \
	$(DBP)/db_probe_reg

all: $(EXECS)


$(GSM)/gs_mitigate: $(GSM)/gs_mitigate.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(ELO)/elevate_lower: $(ELO)/elevate_lower.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(IDT)/cr_tool: $(IDT)/cr_tool.c $(IDT)/cr_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(IDT)/vm_tool: $(IDT)/vm_tool.c $(IDT)/vm_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(IDT)/idt_tool: $(IDT)/idt_tool.c $(IDT)/idt_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(IDT)/snapshot_tool: $(IDT)/snapshot_tool.c $(IDT)/snapshot_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/client_RW: $(ECO)/client.c | $(DEPENDENCY_LIBS)
	$(CC) -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/server_RW: $(ECO)/server.c | $(DEPENDENCY_LIBS)
	gcc -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(CLK)/clock: $(CLK)/clock.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(POLLD)/poller: $(POLLD)/poller.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(GPD)/getppid: $(GPD)/getppid.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(P3D)/print_cr3_static: $(P3D)/print_cr3_static.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(SSD)/stack_starvation: $(SSD)/stack_starvation.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(TFD)/text_fault: $(TFD)/text_fault.c $(TFD)/foo.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(I3D)/int3_probe: $(I3D)/int3_probe.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(I3D)/trivial_int3: $(I3D)/trivial_int3.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe: $(DBP)/db_probe.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe_local: $(DBP)/db_probe_local.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe_reg: $(DBP)/db_probe_reg.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/test: $(DBP)/test.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

clean:
	rm -rf $(EXECS)

