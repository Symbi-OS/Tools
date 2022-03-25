./idt_tool -c
./idt_tool -z df
./idt_tool -a 0xffffc900008e5000 -m addr:0xffffc90000902000 -v 8
taskset -c 1 ./idt_tool -a 0xffffc900008e5000 -i
