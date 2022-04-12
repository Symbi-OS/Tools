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

# Elevate and lower in a loop

# Print cr3

# clock

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
