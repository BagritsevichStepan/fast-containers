#include <iostream>
#include <vector>
#include "id_container.h"

namespace {

    using Base = fast_containers::IdContainerElementBase;

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
    using Container = fast_containers::IdContainer<Order, kCapacity>;

    void Construct(int n, Container& id_container, std::vector<Id>& ids) {
        std::cout << "Constructed:" << std::endl;
        for (int i = 0; i < n; i++) {
            uint64_t price = rand();
            uint64_t client_id = rand();

            Id id = id_container.Construct(price, client_id);
            ids.push_back(id);

            std::cout << "Order: [id=" << id << ", price=" << price
                      << ", client_id=" << client_id << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    void Read(Container& id_container, std::vector<Id>& ids) {
        std::cout << "Read:" << std::endl;
        for (int i = 0; i < ids.size(); i++) {
            Id id = ids[i];
            Order* order = id_container.Get(id);
            std::cout << "Order: [id=" << id << ", price=" << order->price_
                      << ", client_id=" << order->client_id_ << "]" << std::endl;
        }
        std::cout << std::endl;
    }

    void Destroy(int from, int to, Container& id_container, std::vector<Id>& ids) {
        for (int i = from; i < to; i++) {
            Id id = ids[i];
            id_container.Destroy(id);
            std::cout << "Order with id=" << id << " is destroyed:" << !id_container.Contains(id) << std::endl;
        }
        std::cout << std::endl;
        ids.erase(ids.begin() + from, ids.begin() + to);
    }
}

int main() {
    Container id_container{};
    std::vector<Id> ids;

    Construct(kCapacity, id_container, ids);
    Read(id_container, ids);
    Destroy(0, kCapacity / 2, id_container, ids);
    Read(id_container, ids);

    Construct(kCapacity / 2, id_container, ids);
    /*Read(id_container, ids);
    Destroy(kCapacity / 2, kCapacity, id_container, ids);
    Construct(kCapacity / 2, id_container, ids);
    Read(id_container, ids);
    */
    return 0;
}

/*
Order: [id=4294967416, price=1025202362, client_id=1350490027]
Order: [id=4294967440, price=783368690, client_id=1102520059]
Order: [id=4294967464, price=2044897763, client_id=1967513926]
Order: [id=4294967488, price=1365180540, client_id=1540383426]
Order: [id=4294967512, price=304089172, client_id=1303455736]

Order: [id=4294967416, price=1025202362, client_id=1350490027]
Order: [id=4294967440, price=783368690, client_id=1102520059]
Order: [id=4294967464, price=2044897763, client_id=1967513926]
Order: [id=4294967488, price=1365180540, client_id=1540383426]
Order: [id=4294967512, price=140721769610928, client_id=1303455736]
 */