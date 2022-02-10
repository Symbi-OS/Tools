#Some dumb hack to get files into vm disk.

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
