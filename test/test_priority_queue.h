
#ifndef MYSTL_TEST_PRIORITY_QUEUE_H
#define MYSTL_TEST_PRIORITY_QUEUE_H
#include "priority_queue.h"
#include "test_Macros.h"
#include "deque.h"
#include "iostream"
namespace MyStl{
    void test_priority_queue() {
        std::cout << "[============================================================"
                     "===]\n";
        std::cout << "[----------------- Run container test : priority_queue "
                     "-------------------]\n";
        std::cout << "[-------------------------- API test "
                     "---------------------------]\n";


        int a[] = {1, 2, 3, 4, 5};
        MyStl::priority_queue<int> q1(a, a + 5);
        FUN_VALUE(q1.empty());
        FUN_VALUE(q1.top());
        q1.push(1);
        std::cout << "After q1.push(1) -> ";
        FUN_VALUE(q1.size());
        q1.push(7);
        q1.push(6);
        std::cout << "After q1.push(6) , q1.push(7) -> ";
        FUN_VALUE(q1.top());
        q1.pop();
        std::cout << "After q1.pop() -> ";
        FUN_VALUE(q1.top());
        std::cout << std::endl;

        std::cout << "[----------------------- end API test "
                     "---------------------------]\n";
    }
}
#endif //MYSTL_TEST_PRIORITY_QUEUE_H
