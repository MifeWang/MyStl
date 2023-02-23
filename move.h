//
// Created by 19042 on 2023/2/22.
//
#pragma once
#ifndef MYSTL_MOVE_H
#define MYSTL_MOVE_H

#include "type_traits.h"
namespace MyStl{
    //完美转发函数
    template<class T>
    constexpr T&& forward(remove_reference_t<T>& arg) noexcept{
        // 将左值作为右值转发
        return (static_cast<T&&>(arg));
    }
    template<class T>
    constexpr T&& forward(remove_reference_t<T>&& arg) noexcept{
        // 将右值作为右值转发
        return (static_cast<T&&>(arg));
    }
}
#endif //MYSTL_MOVE_H
