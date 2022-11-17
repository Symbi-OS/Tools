#!/bin/bash


function print_help () {
    printf "###################################\n\n"
    printf "./signal_sender [args] <app to send sig to>\n"

    printf "Flags:\n\n"

    printf "The following send signals to the interposer:\n\n"

    printf "\t-e: elevate\n"
    printf "\t-l: lower\n"

    printf "\t-i: intermittant toggle\n"
    printf "\t-s: shortcut toggle \n\n"
    printf "###################################\n\n"

}

function parse_args () {
    # Call getopt to validate the provided input.
    while getopts "ehlis" OPTION; do
        case "${OPTION}" in
        e)
            ELEVATE=1
            ;;
        h)
            print_help
            exit 0
            ;;
        l)
            LOWER=1
            ;;
        i)
            INTERMITTANT=1
            ;;
        s)
            SHORTCUT=1
            ;;
        *)
            echo "Incorrect option provided:" "$OPTION"
            echo bad option: "$OPTION"
            ;;
        esac
    done
}
function send_sigs() {

    # if LOWER then send USR1
    if [[ $LOWER -eq 1 ]]; then
        printf "send usr1 to %s should do lower\n" "$1"
        killall -USR1 "$1"
        return
    fi

    if [[ $ELEVATE -eq 1 ]]; then
        printf "send usr2 to %s should do elevate\n" "$1"
        killall -USR2 "$1"
        return
    fi

    if [[ $INTERMITTANT -eq 1 ]]; then
        printf "send ? to %s should toggle intermittant normal syscall\n" "$1"
        # killall -USR3 "$1"
        printf "NYI\n"
        return
    fi 

    if [[ $SHORTCUT -eq 1 ]]; then
        printf "send sys to %s should toggle shortcutting\n" "$1"
        killall -SYS "$1"
        return
    fi
}

parse_args "$@"


# Assert only one flag is set
if [[ $ELEVATE && $LOWER ]]; then
    echo "Cannot elevate and lower at the same time"
    exit 1
fi

send_sigs "${@: -1}"