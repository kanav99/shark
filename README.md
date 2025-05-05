# Shark

Shark is a framework for actively secure inference of machine learning models. This repository contains implementation of protocols from our [IEEE S&P 2025 paper](https://www.computer.org/csdl/proceedings-article/sp/2025/223600c268/26hiUPCGeti).

## Building

Shark requires CMake (>= 3.16) and a C++ compiler (with support for C++20 standard). Shark also requires Eigen3 which can be installed using `sudo apt install libeigen3-dev` (on Ubuntu/Debian) or `brew install eigen` (on macOS/linuxbrew). Shark optionally requires OpenMP for parallelization.

To compile Shark, run the following command from the project root:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/
cmake --build build/ --config Release --target all -j
```

## Benchmarking

We provide four benchmarking scripts in the root of project that can reproduce the benchmarking results from the paper (specifically, Table 2 and 3).

#### Local Setup

In case you want to run both parties of Shark on a single machine, with each party using 4 threads each, the following commands can be used to run benchmarks and microbenchmarks, respectively.

```bash
bash run-benchmarks-local.sh
bash run-microbenchmarks-local.sh
```

#### Two Machine Setup

If you have two machines connected through IP, lets say at `10.0.0.1` and `10.0.0.2`, run the following commands on each machine to run benchmarks:

```bash
bash run-benchmarks-remote.sh 0 10.0.0.2 # on 10.0.0.1
bash run-benchmarks-remote.sh 1 10.0.0.1 # on 10.0.0.2
```

Microbenchmarks can be run by replacing script with `run-microbenchmarks-remote.sh`. By default, Shark assumes the port range 42000-42100 to be open. In case you would like to use a different range, provide the starting port as the third argument to the scripts.

#### Custom Model Benchmarking

To write a custom model in our framework, please refer to `benchmarks/mnist-A.cpp` for reference. To build your custom model `model.cpp` in Shark, place it in `benchmarks/` directory, add the following to the end of CMakeLists.txt:

```
add_executable(benchmark-model benchmarks/model.cpp)
target_link_libraries(benchmark-model ${PROJECT_NAME})
```

Rebuild Shark and your binary would be available as `build/benchmark-model`. To run it, use the following commands:

```bash
./build/benchmark-model 2 # on both machines, to generate keys
OMP_NUM_THREADS=4 ./build/benchmark-model 0 10.0.0.2 # on 10.0.0.1
OMP_NUM_THREADS=4 ./build/benchmark-model 1 10.0.0.1 # on 10.0.0.2
```

## Bugs

Please feel free to open an issue in case you encounter a bug.

## Citation

If you make use of our work, please consider citing us:

```bibtex
@INPROCEEDINGS {
    author = { Gupta, Kanav and Chandran, Nishanth and Gupta, Divya and Katz, Jonathan and Sharma, Rahul },
    booktitle = { 2025 IEEE Symposium on Security and Privacy (SP) },
    title = {{ Shark: Actively Secure Inference using Function Secret Sharing }},
    year = {2025},
    doi = {10.1109/SP61157.2025.00175},
}
```