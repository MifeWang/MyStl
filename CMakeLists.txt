cmake_minimum_required(VERSION 3.23)
project(MySTL)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)
include_directories(test)

add_executable(MySTL main.cpp type_traits.h new_allocator.h move.h pool_allocator.h test/test_allocator.h iterator.h uninitialized.h construct.h vector.h test/test_Macros.h test/test_vector.h list.h test/test_list.h deque.h)
