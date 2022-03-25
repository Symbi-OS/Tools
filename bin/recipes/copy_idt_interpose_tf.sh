set -x

TASKSET="taskset -c 1"

IDT_TOOL=../idt_tool

# Get initial idt location
INITIAL_IDT=$($TASKSET $IDT_TOOL -g)

# Allocate and copy idt onto kern page
NEW_IDT=$($IDT_TOOL -c)

# Allocate and copy interposing handler onto kern page
HDL_PG=$($IDT_TOOL -z tf)

# IDT Descriptor to modify
PG_FT_VEC=14

# Stich interposing handler into IDT copy.
./idt_tool -a $NEW_IDT -m addr:$HDL_PG -v $(PG_FT_VEC)

# Key for user
echo want to install the new idt?
echo y\)  $TASKSET $IDT_TOOL -a $NEW_IDT -i
echo x\)  exit

# User cmdline control
while true
do
    echo -n type any char from the table above:
    read input
    case $input in
        y)
            echo INSTALL COPIED IDT
            $TASKSET $IDT_TOOL -a $NEW_IDT -i
            ;;
        x)
            echo exit case
            exit
            ;;
        *)
            echo unexpected input $input, exiting
            exit
            ;;
    esac
done

echo out of loop
exit
