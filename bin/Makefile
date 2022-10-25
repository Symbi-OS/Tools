# Variables inherited from parent
EXECS=idt_tool vm_tool snapshot_tool cr_tool shortcut/sc_lib.so

# You should be able to run 'make' in this dir, this just invokes the parent
# makefile just so we can make use of the inherited variables.
redirect:
	make -C ../ bin

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

shortcut/sc_lib.so: shortcut/sc_lib.c 
	#make -C $(SYMLIB_DIR) dynam_L0
	gcc -shared -fPIC -o $@ $(SYMLIB_LINK) $< -I $(SYMLIB_INCLUDE_DIR)

clean:
	rm -rf $(EXECS)
