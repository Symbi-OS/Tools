#!/bin/bash
# Assert that there are 2 command line arguments:
# Which core is getting modified
# If IST is enabled or disabled

if [ $# -ne 2 ]; then
    echo "Usage: $0 <core> <use_ist|no_ist>"
    exit 1
fi

TASKSET="taskset -c $1"

if [ $2 == "use_ist" ]; then
    IST_ENABLE=1
elif [ $2 == "no_ist" ]; then
    IST_ENABLE=0
else
    echo "Usage: $0 <core> <use_ist|no_ist>"
    exit 1
fi

NEW_IDT=$( $TASKSET idt_tool -c )
if [ $IST_ENABLE -eq 1 ]; then
    $TASKSET idt_tool -m ist_enable -v 14 -a $NEW_IDT
else
    $TASKSET idt_tool -m ist_disable -v 14 -a $NEW_IDT
fi

$TASKSET idt_tool -a $NEW_IDT -i
