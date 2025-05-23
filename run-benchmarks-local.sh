set -e

for benchmark in deepsecure4 simc1 simc2 hinet alexnet vgg16 bert; do
# for benchmark in hinet; do
  ./build/benchmark-$benchmark 2 &> /dev/null
  OMP_NUM_THREADS=4 ./build/benchmark-$benchmark 0 &> tmp0.txt &
  OMP_NUM_THREADS=4 ./build/benchmark-$benchmark 1 &> tmp1.txt
  echo "Benchmarking $benchmark"
  echo "======================="
  echo "Party 0"
  echo "======================="
  cat tmp0.txt
  echo "Party 1"
  echo "======================="
  cat tmp1.txt
  rm tmp0.txt tmp1.txt
done
