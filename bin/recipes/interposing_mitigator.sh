#!/bin/bash

# Exit on error
set -e

PRINT_RED=1
function print_color () {
    echo -n "$(tput setaf $2)$1"
    # reset
    echo $(tput setaf 7)
}

function print_help () {
    echo script for performing interrupt interposition mitigations
    echo "flags: -m \"tf\df\" -t <core> -d <debug> "
    echo
    echo examples:
    echo ./interposing_mitigator.sh -m tf -t 1 -d
    echo ./interposing_mitigator.sh -m df -t 1 -d
    echo ./interposing_mitigator.sh -m i3 -t 1 -d
    echo ./interposing_mitigator.sh -m db -t 1 -d
}
function parse_args () {
    # Call getopt to validate the provided input. 
    while getopts "dhm:t:i:" OPTION; do
        case $OPTION in
            d)
                DEBUG=1
                ;;
            h)
                HELP=1
                print_help
                exit 0
                ;;
            m)
                MITIGATION=$OPTARG
                case $MITIGATION in
                    "tf")
                        VECTOR=14
                        ;;
                    "df")
                        VECTOR=8
                        ;;
                    "i3")
                        VECTOR=3
                        ;;
                    "db")
                        VECTOR=1
                        ;;
                    *)
                        echo "Incorrect -m option provided:" $MITIGATION
                        echo bad mitigation: $MITIGATION
                        ;;
                esac
                ;;
            t)
                TASKSET_CORE=$OPTARG
                ;;
	    i)
	        IST=$OPTARG
		case $IST in
		    "OFF")
		        disable_ist
			;;
	            "ON")
			enable_ist
			;;
		esac
		;;
            # v)
            #     VECTOR=$OPTARG
            #     ;;
            *)
                echo "Incorrect options provided"
                exit 1
                ;;
        esac
    done

    # Need all.
    if  [[ ( -z "$MITIGATION"  &&  -z "$IST" ) ||  -z "$TASKSET_CORE" ]]; then
        echo dont have all necessary args
        echo expect : -m "{\"df\"|\"tf\"}" -t "[ 0, (nproc-1)]"
        exit -1
    fi

    if [ ! -z "$DEBUG" ] && [ ! -z "$MITIGATION" ]; then
        echo Apply $MITIGATION mitigation on core $TASKSET_CORE
    fi
}

function get_cur_idt () {
    # Get initial idt location
    GET_CUR_IDT="$TASKSET $IDT_TOOL -g"

    if [ ! -z "$DEBUG" ]; then
        echo
        echo about to get initial idt:
        print_color "$GET_CUR_IDT" $PRINT_RED
    fi

    INITIAL_IDT=$($GET_CUR_IDT)

    if [ ! -z "$DEBUG" ]; then
        echo initial idt: $INITIAL_IDT
    fi
}

function cp_current_idt () {
    GET_NEW_IDT="$TASKSET $IDT_TOOL -c"

    if [ ! -z "$DEBUG" ]; then
        echo
        echo allocate kern page
        echo copy page $INITIAL_IDT
        print_color "$GET_NEW_IDT" $PRINT_RED
    fi

    NEW_IDT=$($GET_NEW_IDT)
    # Allocate and copy idt onto kern page

    if [ ! -z "$DEBUG" ]; then
        echo onto page $NEW_IDT
    fi
}

function cp_hdl () {
    # TODO: finish this.
    # Allocate and copy interposing handler onto kern page
    GET_HDL_PG="$IDT_TOOL -z $MITIGATION"

    if [ ! -z "$DEBUG" ]; then
        echo
        echo allocate kern page
        echo copy $MITIGATION handler
        print_color "$GET_HDL_PG" $PRINT_RED
    fi
    HDL_ARGS=$($GET_HDL_PG)
    IFS=' '
    read -ra HDL_PARSED_ARGS <<< "$HDL_ARGS"
    HDL_PG=${HDL_PARSED_ARGS[0]}
    SCRATCH_PG=${HDL_PARSED_ARGS[1]}

    if [ ! -z "$DEBUG" ]; then
        echo onto page $HDL_PG
	echo with scratchpad page $SCRATCH_PG
    fi

    # store handler pg and scratchpad pg locations in metadata files

    HDL_INFO_DIR="$HOME/sym_metadata"
    mkdir -p "$HDL_INFO_DIR/$TASKSET_CORE"
    echo $HDL_PG > "$HDL_INFO_DIR/$TASKSET_CORE/handler"
    echo $SCRATCH_PG > "$HDL_INFO_DIR/$TASKSET_CORE/scratchpad"
}

function install_hdl () {
    DO_INSTALL_HDL="$IDT_TOOL -a $NEW_IDT -m addr:$HDL_PG -v $VECTOR"
    # Stich interposing handler into IDT copy.
    if [ ! -z "$DEBUG" ]; then
        echo
        echo point new idt $NEW_IDT vector $VECTOR at $HDL_PG:
        print_color "$DO_INSTALL_HDL" $PRINT_RED
    fi
    eval $DO_INSTALL_HDL

    if [ ! -z "$DEBUG" ]; then
        echo hdl installed
    fi
}

function install_idt () {
    INSTALL_IDT="$TASKSET $IDT_TOOL -a $NEW_IDT -i"

    if [ ! -z "$DEBUG" ]; then
        echo
        echo make new idt live
        echo Load IDTR with $NEW_IDT:
        print_color "$INSTALL_IDT" $PRINT_RED
    fi
    OLD_IDT=$($INSTALL_IDT)
    if [ ! -z "$DEBUG" ]; then
        echo new idt should be live
        echo old is $OLD_IDT
    fi


}

function enable_ist () {
    ENABLE_IST="$TASKSET $IDT_TOOL -m ist_enable -v 0"

    if [ ! -z "$DEBUG" ]; then
        echo
	echo enable ist
    fi
    $($ENABLE_IST)
}

function disable_ist () {
    DISABLE_IST="$TASKSET $IDT_TOOL -m ist_disable -v 0"

    if [ ! -z "$DEBUG" ]; then
        echo
	echo disable ist
    fi
    $($DISABLE_IST)
}

# START HERE

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
IDT_TOOL=${SCRIPT_DIR}/../idt_tool

parse_args "$@"

TASKSET="taskset -c $TASKSET_CORE"

if [ ! -z "$MITIGATION" ]; then

    get_cur_idt

    cp_current_idt

    cp_hdl

    install_hdl

    install_idt

fi

exit 0

# User cmdline control
while true
do
    echo -n type any char from the table above:
    read input
    case $input in
        g)
            echo QUERY CURRENT IDT
            $TASKSET $IDT_TOOL -g
            ;;
        z)
            echo QUERY NEW IDT
            $TASKSET $IDT_TOOL -g
            ;;
        v)
            echo 
            $TASKSET $IDT_TOOL -a $NEW_IDT -i
            ;;
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
