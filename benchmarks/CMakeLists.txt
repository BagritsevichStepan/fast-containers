project(fast_containers_benchmarks)

set(BENCH_MAP_TARGET map_bench)

add_executable(BENCH_MAP_TARGET map_benchmark.cpp)

target_include_directories(BENCH_MAP_TARGET PRIVATE ${CONTAINERS_DIRECTORIES}})