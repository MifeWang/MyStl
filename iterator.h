
#ifndef MYSTL_ITERATOR_H
#define MYSTL_ITERATOR_H

#include "move.h"

/*
 * 对于iterator主要实现三步的内容，
 * 分别对应stl当中的stl_iterator_base_types.h
 * 和stl_iterator_base_funcs.h
 * 以及部分的bits/stl_iterator.h，主要是std::reverse_iterator< _Iterator >
 */

namespace MyStl {
    /*
     * 这些空结构体用于区分不同的类型。不同的底层算法可以根据所支持的迭代器类型来选择底层迭代器
     * 这样的话基于继承关系就可以剩下不必要的函数重载
     */
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : public input_iterator_tag {};
    struct bidirectional_iterator_tag : public forward_iterator_tag {};
    struct random_access_iterator_tag : public bidirectional_iterator_tag {};

    /*
     * 所有迭代器公用的萃取对象
     */
    template<typename Category, typename T, typename Distance = ptrdiff_t,
            typename Pointer = T*, typename Reference = T&>
    struct iterator{
        using iterator_category = Category;
        using value_type        = T;
        using difference_type   = Distance;
        using pointer           = Pointer;
        using reference         = Reference;
    };

    /*
     * 有的容器使用的迭代器可能是原生指针或者const指针。
     * 那么对于这些类别来说是不能直接使用上面迭代器的萃取对象的
     * 所以我们需要专门为指针提供萃取对象，通过traits技术
     */

    //萃取技术本质上就是一种模板特例化的技术。
    //非特化版本是最后需要统一的格式，特化版本则提供相应的转换
    // 迭代器的 traits
    template <typename Iterator>
    struct iterator_traits{
        using value_type        = typename Iterator::value_type;
        using iterator_category = typename Iterator::iterator_category;
        using difference_type   = typename Iterator::difference_type;
        using pointer           = typename Iterator::pointer;
        using reference         = typename Iterator::reference;
    };

    // 偏特化版本
    //这里相比stl没有函数对象的偏特化版本实现
    template <typename T>
    struct iterator_traits<T*>{
        using value_type        = T;
        using iterator_category = random_access_iterator_tag;
        using difference_type   = ptrdiff_t;
        using pointer           = T*;
        using reference         = T&;
    };

    template <typename T>
    struct iterator_traits<const T*>{
        using value_type        = T;
        using iterator_category = random_access_iterator_tag;
        using difference_type   = ptrdiff_t;
        using pointer           = const T*;
        using reference         = const T&;
    };

    /*
     * 语法的sugar仅供库内部使用
     * 相当于给模板取别名的一种方法
     */

    template<typename Iterator>
    inline typename Iterator::iterator_category
    iterator_category(const Iterator&){
        return typename iterator_traits<Iterator>::iterator_category() ;
    }

    //c++14中的using别名声明
    //使用typedef给模板定义别名需要用到struct，而using则不用
    template<typename Iterator>
    using iterator_category_t = typename iterator_traits<Iterator>::iterator_category;


    template <typename Iterator>
    inline typename iterator_traits<Iterator>::difference_type* difference_type(const Iterator&) {
        return static_cast<typename iterator_traits<Iterator>::difference*>(0);
    }

    template <typename Iterator>
    inline typename iterator_traits<Iterator>::value_type* value_type(const Iterator&) {
        return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
    }

    //有了上面的内容，我们就可以进行具体迭代器功能的开发了。
    //但是因为迭代器不止为容器提供服务，还要为算法提供服务，所以我们还需要为算法中计算两个迭代器的距离提供服务
    //所以我们还需要提供所以迭代器的通用函数服务distance() and advance().
    //stl_iterator_base_funcs.h

    //接口
    //使用包转发，内部使用重载函数，根据迭代器不同的类别转发到不同的函数
    template<typename InputIterator, typename Distance>
    inline void
    advance(InputIterator& i, Distance n){
        typename iterator_traits<InputIterator>::difference_type d = n;
        _advance(i, d, iterator_category(i));
    }

    template<typename InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last){
        return _distance(first, last, iterator_category(first));
    }

    //转发的重载函数
    //输入迭代器版本, 单项迭代器基于继承关系也能够使用
    template<typename InputIterator, typename Distance>
    inline void
    _advance(InputIterator& i, Distance n, input_iterator_tag){
        while (n--)
            ++i;
    }

    //双向迭代器
    template<typename InputIterator, typename Distance>
    inline void
    _advance(InputIterator& i, Distance n, bidirectional_iterator_tag){
        if (n > 0)
            while (n--)
                ++i;
        else
            while (n++)
                --i;
    }

    //随机访问迭代器
    template<typename InputIterator, typename Distance>
    inline void
    _advance(InputIterator& i, Distance n, random_access_iterator_tag){
        i += n;
    }

    //distance的转发重载函数
    template<typename InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    _distance(InputIterator first, InputIterator last, input_iterator_tag){
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last){
            ++first;
            ++n;
        }
        return n;
    }

    template<typename InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    _distance(InputIterator first, InputIterator last, random_access_iterator_tag){
        return last - first;
    }


    //bits/stl_iterator.h中std::reverse_iterator< _Iterator >的实现
    //继承iterator结构体的作用是使用上面定义的traits功能
    //从而可以通过typename reverse_iterator<Iterator>::value_type 来调用对象
    template<typename Iterator>
    class reverse_iterator
    :public iterator<typename iterator_traits<Iterator>::iterator_category,
                     typename iterator_traits<Iterator>::value_type,
                     typename iterator_traits<Iterator>::difference_type,
                     typename iterator_traits<Iterator>::pointer,
                     typename iterator_traits<Iterator>::reference>
    {
    protected:
        //内置正向迭代器
        Iterator current;
        using _traits_type = iterator_traits<Iterator>;
    public:
        using iterator_type = Iterator;
        using value_type = typename _traits_type ::value_type ;
        using difference_type = typename _traits_type::difference_type;
        using pointer = typename _traits_type::pointer;
        using reference = typename _traits_type::reference ;
        reverse_iterator(): current(){}
        explicit reverse_iterator(iterator_type x):current(x){}
        //默认拷贝构造函数
        //返回内置正向迭代器
        iterator_type base() const {return current;}

        //算数运算操作符重载
        //对于逆向迭代器取值，就是将“对应的正向迭代器”后退一个而后取值。
        reference operator*() const{
            Iterator temp = current;
            return *--temp;
        }

        pointer operator->() const{
            return &(operator*());
        }

        //前置++
        reverse_iterator&
        operator++() {
            --current;
            return *this;
        }

        //后置++
        //后置++返回的是加之前的拷贝
        reverse_iterator
        operator++(int ) {
            Iterator temp = *this;
            --current;
            return temp;
        }

        reverse_iterator&
        operator--() {
            ++current;
            return *this;
        }

        //后置--
        reverse_iterator
        operator--(int ) {
            Iterator temp = *this;
            ++current;
            return temp;
        }

        reverse_iterator
        operator+(difference_type n) const {
            return reverse_iterator(current - n);
        }

        reverse_iterator&
        operator+=(difference_type n){
            current -= n;
            return *this;
        }

        reverse_iterator
        operator-(difference_type n) const {
            return reverse_iterator(current + n);
        }

        reverse_iterator&
        operator-=(difference_type n){
            current += n;
            return *this;
        }

        reference
        operator[](difference_type n) const{
            return *(*this + n);
        }

        friend bool operator==(const reverse_iterator &lhs, const reverse_iterator &rhs);
    };

    //比较(==、<、!=、>、<=、>=)运算符重载
    template<typename Iterator>
    inline bool operator==(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return  lhs.base() == rhs.base();
    }

    template<typename Iterator>
    inline bool operator<(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return  lhs.base() < rhs.base();
    }

    template<typename Iterator>
    inline bool operator-(const reverse_iterator<Iterator> &lhs, const reverse_iterator<Iterator> &rhs) {
        return  lhs.base() - rhs.base();
    }

    template<typename Iterator>
    inline reverse_iterator<Iterator> operator+(typename reverse_iterator<Iterator>::difference_type n,
            const reverse_iterator<Iterator> &x) {
        return reverse_iterator<Iterator>(x.base() - n);
    }

}

#endif //MYSTL_ITERATOR_H
