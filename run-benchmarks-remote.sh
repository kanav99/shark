set -e

for benchmark in hinet deepsecure4 simc1 simc2 vgg16 bert alexnet; do
# for benchmark in hinet; do
  ./build/benchmark-$benchmark 2 &> /dev/null
  OMP_NUM_THREADS=4 ./build/benchmark-$benchmark $@ &> tmp.txt
  echo "Benchmarking $benchmark"
  echo "======================="
  cat tmp.txt
  rm tmp.txt
done
