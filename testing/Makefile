# Test symbiote functionality in roughly more challenging contexts.
EX=../examples/

# Elevate and lower
EL=$(EX)/elevate_lower/elevate_lower

all_elevate_lower: elevate_lower elevate_lower_loop elevate_lower_bash_loop

elevate_lower:
	$(EL) 1
	echo done

elevate_lower_loop:
	$(EL) 1000000
	echo done

elevate_lower_bash_loop:
	for i in $$(seq 1 1000) ; do $(EL) 1000 ; done
	echo done

# Print cr3
P3=$(EX)/print/print_cr3_static

all_print_cr3: print_cr3 print_cr3_loop print_cr3_bash_loop

print_cr3:
	$(P3) 1

print_cr3_loop:
	$(P3) 100000000

print_cr3_bash_loop:
	for i in $$(seq 1 100); do $(P3) 100000; done

# clock
CL=$(EX)/clock/clock
all_clock: clock clock_loop clock_bash_loop

clock:
	$(CL) 1

clock_loop:
	$(CL) 10000000

clock_bash_loop:
	for i in $$(seq 1 1000); do $(CL) 10000; done

GP=$(EX)/getppid/getppid
all_getppid: getppid getppid_loop getppid_bash_loop
# getppid shortcut
getppid:
	$(GP) 1

getppid_loop:
	$(GP) 10000000

getppid_bash_loop:
	for i in $$(seq 1 10); do $(GP) 100000; done

# lebench
LE_DIR=../../new_lebench
lebench:
	make -C $(LE_DIR) sym_run_all

# read


# write

# read short cut

# write short cut


# fork

# mmap shortcup

# stack starvation mitigation

# Stack starvation interpostions

# text fault interposition

# int3 interposition


# echo server

# redis

# memcached

# Steal syscall handler
