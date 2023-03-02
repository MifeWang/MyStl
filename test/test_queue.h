
#ifndef MYSTL_TEST_QUEUE_H
#define MYSTL_TEST_QUEUE_H
#include "../queue.h"
#include "iostream"
#include "test_Macros.h"
#include "../list.h"
namespace MyStl{
    void test_queue(){
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Run container test : queue "
                     "-------------------]\n";
        std::cout << "[-------------------------- API test "
                     "---------------------------]\n";
        MyStl::queue<int> q1;
        MyStl::queue<int, list<int>> q2;

        std::cout << "[----------------- Underlying container : deque "
                     "-------------------]\n";
        FUN_VALUE(q1.empty());
        q1.push(1);
        std::cout << "After q1.push(1) -> ";
        FUN_VALUE(q1.size());
        q1.push(2);
        q1.push(3);
        std::cout << "After q1.push(2) , q1.push(3) -> ";
        FUN_VALUE(q1.front());
        FUN_VALUE(q1.back());
        q1.pop();
        std::cout << "After q1.pop() -> ";
        FUN_VALUE(q1.front());

        std::cout << std::endl;
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Underlying container : list "
                     "-------------------]\n";
        FUN_VALUE(q2.empty());
        q2.push(1);
        std::cout << "After q2.push(1) -> ";
        FUN_VALUE(q2.size());
        q2.push(2);
        q2.push(3);
        std::cout << "After q2.push(2) , q2.push(3) -> ";
        FUN_VALUE(q2.front());
        FUN_VALUE(q2.back());
        q2.pop();
        std::cout << "After q2.pop() -> ";
        FUN_VALUE(q2.front());


        std::cout << "[----------------------- end API test "
                     "---------------------------]\n";

    }
}
#endif //MYSTL_TEST_QUEUE_H
