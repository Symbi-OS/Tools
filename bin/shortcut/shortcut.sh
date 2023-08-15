#!/bin/bash
# set -x
set -e


function print_help () {
    printf "###################################\n"

# square brackets [optional option]
# angle brackets <required argument>
# curly braces {default values}
# parenthesis (miscellaneous info)

    printf "\nshortcut.sh [args] --- <app> [app args]\n\n"
    printf "Script for launching apps with options for elevation and shortcutting.\n\n"

    printf "Flags:\n\n"

    # Passthrough


    # kElevate related options

    printf "\t-p: passthrough mode, run application without interposer library.\n"
    printf "\t\t Ignores all other args, \n"

    printf "The following options may interact with kElevate\n\n"

    printf "\t-be / -bl: begin with executable elevated / lowered\n\n"

    printf "These ones  can be repeated to specify multiple functions:\n"
    printf "\t-e <fns>: Elevate around these functions\n"
    printf "\t-l <fns>: Lower around these functions\n"
    printf "\t-s <fn1->fn2>: Shortcut between these functions: eg: 'write->ksys_write'\n"
    printf "\t\tDon't forget to use quotes around the function names!\n\n"

    printf "Debugging options\n\n"
    printf "\t-v: verbose mode for debugging\n"
    printf "\t-d: dry run, do everything EXCEPT running the application\n\n"

    printf "###################################\n"

}

function parse_args () {
    # Call getopt to validate the provided input.
    while getopts "b:e:dhl:pr:s:vw:" OPTION; do
        case "${OPTION}" in
            # Begin with executable elevated / lowered
            b)
                # echo OPTION: $OPTION
                # echo process b flag with "$OPTARG"
                BEGIN_ELE=$OPTARG
                case $BEGIN_ELE in
                    "e")
                        BEGIN_ELE=1
                        ;;
                    "l")
                        # Do nothing, this is the default
                        ;;
                    *)
                        echo "Incorrect -b option provided:" $BEGIN_ELE
                        echo bad begin: $BEGIN_ELE
                        ;;
                esac
                ;;
            d) # dry run
                DRY_RUN=1
                ;;
            # VERBOSE
            v)
                # echo process d flag
                VERBOSE=1
                ;;
            # elevate around these functions
            e)
                # echo process e flag with "$OPTARG"
                # Add to list of functions to elevate around
                ELEVATE_FN_LIST="$ELEVATE_FN_LIST $OPTARG"
                ;;
            # lower around these functions
            l)
                # echo process l flag with "$OPTARG"
                # Add to list of functions to lower around
                LOWER_FN_LIST="$LOWER_FN_LIST $OPTARG"
                ;;
            # passthrough mode, run without interposer library.
            p)
                # echo process p flag
                PASSTHROUGH=1
                ;;
            # shortcut between these functions
            s)
                # echo process s flag with "$OPTARG"
                # Add to list of functions to shortcut between
                SHORTCUT_FN_LIST="$SHORTCUT_FN_LIST $OPTARG"
                ;;
            h)
                print_help
                exit 0
                ;;
            *)
                echo "Incorrect option provided:" "$OPTION"
                echo bad option: "$OPTION"
                exit 1
                ;;
        esac
    done
}

# A function that checks the sanity of provided arguments.
function check_opts () {
    # This is currently mostly a stub.

    # Check that the user provided an executable to run

    if [ -z "$APP_AND_ARGS" ]; then
        echo "No executable provided"
        exit 1
    fi

    # If verbose, print out the arguments
    if [ -n "$VERBOSE" ]; then
        echo "Script arguments: $SCRIPT_ARGS"
        echo "APP_AND_ARGS: $APP_AND_ARGS"
        echo "ELEVATE_FN_LIST: $ELEVATE_FN_LIST"
        echo "LOWER_FN_LIST: $LOWER_FN_LIST"
        echo "SHORTCUT_FN_LIST: $SHORTCUT_FN_LIST"
        echo "BEGIN_ELE: $BEGIN_ELE"
    fi

    return
}

# Create environment variables that control how the interposer
# runs the executable.
create_envt_vars () {

    # If passthrough mode, don't bother, just return
    if [ -n "$PASSTHROUGH" ]; then
        # if VERBOSE
        if [ -n "$VERBOSE" ]; then
            echo "Passthrough mode, not creating envt vars"
            return
        fi
    fi

    ENVT_VARS=""

    # If we should begin elevated, add that to the environment
    if [ -n "$BEGIN_ELE" ]; then
        ENVT_VARS="$ENVT_VARS BEGIN_ELE=$BEGIN_ELE"
    fi

    # Create a string of functions to elevate around
    for fn in $ELEVATE_FN_LIST; do
        ENVT_VARS="$ENVT_VARS ELEVATE_$fn=1"
    done

    # Create a string of functions to lower around
    for fn in $LOWER_FN_LIST; do
        ENVT_VARS="$ENVT_VARS LOWER_$fn=1"
    done

    # Create a string of functions to shortcut between
    for fn in $SHORTCUT_FN_LIST; do
        # TODO: I couldn't figure out how to use a '->' in an envt var

        # Assert there is exactly one '-' on the line
        if [ $(echo $fn | grep -o '-' | wc -l) -ne 1 ]; then
            echo "Incorrect shortcut format: $fn"
            exit 1
        fi
        # Assert there is exactly one '>' on the line
        if [ $(echo $fn | grep -o '>' | wc -l) -ne 1 ]; then
            echo "Incorrect shortcut format: $fn"
            exit 1
        fi

        # Split the string on the ->, prefix and suffix
        prefix=$(echo $fn | cut -d'-' -f1)
        suffix=$(echo $fn | cut -d'>' -f2)

        ENVT_VARS="$ENVT_VARS SHORTCUT_${prefix}_TO_${suffix}=1"
    done

    if [ -n "$VERBOSE" ]; then
        echo "ENVT_VARS: $ENVT_VARS"
    fi
}

# Execute the application with the interposer library and all envt vars.
function run () {
    # We will run this program.
    RUN_CMD=$APP_AND_ARGS

    # If passthrough mode, skip

    # If not passthrough mode, add the interposer library and envt vars to the command
    if [ -z "$PASSTHROUGH" ]; then
        # Is this doing anything?
        # LD_LIBRARY_PATH="~/Symbi-OS/Symlib/dynam_build"

        # Gets overwritten here...
        # HACK: FIXME
        #export LD_LIBRARY_PATH="/home/$(whoami)/Symbi-OS/Symlib/dynam_build:/home/$(whoami)/Symbi-OS/Tools/bin/shortcut/deep_sc"
        #export LD_LIBRARY_PATH="/home/$(whoami)/Symbi-OS/Symlib/dynam_build:/home/$(whoami)/Symbi-OS/Tools/bin/shortcut/deep_sc"

        LD_LIBRARY_ENVT_VAR="LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

        RUN_CMD="$LD_LIBRARY_ENVT_VAR $ENVT_VARS $INTERPOSER_ENVT_VAR $RUN_CMD " 
    fi

    # If VERBOSEging, print out the command
    if [ -n "$VERBOSE" ]; then
        echo "RUN_CMD: $RUN_CMD"
    fi

    # Run the command
    # TODO: Someone tell me why this doesn't work
    # $RUN_CMD
    # But this does...
    # If dry run, don't actually run the command
    if [ -z "$DRY_RUN" ]; then
        eval "$RUN_CMD"
    else
        # if VERBOSE
        if [ -n "$VERBOSE" ]; then
            echo "Dry run, not running command"
        fi
    fi

}

# Program starts here
# Todo probably want to keep this as an array, but it's breaking somewhere...
INPUT_ARGS="$@"

# if [[ "$(declare -p "$INPUT_ARGS")" =~ "declare -a" ]]; then
#     echo array
# else
#     echo no array
# fi
# This is provided to separate the application arguments from the shortcut
SEARCH_STR="---"

# Return the prefix of the array up to the search string, these are for this script
SCRIPT_ARGS="${INPUT_ARGS%%"$SEARCH_STR"*}"

# Return the suffix of the array after the search string, these are for the application
APP_AND_ARGS="${INPUT_ARGS##*"$SEARCH_STR"}"

# echo INPUT_ARGS: $INPUT_ARGS
# echo SCRIPT_ARGS: $SCRIPT_ARGS
# echo APP_AND_ARGS: $APP_AND_ARGS

parse_args $SCRIPT_ARGS

check_opts

create_envt_vars

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
INTERPOSER_LIB="$SCRIPT_DIR/sc_lib.so"
INTERPOSER_ENVT_VAR="LD_PRELOAD=$INTERPOSER_LIB"

run
# todo, add error checking e.g. no element of ELEVATE_FN_LIST is in LOWER_FN_LIST
