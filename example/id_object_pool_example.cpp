#include <iostream>
#include <vector>
#include "id_object_pool.h"

namespace {

    using Base = fast_containers::IdObjectPoolElementBase;

    class Order : public Base {
    public:
        using Base::generation_;

        Order() = default;

        Order(uint64_t price, uint64_t client_id) : price_(price), client_id_(client_id) {}

        Order(const Order&) = delete;
        Order(Order&&) = delete;
        Order& operator=(const Order&) = delete;
        Order& operator=(Order&&) = delete;

        ~Order() = default;

        uint64_t price_{0};
        uint64_t client_id_{0};
    };


    inline constexpr std::size_t kCapacity = 10;

    using Id = fast_containers::ContainerElementId;
    using ObjectPool = fast_containers::IdObjectPool<Order, kCapacity>;

    void Construct(int n, ObjectPool& object_pool, std::vector<Id>& ids) {
        std::cout << "Constructed:" << std::endl;
        for (int i = 0; i < n; i++) {
            uint64_t price = rand();
            uint64_t client_id = rand();

            Id id = object_pool.Construct(price, client_id);
            ids.push_back(id);

            std::cout << "Order: [id=" << id << ", price=" << price
                      << ", client_id=" << client_id << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    void Read(ObjectPool& object_pool, std::vector<Id>& ids) {
        std::cout << "Read:" << std::endl;
        for (int i = 0; i < ids.size(); i++) {
            Id id = ids[i];
            Order* order = object_pool.Get(id);
            std::cout << "Order: [id=" << id << ", price=" << order->price_
                      << ", client_id=" << order->client_id_ << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    void Destroy(int from, int to, ObjectPool& object_pool, std::vector<Id>& ids) {
        for (int i = from; i < to; i++) {
            Id id = ids[i];
            object_pool.Destroy(id);
            std::cout << "Order with id=" << id << " is destroyed:" << !object_pool.Contains(id) << std::endl;
        }
        std::cout << std::endl;
        ids.erase(ids.begin() + from, ids.begin() + to);
    }
}

int main() {
    ObjectPool object_pool{};
    std::vector<Id> ids;

    Construct(kCapacity, object_pool, ids);
    Read(object_pool, ids);
    Destroy(0, kCapacity / 2, object_pool, ids);
    Read(object_pool, ids);

    Construct(kCapacity / 2, object_pool, ids);
    Read(object_pool, ids);
    Destroy(kCapacity / 2, kCapacity, object_pool, ids);
    Construct(kCapacity / 2, object_pool, ids);
    Read(object_pool, ids);
    return 0;
}
