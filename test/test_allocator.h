
#pragma once
#ifndef MYSTL_TEST_ALLOCATOR_H
#define MYSTL_TEST_ALLOCATOR_H


#include <list>
#include <iostream>
#include "../new_allocator.h"
#include "../pool_allocator.h"
#include "ext/pool_allocator.h"

namespace MyStl
{
    enum { NUMBERS = 10000000 };

    void test_allocator() {
        // test of list
        // test of push_back
        std::list<int> list1;
        std::list<int, MyStl::new_allocator<int>> list2;
        std::list<int, MyStl::pool_alloc<int>> list3;
        std::list<int, __gnu_cxx::__pool_alloc<int>> list4;
        clock_t start = clock();
        for(size_t i = 0; i < NUMBERS; ++i)
            list1.push_back(1);
        clock_t end = clock();
        std::cout << "Time to insert " << NUMBERS
                  << " numbers in list with STL alloctor: "
                  << end - start << std::endl;
        start = clock();
        for(size_t i = 0; i < NUMBERS; ++i)
            list2.push_back(1);
        end = clock();
        std::cout << "Time to insert " << NUMBERS
                  << " numbers in list with MyStl new_allocator: "
                  << end - start << std::endl;
        start = clock();
        for(size_t i = 0; i < NUMBERS; ++i)
            list3.push_back(1);
        end = clock();
        std::cout << "Time to insert " << NUMBERS
                  << " numbers in list with MyStl pool_alloc: "
                  << end - start << std::endl;

        start = clock();
        for(size_t i = 0; i < NUMBERS; ++i)
            list4.push_back(1);
        end = clock();
        std::cout << "Time to insert " << NUMBERS
                  << " numbers in list with MyStl pool_alloc: "
                  << end - start << std::endl;
    }
}

#endif //MYSTL_TEST_ALLOCATOR_H
