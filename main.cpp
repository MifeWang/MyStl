#pragma once
#include <iostream>
#include "vector"
#include "typeinfo"

using namespace std;


#include "test/test_allocator.h"
#include "test_vector.h"
#include "uninitialized.h"
#include "list"
#include "test_list.h"
#include "test_deque.h"
using namespace std;
int main(){
    MyStl::deque_test();

}