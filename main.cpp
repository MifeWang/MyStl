//#pragma once
#include <iostream>
#include "vector"
#include "typeinfo"

//using namespace std;
#include "list.h"

#include "test/test_allocator.h"
#include "test_vector.h"
#include "uninitialized.h"
#include "list"
using namespace std;
int main(){
//    MyStl::vector_test();
//    MyStl::link_node_traits<int> test1;
//    std::cout << test1.test << std::endl;
//    MyStl::link_node_traits<const int> test2;
//    std::cout << test2.test << std::endl;

    list<int> lt;
    lt.push_back(1);
    lt.push_back(2);
    lt.push_back(3);
    lt.push_back(4);
    lt.push_back(5);

    auto it = lt.begin();
    while (1)
    {
        cout << *it << " ";
        ++it;
    }
    cout << endl;

}