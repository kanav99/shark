cmake_minimum_required(VERSION 3.16)
project(shark)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-write-strings -Wno-unused-result -maes -Wno-ignored-attributes -march=native -Wno-deprecated-declarations -fopenmp")

find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Threads REQUIRED)

add_subdirectory(ext/cryptoTools)

add_library(${PROJECT_NAME}
    src/utils/timer.cpp
    src/utils/comm.cpp
    src/protocols/common.cpp
    src/protocols/init.cpp
    src/protocols/finalize.cpp
    src/protocols/input.cpp
    src/protocols/output.cpp
    src/protocols/matmul.cpp
    src/protocols/relutruncate.cpp
    src/protocols/drelu.cpp
    src/protocols/select.cpp
    src/protocols/relu.cpp
    src/protocols/conv.cpp
    src/protocols/maxpool.cpp
    src/protocols/spline.cpp
    src/protocols/lrs.cpp
    src/protocols/sumpool.cpp
    src/protocols/mul.cpp
    src/protocols/lut.cpp
    src/protocols/reciprocal.cpp
    src/crypto/dcfbit.cpp
    src/crypto/dcfring.cpp
    src/crypto/dpfring.cpp
)

target_link_libraries (${PROJECT_NAME} Eigen3::Eigen Threads::Threads cryptoTools)
target_include_directories(${PROJECT_NAME}
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_executable(test-matmul-time tests/single/matmul-time.cpp)
target_link_libraries(test-matmul-time ${PROJECT_NAME})

add_executable(test-init tests/multi/init.cpp)
target_link_libraries(test-init ${PROJECT_NAME})


add_executable(test-matmul tests/multi/matmul.cpp)
target_link_libraries(test-matmul ${PROJECT_NAME})

add_executable(test-dcfbit tests/single/dcfbit.cpp)
target_link_libraries(test-dcfbit ${PROJECT_NAME})

add_executable(test-dcfring tests/single/dcfring.cpp)
target_link_libraries(test-dcfring ${PROJECT_NAME})

add_executable(test-dpfring tests/single/dpfring.cpp)
target_link_libraries(test-dpfring ${PROJECT_NAME})

add_executable(test-relutruncate tests/multi/relutruncate.cpp)
target_link_libraries(test-relutruncate ${PROJECT_NAME})

add_executable(test-select tests/multi/select.cpp)
target_link_libraries(test-select ${PROJECT_NAME})

add_executable(test-span tests/single/span.cpp)
target_link_libraries(test-span ${PROJECT_NAME})

add_executable(test-matmul-big tests/multi/matmul-big.cpp)
target_link_libraries(test-matmul-big ${PROJECT_NAME})

add_executable(test-drelu tests/multi/drelu.cpp)
target_link_libraries(test-drelu ${PROJECT_NAME})

add_executable(test-relu tests/multi/relu.cpp)
target_link_libraries(test-relu ${PROJECT_NAME})

add_executable(test-conv tests/failing/conv.cpp)
target_link_libraries(test-conv ${PROJECT_NAME})

add_executable(test-maxpool tests/multi/maxpool.cpp)
target_link_libraries(test-maxpool ${PROJECT_NAME})

add_executable(benchmark-hinet benchmarks/hinet.cpp)
target_link_libraries(benchmark-hinet ${PROJECT_NAME})

add_executable(benchmark-vgg16 benchmarks/vgg16.cpp)
target_link_libraries(benchmark-vgg16 ${PROJECT_NAME})

add_executable(test-spline tests/multi/spline.cpp)
target_link_libraries(test-spline ${PROJECT_NAME})

add_executable(test-sigmoid tests/multi/sigmoid.cpp)
target_link_libraries(test-sigmoid ${PROJECT_NAME})

add_executable(test-tanh tests/multi/tanh.cpp)
target_link_libraries(test-tanh ${PROJECT_NAME})

add_executable(test-reciprocal tests/failing/reciprocal.cpp)
target_link_libraries(test-reciprocal ${PROJECT_NAME})

add_executable(test-lut tests/multi/lut.cpp)
target_link_libraries(test-lut ${PROJECT_NAME})

add_executable(benchmark-deepsecure1 benchmarks/deepsecure1.cpp)
target_link_libraries(benchmark-deepsecure1 ${PROJECT_NAME})

add_executable(benchmark-deepsecure2 benchmarks/deepsecure2.cpp)
target_link_libraries(benchmark-deepsecure2 ${PROJECT_NAME})

add_executable(benchmark-deepsecure3 benchmarks/deepsecure3.cpp)
target_link_libraries(benchmark-deepsecure3 ${PROJECT_NAME})

add_executable(benchmark-deepsecure4 benchmarks/deepsecure4.cpp)
target_link_libraries(benchmark-deepsecure4 ${PROJECT_NAME})

add_executable(benchmark-simc1 benchmarks/simc1.cpp)
target_link_libraries(benchmark-simc1 ${PROJECT_NAME})

add_executable(benchmark-simc2 benchmarks/simc2.cpp)
target_link_libraries(benchmark-simc2 ${PROJECT_NAME})

add_executable(benchmark-bert benchmarks/bert.cpp)
target_link_libraries(benchmark-bert ${PROJECT_NAME})

add_executable(benchmark-alexnet benchmarks/alexnet.cpp)
target_link_libraries(benchmark-alexnet ${PROJECT_NAME})

add_executable(benchmark-alexnet-localtrunc benchmarks/alexnet-localtrunc.cpp)
target_link_libraries(benchmark-alexnet-localtrunc ${PROJECT_NAME})

add_executable(benchmark-mnist-A benchmarks/mnist-A.cpp)
target_link_libraries(benchmark-mnist-A ${PROJECT_NAME})

add_executable(micro-relu microbenchmarks/relu.cpp)
target_link_libraries(micro-relu ${PROJECT_NAME})

add_executable(micro-spline microbenchmarks/spline.cpp)
target_link_libraries(micro-spline ${PROJECT_NAME})

add_executable(micro-relutruncate microbenchmarks/relutruncate.cpp)
target_link_libraries(micro-relutruncate ${PROJECT_NAME})

add_executable(micro-maxpool microbenchmarks/maxpool.cpp)
target_link_libraries(micro-maxpool ${PROJECT_NAME})

add_executable(micro-reciprocal microbenchmarks/reciprocal.cpp)
target_link_libraries(micro-reciprocal ${PROJECT_NAME})

add_executable(micro-matmul microbenchmarks/matmul.cpp)
target_link_libraries(micro-matmul ${PROJECT_NAME})

add_executable(micro-linear microbenchmarks/linear.cpp)
target_link_libraries(micro-linear ${PROJECT_NAME})
