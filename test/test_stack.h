
#ifndef MYSTL_TEST_STACK_H
#define MYSTL_TEST_STACK_H
#include "../stack.h"
#include "iostream"
#include "test_Macros.h"
#include "../list.h"
namespace MyStl{
    void test_stack(){
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Run container test : stack "
                     "-------------------]\n";
        std::cout << "[-------------------------- API test "
                     "---------------------------]\n";
        MyStl::stack<int> s1;
        MyStl::stack<int, list<int>> s2;

        std::cout << "[----------------- Underlying container : deque "
                     "-------------------]\n";
        FUN_VALUE(s1.empty());
        s1.push(1);
        std::cout << "After s1.push(1) -> ";
        FUN_VALUE(s1.size());
        s1.push(2);
        s1.push(3);
        std::cout << "After s1.push(2) , s1.push(3) -> ";
        FUN_VALUE(s1.top());
        s1.pop();
        std::cout << "After s1.pop() -> ";
        FUN_VALUE(s1.top());

        std::cout << std::endl;
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Underlying container : list "
                     "-------------------]\n";
        FUN_VALUE(s2.empty());
        s2.push(1);
        std::cout << "After s2.push(1) -> ";
        FUN_VALUE(s2.size());
        s2.push(2);
        s2.push(3);
        std::cout << "After s2.push(2) , s2.push(3) -> ";
        FUN_VALUE(s2.top());
        s2.pop();
        std::cout << "After s2.pop() -> ";
        FUN_VALUE(s2.top());

        std::cout << "[----------------------- end API test "
                     "---------------------------]\n";

    }
}
#endif //MYSTL_TEST_STACK_H
