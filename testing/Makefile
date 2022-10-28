# XXX Mitigate all sometimes fails. Lots of cores spinning on a ?lock?

# Test symbiote functionality in roughly more challenging contexts.
EX=../examples/

# Want these in order, think undefined by make.
all: sync sudo_check check_proc_cmdline check_fsgsbase rebuild_all mitigate_cores all_clock all_elevate_lower all_getppid all_print_cr3 lebench stack_starvation trivial_int3 run_db run_db_reg

# Remove lebench which has an intermittant failure on send recv
most: sync sudo_check check_proc_cmdline check_fsgsbase rebuild_all mitigate_cores all_clock all_elevate_lower all_getppid all_print_cr3 stack_starvation trivial_int3 run_db run_db_reg

# 1 step toward protecting fs from panic
sync:
	@echo sync
	sync
	sync
	sync

sudo_check:
	sudo echo hi

rebuild_all:
	@echo building lib and apps.
	make -C ../ &> /dev/null

NOSMAP := $(shell cat /proc/cmdline | grep nosmap | wc -l)
NOSMEP := $(shell cat /proc/cmdline | grep nosmep | wc -l)
MITIGATIONS_OFF := $(shell cat /proc/cmdline | grep mitigations=off | wc -l)

FSGSBASE := $(shell cat /proc/cpuinfo | grep fsgsbase | uniq | wc -l)

check_proc_cmdline:
# @if [ $(NOSMAP) = "1" ]; then\
# 	echo "NOSMAP is good";\
# else \
# 	echo "NOSMAP did not appear!";\
# 	exit 42;\
# fi
# @if [ $(NOSMEP) = "1" ]; then\
# 	echo "NOSMEP is good";\
# else \
# 	echo "NOSMEP did not appear!";\
# 	exit 42;\
# fi
	@if [ $(MITIGATIONS_OFF) = "1" ]; then\
		echo "MITIGATIONS_OFF is good";\
	else \
		echo "MITIGATIONS_OFF did not appear!";\
		exit 42;\
	fi

check_fsgsbase:
	@if [ $(FSGSBASE) = "1" ]; then\
		echo "FSGSBASE is good";\
	else \
		echo "FSGSBASE did not appear!";\
		exit 42;\
	fi


# Elevate and lower
EL=$(EX)/elevate_lower/elevate_lower

all_elevate_lower: elevate_lower elevate_lower_loop elevate_lower_bash_loop p_elevate_lower

elevate_lower: .mitigated_all
	$(EL) 1
	echo done

elevate_lower_loop: .mitigated_all
	$(EL) 1000000
	echo done

elevate_lower_bash_loop: .mitigated_all
	for i in $$(seq 1 1000) ; do $(EL) 1000 &> /dev/null ; done
	echo done

p_elevate_lower: .mitigated_all
	./parallel_e_l.sh 30000

# Print cr3
P3=$(EX)/print/print_cr3_static

all_print_cr3: .mitigated_all print_cr3 print_cr3_loop print_cr3_bash_loop

print_cr3: .mitigated_all
	$(P3) 1

print_cr3_loop: .mitigated_all
	$(P3) 100000000

print_cr3_bash_loop: .mitigated_all
	for i in $$(seq 1 100); do $(P3) 100000 &> /dev/null ; done

# clock
CL=$(EX)/clock/clock
all_clock: .mitigated_all clock clock_loop clock_bash_loop

clock: .mitigated_all
	$(CL) 1

clock_loop: .mitigated_all
	$(CL) 10000000

clock_bash_loop: .mitigated_all
	for i in $$(seq 1 1000); do $(CL) 10000 &> /dev/null; done

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
	cd $(MG) && ./mitigate_all.sh &> /dev/null
	touch .mitigated_all

mitigate_cores_loop:
	for i in $$(seq 0 100); do make mitigate_cores && make idt_reset; done


# Happens to reset all cores to default idt
idt_reset:
	make idt_set
	make idt_set_loop
	rm .mitigated_all

# lebench
LE_DIR=../../new_lebench
lebench:
	@echo doing long lebench tests.
	make -C $(LE_DIR) sym_clean_all &> /dev/null
	@echo survived elevated, now try shortcutted... allow 1 min, but have observed it hanging....
	make -C $(LE_DIR) sym_run_all &> /dev/null

# stack starvation mitigation
SS=$(EX)/stack_starvation/stack_starvation
stack_starvation: .mitigated_all
	$(SS)

# text fault interposition
# NOTE: NYI because can't repro text fault...
# text_fault:


# int3 interposition
TI3=$(EX)/int3_probe/trivial_int3
trivial_int3:
	$(TI3)

DBP=$(EX)/db_probe/
run_db:
	$(DBP)/db_probe 0 1

run_db_reg:
	$(DBP)/db_probe_reg

SHORTCUT=../bin/shortcut/shortcut
SC_FLAGS=-w ksys -r ksys
SCT=$(EX)/shortcut/shortcut_test
time_shortcut:
	time taskset -c 0 sh -c "./$(SHORTCUT) $(SC_FLAGS) -a ./$(SCT)"

time_no_shortcut:
	time ./$(SCT)

perf_shortcut:
	perf stat taskset -c 0 sh -c "./$(SHORTCUT) $(SC_FLAGS) -a ./$(SCT)"

perf_no_shortcut:
	perf stat ./$(SCT)
# parallel

# echo server

# redis

# memcached

# Steal syscall handler
