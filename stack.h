
#ifndef MYSTL_STACK_H
#define MYSTL_STACK_H
#include "deque.h"

namespace MyStl{
    template<typename T, typename Container = MyStl::deque<T>>
    class stack{
        //非成员函数
    public:
        friend bool operator==(const stack &lhs, const stack &rhs);
        friend bool operator!=(const stack &lhs, const stack &rhs);
        friend bool operator<(const stack &lhs, const stack &rhs);
        friend bool operator>(const stack &lhs, const stack &rhs);
        friend bool operator<=(const stack &lhs, const stack &rhs);
        friend bool operator>=(const stack &lhs, const stack &rhs);

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
        //只需要使用底层容器的操作，即可完成所有的stack成员函数
        reference top() { return con.back(); }
        const_reference top() const { return con.back(); }
        bool empty() const { return con.empty(); }
        size_type size() const { return con.size(); }
        void push(const value_type& value) { con.push_back(value); }
        void pop() { con.pop_back(); }
    };

    template <typename T, typename Container>
    bool operator==(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return lhs.con == rhs.con;
    }

    template <typename T, typename Container>
    bool operator!=(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return !(rhs == lhs);
    }

    template <typename T, typename Container>
    bool operator<(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return lhs.con < rhs.con;
    }

    template <typename T, typename Container>
    bool operator>(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return rhs < lhs;
    }

    template <typename T, typename Container>
    bool operator<=(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return !(rhs < lhs);
    }

    template <typename T, typename Container>
    bool operator>=(const stack<T, Container> &lhs, const stack<T, Container> &rhs) {
        return !(lhs < rhs);
    }


}

#endif //MYSTL_STACK_H
