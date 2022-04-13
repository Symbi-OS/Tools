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

target:
	for number in 1 2 3 4 ; do echo $$number ; done

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
clock:
	$(CL) 1

# Steal syscall handler

# Stack starvation interpostions

# text fault interposition

# int3 interposition

# read

# write

# read short cut

# write short cut

# getppid shortcut

# fork

# mmap shortcup

# stack starvation mitigation

# lebench

# echo server

# redis

# memcached
