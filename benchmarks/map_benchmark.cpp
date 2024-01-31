#include <iostream>
#include <vector>
#include <cstdint>

int main() {
    const int64_t iterations = 10000;
    //const int64_t times = 10;

    {
        rigtorp::HashMap<int, int> map{iterations, 0};

        auto start = std::chrono::steady_clock::now(); // Start measure the time
        //for (int t = 0; t < times; t++) {
            for (int i = 1; i < iterations; i++) {
                map[i] = i;
                //std::cout << i << std::endl;
            }

            /*for (int i = 1; i < iterations; i++) {
                map.erase(i);
                //std::cout << i << std::endl;
            }*/
       // }

        auto stop = std::chrono::steady_clock::now(); // Stop measure the time

        std::cout << "Throughput of the rigtorp::HashMap :" << std::endl;
        std::cout << iterations * int64_t(1000000) / std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() << " ops/ms" << std::endl;
    }

    {
        dod::slot_map<int> map;

        std::vector<decltype(map)::key> keys{iterations};
        auto start = std::chrono::steady_clock::now(); // Start measure the time
        //for (int t = 0; t < times; t++) {
            for (int i = 0; i < iterations; i++) {
                keys[i] = map.emplace(i);
                //std::cout << i << std::endl;
            }

            /*for (int i = 0; i < iterations; i++) {
                map.erase(keys[i]);
                //std::cout << i << std::endl;
            }*/
        //}

        auto stop = std::chrono::steady_clock::now(); // Stop measure the time

        std::cout << "Throughput of the dod::slot_map :" << std::endl;
        std::cout << iterations * int64_t(1000000) / std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() << " ops/ms" << std::endl;
    }

    {
        container::map::ChunkMap<int, container::map::detail::kDefaultChunkCapacity, iterations + 4> map{};

        auto start = std::chrono::steady_clock::now(); // Start measure the time

        //for (int t = 0; t < times; t++) {
            for (int i = 1; i < iterations; i++) {
                map[i] = i;
                //std::cout << i << std::endl;
            }

            /*for (int i = 1; i < iterations; i++) {
                //map.Erase(i);
                //std::cout << i << std::endl;
            }*/
        //}

        auto stop = std::chrono::steady_clock::now(); // Stop measure the time

        std::cout << "Throughput of the container::map::ChunkMap :" << std::endl;
        std::cout << iterations * int64_t(1000000) / std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() << " ops/ms" << std::endl;
    }
    return 0;
}
