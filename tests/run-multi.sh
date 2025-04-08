set -e
# get tname from argv
tname=$1
./build/test-$tname 2 &> /dev/null
./build/test-$tname 1 &> /dev/null &
./build/test-$tname 0 &> /dev/null
echo "passed"
