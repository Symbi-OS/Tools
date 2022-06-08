# Test symbiote functionality in roughly more challenging contexts.
EX=../examples/

all: sudo_check rebuild_all mitigate_cores all_clock all_elevate_lower all_getppid all_print_cr3 lebench stack_starvation trivial_int3

sudo_check:
	sudo echo hi
rebuild_all:
	make -C ../

# Elevate and lower
EL=$(EX)/elevate_lower/elevate_lower

all_elevate_lower: elevate_lower elevate_lower_loop elevate_lower_bash_loop

elevate_lower: .mitigated_all
	$(EL) 1
	echo done

elevate_lower_loop: .mitigated_all
	$(EL) 1000000
	echo done

elevate_lower_bash_loop: .mitigated_all
	for i in $$(seq 1 1000) ; do $(EL) 1000 ; done
	echo done

# Print cr3
P3=$(EX)/print/print_cr3_static

all_print_cr3: .mitigated_all print_cr3 print_cr3_loop print_cr3_bash_loop

print_cr3: .mitigated_all
	$(P3) 1

print_cr3_loop: .mitigated_all
	$(P3) 100000000

print_cr3_bash_loop: .mitigated_all
	for i in $$(seq 1 100); do $(P3) 100000; done

# clock
CL=$(EX)/clock/clock
all_clock: .mitigated_all clock clock_loop clock_bash_loop

clock: .mitigated_all
	$(CL) 1

clock_loop: .mitigated_all
	$(CL) 10000000

clock_bash_loop: .mitigated_all
	for i in $$(seq 1 1000); do $(CL) 10000; done

GP=$(EX)/getppid/getppid
all_getppid: .mitigated_all getppid getppid_loop getppid_bash_loop
# getppid shortcut
getppid:
	$(GP) 1

getppid_loop: .mitigated_all
	$(GP) 10000000

getppid_bash_loop: .mitigated_all
	for i in $$(seq 1 10); do $(GP) 100000; done

# IDT tool
IDT=../bin/idt_tool
idt_get:
	taskset -c 0 $(IDT) -g

# hardcoding
idt_set:
	taskset	-c 0 $(IDT) -a fffffe0000000000 -i
	rm .mitigated_all

# Identity, get the idt and set it.
idt_get_set:
	$(eval IDT_ADDR=$(shell sh -c "taskset -c 0 $(IDT) -g" ))
	taskset -c 0 $(IDT) -a $(IDT_ADDR) -i

idt_get_set_loop:
	for i in $$(seq 0 10); do make idt_get_set; done

idt_get_loop:
	for i in $$(seq 0 $$(( $$(nproc) - 1 )) ); do echo $$i && taskset -c $$i $(IDT) -g; done

idt_set_loop:
	$(eval IDT_ADDR=$(shell sh -c "taskset -c 0 $(IDT) -g" ))
	for i in $$(seq 0 $$(( $$(nproc) - 1 )) ); do echo $$i && taskset -c $$i $(IDT) -a $(IDT_ADDR) -i; done

MG=../bin/recipes/
mitigate_cores:
	cd $(MG) && ./mitigate_all.sh
	touch .mitigated_all

mitigate_cores_loop:
	for i in $$(seq 0 100); do make mitigate_cores && make idt_reset; done


# Happens to reset all cores to default idt
idt_reset:
	make idt_set
	make idt_set_loop
	rm .mitigated_all


# TEST=$$(echo hi)
# VAL=dog
# idt_test:
# # echo $(IDT_ADDR)
# # $(TEST)
# 	$(eval VAL= $(shell sh -c "echo cat"))
# # VAL=$(echo hi) ; echo $$VAL
# 	echo $(VAL)
# 	VAL=$$(echo bye)
# 	echo $(VAL)

# getpodname:
#     # Getting pod name
#     @eval $$(minikube docker-env) ;\
#     $(eval PODNAME=$(shell sh -c "kubectl get pods | grep profile-posts-api | grep Running" | awk '{print $$1}'))
#     echo $(PODNAME)

# dependency on .PHONY prevents Make from 
# thinking there's `nothing to be done`
# set_opts: .PHONY
#   $(eval DOCKER_OPTS = -v $(shell mktemp -d -p /scratch):/output)

# lebench
LE_DIR=../../new_lebench
lebench:
	make -C $(LE_DIR) sym_clean_all
	make -C $(LE_DIR) sym_run_all

# read


# write

# read short cut

# write short cut


# fork

# mmap shortcup

# stack starvation mitigation
SS=$(EX)/stack_starvation/stack_starvation
stack_starvation: .mitigated_all
	$(SS)





# Stack starvation interpostions

# text fault interposition

# int3 interposition

TI3=$(EX)/int3_probe/trivial_int3
trivial_int3:
	$(TI3)

# echo server

# redis

# memcached

# Steal syscall handler
