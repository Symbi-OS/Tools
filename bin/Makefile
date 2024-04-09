# Variables inherited from parent
EXECS=idt_tool vm_tool snapshot_tool cr_tool shortcut/sc_lib.so shortcut/sc_lib.o shortcut/deep_sc/deep_sc.o

# Variable to toggle deep shortcutting, undefine if doing shallow shortcuts
DEEP_SC_DEFINE = "-UDEEP_SHORTCUT"

# You should be able to run 'make' in this dir, this just invokes the parent
# makefile just so we can make use of the inherited variables.
redirect:
	make -C ../ bin

%.o: %.c
	$(CC) -c $(CFLAGS) -I $(SYMLIB_INCLUDE_DIR) $(DEEP_SC_DEFINE) -fPIC -o $@ $<

shortcut/sc_lib.o: shortcut/sc_lib.c shortcut/sc_lib.h

shortcut/deep_sc/deep_sc.o: shortcut/deep_sc/deep_sc.c shortcut/deep_sc/deep_sc.h

# Don't call this target directly
all: $(EXECS)

cr_tool: cr_tool.c cr_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

vm_tool: vm_tool.c vm_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

idt_tool: idt_tool.c idt_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

snapshot_tool: snapshot_tool.c snapshot_tool.h | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $< $(SYMLIB_LINK) -I $(SYMLIB_INCLUDE_DIR)

shortcut/sc_lib.so: shortcut/sc_lib.c shortcut/sc_lib.h
	gcc $(CFLAGS) -shared -fPIC -o $@ $(SYMLIB_LINK) $^ $(DEEP_SC_DEFINE)

clean:
	rm -rf $(EXECS)
