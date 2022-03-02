#Some dumb hack to get files into vm disk.

# DEST=
# IMAGE=
# SOURCE=

# usage()
# {
#   echo "Usage: alphabet [ -d | --dest PUT HERE ON DISK ]
#                         [ -i | --image PATH_TO_DISK ]
#                         [ -s | --source FILES_TO_COPY ]
#                         [ -h | --help  ]"
#   exit 2
# }

# PARSED_ARGUMENTS=$(getopt -a -n alphabet -o hd:i:s: --long dest:,help,image:,source: -- "$@")
# VALID_ARGUMENTS=$?
# if [ "$VALID_ARGUMENTS" != "0" ]; then
#   usage
# fi

# echo "PARSED_ARGUMENTS is $PARSED_ARGUMENTS"
# eval set -- "$PARSED_ARGUMENTS"
# while :
# do
#   case "$1" in
#     # -a | --alpha)   ALPHA=1      ; shift   ;;
#       -d | --dest)    DEST="$2"   ; shift 2 ;;
#       -i | --image)   IMAGE="$2"  ; shift 2 ;;
#       -s | --source)  SOURCE="$2" ; shift 2 ;;
#       -h | --help)  HELP=1 ; shift ;;
#       # -- means the end of the arguments; drop this, and break out of the while loop
#       --) shift; break ;;
#     # If invalid options were passed, then getopt should have reported an error,
#     # which we checked as VALID_ARGUMENTS when getopt was called...
#       *) echo "Unexpected option: $1 - this should not happen."
#          usage ;;
#   esac
# done

# echo "DEST   : $DEST"
# echo "IMAGE  : $IMAGE "
# echo "SOURCE : $SOURCE"
# echo "Parameters remaining are: $@"

# exit

if (( $# < 2 )); then
    >&2 echo "expected $0 <path to disk> <any number of files to copy>"
fi

echo got $@

mkdir ./mt
sudo mount $1 ./mt
sleep 1

args=("$@")
for i in $( seq 1 $(($# - 1)) ); do
    echo cp ${args[$i]} $1
    sudo cp ${args[$i]} ./mt
done
sleep 1
sudo umount ./mt
rm -d ./mt
