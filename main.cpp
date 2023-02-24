//#pragma once
#include <iostream>
#include "vector"


//using namespace std;

#include "test/test_allocator.h"
#include "iterator.h"
int main(){
//    MyStl::test_allocator();
    std::vector<int, MyStl::pool_alloc<int>> v1;
    v1.push_back(1);
    v1.push_back(2);
    v1.push_back(3);
    std::cout << v1[1] << std::endl;

}