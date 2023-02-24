
#ifndef MYSTL_UNINITIALIZED_H
#define MYSTL_UNINITIALIZED_H

//实现一下在分配好的内存中填充元素的初始化工具
#include "iterator.h"
#include "construct.h"
#include "iostream"
namespace MyStl{
    //根据是不是POD类型，选择不同函数重载
    template <typename InputIterator, typename ForwardIterator>
    inline ForwardIterator uninitialized_copy(InputIterator first,
                                              InputIterator last,
                                              ForwardIterator result) {
        using value_type =
                typename MyStl::iterator_traits<ForwardIterator>::value_type;
        using is_POD = typename type_traits<value_type>::is_POD_type;
        return _uninitialized_copy(first, last, result, is_POD());
    }

    //如果是POD类型，或者叫TrivialValueTypes，那么直接调用copy函数，也就是软拷贝
    template <typename InputIterator, typename ForwardIterator>
    inline ForwardIterator uninitialized_copy_aux(InputIterator first,
                                                  InputIterator last,
                                                  ForwardIterator result,
                                                  _true_type) {
        return std::copy(first, last, result);
    }

    //如果不是POD类型，则需要调用对应的构造函数
    template <typename InputIterator, typename ForwardIterator>
    inline ForwardIterator uninitialized_copy_aux(InputIterator first,
                                                  InputIterator last,
                                                  ForwardIterator result,
                                                  _false_type){
        ForwardIterator cur = result;
        try {
            for (; first != last ; ++first, ++cur) {
                construct(cur, *first);
                return cur;
            }
        } catch (...) {
            destroy(result, cur);
        }
    }

    template <typename ForwardIterator, typename T>
    inline void uninitialized_fill(ForwardIterator first,
                                   ForwardIterator last,
                                   const T& value) {
        using value_type =
                typename MyStl::iterator_traits<ForwardIterator>::value_type;
        using is_POD = typename type_traits<value_type>::is_POD_type;
        uninitialized_fill_aux(first, last, value, is_POD());
    }

    template <typename ForwardIterator, typename T>
    inline void uninitialized_fill_aux(ForwardIterator first,
                                       ForwardIterator last,
                                       const T& value,
                                       _true_type) {
        std::fill(first, last, value);
    }

    template <typename ForwardIterator, typename T>
    inline void uninitialized_fill_aux(ForwardIterator first,
                                       ForwardIterator last,
                                       const T& value,
                                       _false_type) {
        ForwardIterator cur = first;
        try {
            for (; cur != last; ++cur)
                construct(cur, value);
            return cur;
        } catch (...) {
            destroy(first, cur);
            throw;
        }
    }

    template <typename ForwardIterator, typename T>
    inline ForwardIterator uninitialized_fill_n(ForwardIterator first,
                                                size_t n,
                                                const T& value) {
        using value_type =
                typename MyStl::iterator_traits<ForwardIterator>::value_type;
        using is_POD = typename type_traits<value_type>::is_POD_type;
        return uninitialized_fill_n_aux(first, n, value, is_POD());
    }

    template <typename ForwardIterator, typename T>
    inline ForwardIterator uninitialized_fill_n_aux(ForwardIterator first,
                                                    size_t n,
                                                    const T& value,
                                                    _true_type) {
        return std::fill_n(first, n, value);
    }

    template <typename ForwardIterator, typename T>
    inline ForwardIterator uninitialized_fill_n_aux(ForwardIterator first,
                                                    size_t n,
                                                    const T& value,
                                                    _false_type) {
        ForwardIterator cur = first;
        try {
            for (; n != 0; --n, ++cur)
                construct(cur, value);
            return cur;
        } catch (...) {
            destroy(first, cur);
            throw;
        }
    }


}



#endif //MYSTL_UNINITIALIZED_H
