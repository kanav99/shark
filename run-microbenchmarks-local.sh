set -e

for benchmark in relu relutruncate spline reciprocal; do
# for benchmark in relutruncate; do
  ./build/micro-$benchmark 2 &> /dev/null
  OMP_NUM_THREADS=4 ./build/micro-$benchmark 0 &> tmp0.txt &
  OMP_NUM_THREADS=4 ./build/micro-$benchmark 1 &> tmp1.txt
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
