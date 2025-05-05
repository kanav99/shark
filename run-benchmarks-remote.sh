set -e

for benchmark in deepsecure4 simc1 simc2 hinet alexnet vgg16 bert; do
# for benchmark in hinet; do
  ./build/benchmark-$benchmark 2 &> /dev/null
  OMP_NUM_THREADS=4 ./build/benchmark-$benchmark $@ &> tmp.txt
  echo "Benchmarking $benchmark"
  echo "======================="
  cat tmp.txt
  rm tmp.txt
done
