
#ifndef MYSTL_PRIORITY_QUEUE_H
#define MYSTL_PRIORITY_QUEUE_H
#include "deque.h"
#include "heap.h"
#include "vector.h"
namespace MyStl{
    template <typename T, typename Container = MyStl::deque<T>>
    class priority_queue{
    public:
        using coner_type = Container;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        using reference = typename Container::reference;
        using const_reference = typename Container::const_reference;

    protected:
        Container con;
    public:
        //构造函数
        priority_queue() : con() {}
        template <typename InputIter>
        priority_queue(InputIter first, InputIter last) : con(first, last)
        { make_heap(con.begin(), con.end()); }
        
        
        const_reference top() const { return con.front(); }
        bool empty() const { return con.empty(); }
        size_type size() const { return con.size(); }
        void push( const value_type& value ) {
            con.push_back(value);
            push_heap(con.begin(), con.end());
        }
        void pop() {
            pop_heap(con.begin(), con.end());
            con.pop_back();
        }

    };


}

#endif //MYSTL_PRIORITY_QUEUE_H
