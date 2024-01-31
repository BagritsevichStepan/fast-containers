# Fast Containers
Implementation of low latency stack based containers: IdContainer, DHeap, Map. Fast allocators implementations.

Most of these containers are used as components in HFT.

# Links

+ [IdContainer](#id_container)
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

# <a name="id_container"></a>IdContainer
`IdContainer` is fast [`std::unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) and [`std::allocator`](https://en.cppreference.com/w/cpp/memory/allocator) together. It allocates memory for objects and generates a key that can be used to find an object in `O(1)`.

The key is generated based on the address and generation. So the user always gets a unique id.

Unlike in the associative `std::unordered_map`, in `IdContainer` the data is located close to each other, which reduces the number of cache misses.

In addition, you can reduce the number of cache misses more by "packaging" data after several deletions. But for this you will need to store another array to support the old IDs.

# <a name="d_heap"></a>D-ary Heap
Fast implementation of [`std::pripority_queue`](https://en.cppreference.com/w/cpp/container/priority_queue).

[D-ary Heap](https://en.wikipedia.org/wiki/D-ary_heap) is faster than a binary heap because it is better located in the cache.

## <a name="d_heap_simd"></a>Simd
For integer keys, [SIMD](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data) is used to speed up operations.

# <a name="inplace_any"></a>InplaceAny
`InplaceAny` is an analog of [`boost::Any`](https://www.boost.org/doc/libs/1_74_0/boost/any.hpp), but unlike it, it allocates memory on the stack.

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


