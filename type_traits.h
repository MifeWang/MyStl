//
// Created by 19042 on 2022/11/17.
//
#pragma once
#ifndef MYSTL_TYPE_TRAITS_H
#define MYSTL_TYPE_TRAITS_H


#endif //MYSTL_TYPE_TRAITS_H

//type_traits一共有三部分组成：1、辅助基类；2、类型萃取模板；3、类型转换模板


namespace MyStl{
    //stl类型设置
    using ptrdiff_t = long long int;
    using size_t = unsigned long long int;


    //一、辅助基类。来实现ture_type和false_type
    template<typename T, T v>
    struct integral_constant
    {
        static constexpr T                  value = v;
        using value_type  = T;
        using type = integral_constant<T,v>;
        constexpr operator value_type() const noexcept { return value; }    //函数对象
        constexpr value_type operator()() const noexcept { return value; } //仿函数
    };


//    template<typename T, T v>
//    constexpr T integral_constant<T, v>::value;

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;


    //二、类型萃取模板示例
    //基本思路是使用范围偏特化来实现类型判断

    //is_const
    template <typename>
    struct is_const: public false_type {};
    //只有采用下面偏特化的形式才是const
    template <typename T>
    struct is_const<T const>: public true_type {};

    //is_array


    template <typename>
    struct is_array: public false_type {};
    template<typename T>
    struct is_array<T[]>: public true_type {};
    template<typename T, size_t size>
    struct is_array<T[size]>: public true_type {};


    //三、类型转换修改操作
    //移除const
    template<typename T>
    struct remove_const {using type = T;};
    template<typename T>
    struct remove_const<T const> {using type = T;};
    template<typename T>
    using remove_const_t = typename remove_const<T>::type;

    //移除引用，可以用到完美转发的forward函数
    template<typename T>
    struct remove_reference {using type = T;};
    template<typename T>
    struct remove_reference<T&> {using type = T;};
    template<typename T>
    struct remove_reference<T&&> {using type = T;};
    template<typename T>
    using remove_reference_t = typename remove_reference<T>::type;



    //以上是标准库中的做法
    //要完全实现一个type_traits.h是非常复杂的事情，对于我们来说，只要实现我们需要的功能就可以，这里进行人工标注
    //使用两个具有真假性质的空结构体
    struct _true_type { };
    struct _false_type { };

    template<typename type>
    struct type_traits {
        using has_trivial_default_constructor   = _false_type;
        using has_trivial_copy_constructtor     = _false_type;
        using has_trivial_assignment_operator   = _false_type;
        using has_trivial_destructor            = _false_type;
        using is_POD_type                       = _false_type;
    };

    //特化版本，包括算术类型和指针类型
    template<>
    struct type_traits<bool> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<char> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<unsigned char> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<signed char> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<wchar_t> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<short> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<unsigned short> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<int> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<unsigned int> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<long> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<unsigned long> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<long long> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<unsigned long long> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<float> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<double> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<>
    struct type_traits<long double> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

    template<typename type>
    struct type_traits<type*> {
        using has_trivial_default_constructor   = _true_type;
        using has_trivial_copy_constructtor     = _true_type;
        using has_trivial_assignment_operator   = _true_type;
        using has_trivial_destructor            = _true_type;
        using is_POD_type                       = _true_type;
    };

}