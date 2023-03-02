
#ifndef MYSTL_TEST_DEQUE_H
#define MYSTL_TEST_DEQUE_H
#include <iostream>
#include "test_Macros.h"
#include "../deque.h"
namespace MyStl{
    void deque_test() {
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Run container test : deque "
                     "-------------------]\n";
        std::cout << "[-------------------------- API test "
                     "---------------------------]\n";
        int a[] = {1, 2, 3, 4, 5};
        MyStl::deque<int> d1;
        MyStl::deque<int> d2(10);
        MyStl::deque<int> d3(10, 1);
        MyStl::deque<int> d4(a, a + 5);
        MyStl::deque<int> d5(d4);
        MyStl::deque<int> d6 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        MyStl::deque<int> d7(d6.begin(), d6.end());
        const MyStl::deque<int>& rhs(d3);
        d1 = d3;
        PRINT(d1);
        PRINT(d2);
        PRINT(d3);
        PRINT(d4);
        PRINT(d5);
        PRINT(d6);
        PRINT(d7);
        FUN_AFTER(d1, d1.push_back(6));
        FUN_AFTER(d1, d1.push_front(8));
        FUN_AFTER(d1, d1.insert(d1.end(), 7));
        FUN_AFTER(d1, d1.insert(d1.begin(), 2, 3));
        FUN_AFTER(d1, d1.pop_back());
        FUN_AFTER(d1, d1.pop_front());
        FUN_AFTER(d1, d1.erase(d1.begin()));
        FUN_AFTER(d1, d1.erase(d1.begin(), d1.begin() + 3));
        FUN_AFTER(d1, d1.swap(d4));
        FUN_VALUE(*d1.begin());
        FUN_VALUE(*d1.cbegin());
        FUN_VALUE(*(d1.end() - 1));
        FUN_VALUE(*(d1.cend() - 1));
        FUN_VALUE(*d1.rbegin());
        FUN_VALUE(*(d1.rend() - 1));
        FUN_VALUE(d1.front());
        FUN_VALUE(d1.back());
        FUN_VALUE(d1[2]);
        FUN_VALUE(d1.size());
        FUN_VALUE(d1.max_size());
        FUN_VALUE(d1.empty());
        FUN_AFTER(d1, d1.resize(3));
        FUN_AFTER(d1, d1.resize(10, 5));
        FUN_AFTER(d1, d1.clear());
        FUN_VALUE(d1.size());
        FUN_VALUE(d1.empty());

        std::cout << "[----------------------- end API test "
                     "---------------------------]\n";
    }
}
#endif //MYSTL_TEST_DEQUE_H
