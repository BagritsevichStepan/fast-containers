project(fast_containers_example)

# Add targets
set(EXAMPLE_ANY_TARGET any_example)
set(EXAMPLE_D_HEAP_TARGET d_heap_example)
set(EXAMPLE_ID_CONTAINER_TARGET id_container_example)
set(EXAMPLE_ALLOCATORS_TARGET allocators_example)

# Add executables
add_executable(EXAMPLE_ANY_TARGET any_example.cpp)
add_executable(EXAMPLE_D_HEAP_TARGET d_heap_example.cpp)
add_executable(EXAMPLE_ID_CONTAINER_TARGET id_container_example.cpp)
add_executable(EXAMPLE_ALLOCATORS_TARGET allocators_example.cpp)

# Link libraries
target_link_libraries(EXAMPLE_ANY_TARGET LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(EXAMPLE_D_HEAP_TARGET LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(EXAMPLE_ID_CONTAINER_TARGET LINK_PUBLIC ${Boost_LIBRARIES})
target_link_libraries(EXAMPLE_ALLOCATORS_TARGET LINK_PUBLIC ${Boost_LIBRARIES})

# Include directories
target_include_directories(EXAMPLE_ANY_TARGET PRIVATE ${CONTAINERS_DIRECTORIES})
target_include_directories(EXAMPLE_D_HEAP_TARGET PRIVATE ${CONTAINERS_DIRECTORIES})
target_include_directories(EXAMPLE_ID_CONTAINER_TARGET PRIVATE ${CONTAINERS_DIRECTORIES})
target_include_directories(EXAMPLE_ALLOCATORS_TARGET PRIVATE ${CONTAINERS_DIRECTORIES})