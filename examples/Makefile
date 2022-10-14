# Variables included from parent makefile. Don't execute this directly.

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
SCT=shortcut

EXECS=$(SSD)/stack_starvation $(TFD)/text_fault $(I3D)/int3_probe $(I3D)/trivial_int3 \
	$(P3D)/print_cr3_static $(GPD)/getppid $(POLLD)/poller \
	$(ECO)/client_RW $(ECO)/server_RW $(ECO)/client_SR $(ECO)/server_SR \
	$(ELO)/elevate_lower $(GSM)/gs_mitigate $(DBP)/db_probe $(DBP)/db_probe_local $(DBP)/test \
	$(DBP)/db_probe_reg $(CLK)/clock $(SCT)/shortcut_test

# You should be able to run 'make' in this dir, this just invokes the parent
# makefile just so we can make use of the inherited variables.
redirect:
	make -C ../ examples

all: $(EXECS)

test: $(ELO)/elevate_lower

$(GSM)/gs_mitigate: $(GSM)/gs_mitigate.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(ELO)/elevate_lower: $(ELO)/elevate_lower.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/client_RW: $(ECO)/client.c | $(DEPENDENCY_LIBS)
	$(CC) -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/server_RW: $(ECO)/server.c | $(DEPENDENCY_LIBS)
	gcc -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/client_SR: $(ECO)/client.c | $(DEPENDENCY_LIBS)
	$(CC) -DUSE_SEND_RECV $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(ECO)/server_SR: $(ECO)/server.c | $(DEPENDENCY_LIBS)
	gcc -DUSE_SEND_RECV $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(CLK)/clock: $(CLK)/clock.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(POLLD)/poller: $(POLLD)/poller.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(GPD)/getppid: $(GPD)/getppid.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(P3D)/print_cr3_static: $(P3D)/print_cr3_static.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(SSD)/stack_starvation: $(SSD)/stack_starvation.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

$(TFD)/text_fault: $(TFD)/text_fault.c $(TFD)/foo.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(I3D)/int3_probe: $(I3D)/int3_probe.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(I3D)/trivial_int3: $(I3D)/trivial_int3.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe: $(DBP)/db_probe.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe_local: $(DBP)/db_probe_local.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/db_probe_reg: $(DBP)/db_probe_reg.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(DBP)/test: $(DBP)/test.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(SCT)/shortcut_test: $(SCT)/shortcut_test.c
	$(CC) -o $@ $^

clean:
	rm -rf $(EXECS)

