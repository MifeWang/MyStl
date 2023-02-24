
#ifndef MYSTL_NEW_ALLOCATOR_H
#define MYSTL_NEW_ALLOCATOR_H

#include "move.h"
namespace MyStl{
    //实现stl中的默认分配器，new_allocator


    template <typename T>
    class new_allocator{
    public:
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = size_t;
        using difference_type   = ptrdiff_t;

        //使T类型的allocator可以为T1类型的对象分配内存
        template<typename T1>
        struct rebind{
            using other = new_allocator<T1>;
        };

        //使用默认构造函数
        //...
        //获取对象的地址
        pointer
        address(reference x) const{
            return  pointer(&x);
        }
        const_pointer
        address(const_reference x) const{
            return  pointer(&x);
        }

        //allocator
        //::operator new返回的是void*类型，需要做一个强制类型转换来确保类型安全。
        static T*
        allocate(){
            return static_cast<T*>(::operator new(sizeof(T)));
        }
        static T*
        allocate(size_type n){
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }

        //deallocate
        static void
        deallocate(T* ptr){
            ::operator delete(ptr);
        }
        /* gcc */
        static void
        deallocate(T* ptr, size_type){
            ::operator delete(ptr);
        }

        //construct, 构造
        template<typename Up, typename... Args>
        static
        void construct(Up* p, Args&&... args) noexcept{
            ::new((void *)p) Up(forward<Args>(args)...);
        }

        //destroy。 析构
        //直接析构
        /*
        template<typename Up>
        void destroy(Up* p){
            p->~Up();
        }
         */
        //如果对象是trivial destructor，那么destroy函数可以什么也不做
        template<typename Up>
        static void destroy(Up* p) {
            using is_trivial_dtor = typename type_traits<Up>::has_trivial_destructor;
            destroy_aux(p, is_trivial_dtor());
        }
        template<typename Up>
        static void destroy_aux(Up* p, _false_type){
            p->~Up();
        }
        template<typename Up>
        static void destroy_aux(Up* p, _true_type){
        }


        //max_size,返回最大的可分配数量
        //size_t(~0)获取字节数
        static size_type max_size() {
            return size_type(~0) / sizeof(value_type);
        }
    };
}


#endif //MYSTL_NEW_ALLOCATOR_H
