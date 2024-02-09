# Fast Containers
Implementation of low latency stack based containers: IdObjectPool, DHeap, Map. Fast allocators implementations.

Most of these containers are used as components in HFT.

# Links

+ [IdObjectPool](#id_object_pool)
+ [D-ary Heap](#d_heap)
    * [SIMD](#d_heap_simd)
+ [InplaceAny](#inplace_any)
+ [InplaceString](#inplace_string)
+ [Fast unordered map](#map)
    * [Open Addressing](#map_addressing)
    * [Robin Hood Hashing](#map_hashing)
+ [Allocators](#allocators)
    * [StackBasedAllocator](#stack_allocator)
    * [HugePageAllocator](#huge_page_allocator)

# <a name="id_object_pool"></a>IdObjectPool
```cpp
class Order : public fast_containers::IdObjectPoolElementBase {
public:
   using Base::generation_;

   Order(uint64_t price, uint64_t client_id) : price_(price), client_id_(client_id) {}

   uint64_t price_{0};
   uint64_t client_id_{0};
};

...

fast_containers::IdObjectPool<Order, Capacity> orders_pool{};
auto id = orders_pool.Construct(5, 7);
assert(orders_pool.Contains(id));
assert(orders_pool.Get(id)->price_ == 5 && orders_pool.Get(id)->client_id_ == 7);
```
`IdObjectPool` is fast [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and [`std::allocator`](https://en.cppreference.com/w/cpp/memory/allocator) together. It allocates memory for objects and generates a key that can be used to find an object in `O(1)`.

To use the `IdObjectPool` for your class, it must inherit from the `fast_containers::IdObjectPoolElementBase` class.

The key is generated based on the address and generation. So the user always gets a unique id.

Unlike in the associative `std::unordered_map`, in `IdObjectPool` the data is located close to each other, which reduces the number of cache misses.

In addition, you can reduce the number of cache misses more by "packaging" data after several deletions. But for this you will need to store another array to support the old IDs.

# <a name="d_heap"></a>D-ary Heap
```cpp
void HeapSort(std::vector<std::int32_t> v) {
   fast_containers::MinDHeap<std::int32_t, Capacity> min_heap{};
   for (auto x : v) {
      min_heap.Insert(x);
   }
   std::int32_t min_element;
   for (int i = 0; i < v.size(); i++) {
      min_heap.Pop(min_element);
      OutputNextElement(min_element);
   }
} 
```

Fast implementation of [`std::priority_queue`](https://en.cppreference.com/w/cpp/container/priority_queue).

To get the minimum element from the heap `Top()` method, use `fast_containers::MinDHeap`. And conversely, for the maximum element, use `fast_containers::MaxDHeap`.

In order to use your custom comparator, please use the base class `fast_containers::DHeap<typename ValueType, ValueType DefaultValue, std::size_t Capacity, std::size_t D, auto Comparator>`, where `DefaultValue` is the initial value for which `Comparator` returns false.

[D-ary Heap](https://en.wikipedia.org/wiki/D-ary_heap) is faster than a binary heap because it is better located in the cache.

## <a name="d_heap_simd"></a>Simd
For integer keys, [SIMD](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data) is used to speed up operations.

# <a name="inplace_any"></a>InplaceAny
```cpp
fast_containers::InplaceTrivialAny<32, alignof(int)> a = 5;
assert(a.Get<int>() == 5);
a = 543;
assert(a.Get<int>() != 5);
assert(a.Get<int>() == 543);
```

`InplaceAny` is an analog of [`boost::Any`](https://www.boost.org/doc/libs/1_74_0/boost/any.hpp), but unlike it, it allocates memory on the stack.

Use `InplaceTrivialAny` for trivially copyable types. It uses [`std::memcpy`](https://en.cppreference.com/w/cpp/string/byte/memcpy) and thus works faster than basic `InplaceAny`.

Therefore, it works much faster than its analog from `boost`.

# <a name="inplace_string"></a>InplaceString
Implementation of [`std::string`](https://en.cppreference.com/w/cpp/string/basic_string) that allocates memory on the stack.

# <a name="map"></a>Fast unordered map
Implementation of [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map).

## <a name="map_addressing"></a>Open Addressing
This hash table uses [open addressing](https://en.wikipedia.org/wiki/Open_addressing) with linear probing.

## <a name="map_hashing"></a>Robin Hood Hashing
In addition, to speed up [open addressing](#map_addressing) is uses [Robin Hood Hashing](https://programming.guide/robin-hood-hashing.html) method.

# Allocators
Several useful allocators implementations.

## <a name="stack_allocator"></a>StackBasedAllocator
This allocator that uses stack memory.

The `StackBasedAllocator` uses chunks of different sizes (each of them is a power of two) to perform allocations.

## <a name="huge_page_allocator"></a>HugePageAllocator
`HugePageAllocator` uses [huge pages](https://wiki.debian.org/Hugepages) when allocating.

The allocator is well suited for collections with a large data set.

It reduses cache misses in [TLB](https://en.wikipedia.org/wiki/Translation_lookaside_buffer). So in some cases it can significantly speed up your program.


