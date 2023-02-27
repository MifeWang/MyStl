
#ifndef MYSTL_CONSTRUCT_H
#define MYSTL_CONSTRUCT_H
#include "move.h"
#include "iterator.h"
//这部分的内容实际上已经在new_allocator中实现了，但是仍然需要提供一共公共的构造函数接口供其余组件调用。
namespace MyStl{

    template<typename Up, typename... Args>
    inline
    void construct(Up* p, Args&&... args) noexcept{
        ::new((void*)p) Up(forward<Args>(args)...);
    }

    template<typename T1, typename T2>
    inline
    void construct(T1* p, const T2& value){
        ::new((void*) p) T1(value);
    }

    template<typename T>
    inline void destroy(T* ptr) {
        ptr->~T();
    }


    template <typename ForwardIterator>
    inline void destroy_aux(ForwardIterator first,
                            ForwardIterator last,
                            _true_type) {}

    template <typename ForwardIterator>
    inline void destroy_aux(ForwardIterator first,
                            ForwardIterator last,
                            _false_type) {
        for (; first != last; ++first)
            destroy(first);
    }

    // 两个参数的全局 destroy 函数，根据其是否具有 trivial 析构函数进行重载
    template <typename ForwardIterator>
    inline void destroy(ForwardIterator first, ForwardIterator last) {
        using value_type =
                typename MyStl::iterator_traits<ForwardIterator>::value_type;
        using is_trivial_dtor =
                typename type_traits<value_type>::has_trivial_destructor;
        destroy_aux(first, last, is_trivial_dtor());
    }
}
#endif //MYSTL_CONSTRUCT_H
