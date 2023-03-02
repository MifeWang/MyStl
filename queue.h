
#ifndef MYSTL_QUEUE_H
#define MYSTL_QUEUE_H

#include "deque.h"
namespace MyStl{
    template<typename T, typename Container = MyStl::deque<T>>
    class queue{
        //非成员函数
    public:
        friend bool operator==(const queue &lhs, const queue &rhs);
        friend bool operator!=(const queue &lhs, const queue &rhs);
        friend bool operator<(const queue &lhs, const queue &rhs);
        friend bool operator>(const queue &lhs, const queue &rhs);
        friend bool operator<=(const queue &lhs, const queue &rhs);
        friend bool operator>=(const queue &lhs, const queue &rhs);

    public:
        using container_type = Container;
        using value_type = typename Container::value_type;
        using size_type = typename Container::size_type;
        using reference = typename Container::reference;
        using const_reference = typename Container::const_reference;
    protected:
        //底层容器
        Container con;
    public:
        //只需要使用底层容器的操作，即可完成所有的queue成员函数
        bool empty() const { return con.empty(); }
        size_type size() const { return con.size(); }
        reference front() { return con.front(); }
        const_reference front() const { return con.front(); }
        reference back() { return con.back(); }
        const_reference back() const { return con.back(); }
        void push(const value_type& value) { con.push_back(value); }
        void pop() { con.pop_front(); }
    };

    template <typename T, typename Container>
    bool operator==(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return lhs.con == rhs.con;
    }

    template <typename T, typename Container>
    bool operator!=(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return !(rhs == lhs);
    }

    template <typename T, typename Container>
    bool operator<(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return lhs.con < rhs.con;
    }

    template <typename T, typename Container>
    bool operator>(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return rhs < lhs;
    }

    template <typename T, typename Container>
    bool operator<=(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return !(rhs < lhs);
    }

    template <typename T, typename Container>
    bool operator>=(const queue<T, Container> &lhs, const queue<T, Container> &rhs) {
        return !(lhs < rhs);
    }


}
#endif //MYSTL_QUEUE_H
