set -e

for benchmark in relu relutruncate spline reciprocal; do
# for benchmark in relutruncate; do
  ./build/micro-$benchmark 2 &> /dev/null
  OMP_NUM_THREADS=4 ./build/micro-$benchmark $@ &> tmp.txt
  echo "Benchmarking $benchmark"
  echo "======================="
  cat tmp.txt
  rm tmp.txt
done
