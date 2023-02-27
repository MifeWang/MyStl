
#ifndef MYSTL_VECTOR_H
#define MYSTL_VECTOR_H

#include "pool_allocator.h"
#include "iterator.h"
#include "uninitialized.h"
#include "initializer_list"
namespace MyStl{
    template <typename T, typename Allocator = pool_alloc<T>>
    class vector{
    public:
        //别名设置
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using iterator          = T*;
        using const_iterator    = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = size_t;
        using difference_type   = ptrdiff_t;

        using reverse_iter       = reverse_iterator<iterator>;
        using const_reverse_iter = reverse_iterator<const_iterator>;
    protected:
        //内调函数
        //这部分其实是vector_base的部分，主要是关于内存操作的一些函数和方法
        iterator start;
        iterator finish;
        iterator end_of_storage;
        //释放vector占用的空间
        void deallocate() {
            if (start) Allocator::deallocate(start, end_of_storage - start);
        }
        //使用uninitialized_fill进行填充
        void fill_initialize(size_type n, const T& value);
        //在position插入x，如果空间不足则进行扩充
        //是源码中M_insert_aux和M_realloc_insert的结合
        void insert_aux(iterator position, const T& value);
        //拷贝初始化内置函数,硬拷贝
        template <typename InputIterator>
        void copy_initialize(InputIterator first, InputIterator last);
    public:
        //公开成员函数与接口
        //构造与析构函数
        vector():start(0), finish(0), end_of_storage(0) { }
        explicit vector(size_type n) { fill_initialize(n, T());}
        vector(size_type n, const value_type& value){ fill_initialize(n, value);}
        vector(int n, const T& value) { fill_initialize(n, value); }
        vector(long n, const T& value) { fill_initialize(n, value); }
        vector(const vector<T>& x) { copy_initialize(x.begin(), x.end());}
        template <typename InputIterator>
        vector(InputIterator first, InputIterator last) { copy_initialize(first, last);}
        vector(std::initializer_list<T> L){ copy_initialize(L.begin(), L.end());}

        vector<T, Allocator>& operator=(const vector<T, Allocator>& vec);
        vector<T, Allocator>& operator=(std::initializer_list<T> rhs);

        ~vector(){
            destroy(start, finish);
            deallocate();
        }

        //元素访问
        reference front() noexcept {return *begin();}
        const_reference front() const noexcept {return *begin();}
        reference back() noexcept {return *(end() - 1);}
        const_reference back() const noexcept {return *(end() - 1);}
        reference operator[](size_type pos){return *(start + pos);}
        const_reference operator[](size_type pos) const {return *(start + pos);}
        reference at(size_type n){return (*this)[n];}
        const_reference at(size_type n) const {return (*this)[n];}
        T* data() {return start;}
        const T* data() const {return start;}


        // iterators
        iterator begin() noexcept {return start;}
        const_iterator begin() const noexcept{return start;}
        iterator end() noexcept {return finish;}
        const_iterator end() const noexcept {return finish;}

        reverse_iter rbegin() { return reverse_iter(end()); }
        const_reverse_iter rbegin() const noexcept { return const_reverse_iter(end());}
        reverse_iter rend() noexcept { return reverse_iter(begin());}
        const_reverse_iter rend() const noexcept { return const_reverse_iter(begin());}

        const_iterator cbegin() const noexcept { return const_iterator(begin());}
        const_iterator cend() const noexcept { return const_iterator(end());}
        const_reverse_iter crbegin() const noexcept { return const_reverse_iter(rbegin());}
        const_reverse_iter crend() const noexcept { return const_reverse_iter(rend());}

        // capacity
        size_type size() const {return finish - start;}
        size_type capacity() const {return end_of_storage - start;}
        //计算机能放的最大容积
        size_type max_size() const {return size_type(-1) / sizeof(value_type);}
        void reserve(size_type new_cap);
        bool empty() const{return start == finish;}

        //修改器
        void push_back(const T& value);
        void pop_back();
        void swap(vector<T, Allocator>& other);
        iterator insert(iterator pos, const value_type& value = T()){
            auto offset = pos - cbegin();
            if (finish != end_of_storage && pos == finish)
                construct(finish, value);
            else
                insert_aux(pos, value);
            return start + offset;
        }
        void insert(iterator pos, size_type n, const value_type& value);
        iterator erase(iterator pos);
        iterator erase( iterator first, iterator last );
        void resize( size_type count, const value_type& value = T());
        void clear(){ erase(start,finish);}
    };


    template<typename T, typename Allocator>
    void vector<T, Allocator>::resize(vector::size_type count, const value_type &value) {
        if (count < size())
            erase(start + count, finish);
        else
            insert(finish, count - size(), value);
    }

    template<typename T, typename Allocator>
    typename vector<T, Allocator>::iterator vector<T, Allocator>::erase(iterator first, iterator last) {
        iterator new_finish = std::copy(last, finish, first);
        destroy(new_finish, finish);
        finish = new_finish;
        return first;
    }

    template<typename T, typename Allocator>
    typename vector<T, Allocator>::iterator vector<T, Allocator>::erase(iterator pos) {
        if (pos != finish - 1){
            std::copy(pos + 1, finish, pos);
        }
        --finish;
        destroy(finish);
        return pos;
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::insert(iterator pos, size_type n, const value_type& value) {
        if (n == 0) return;
        //还有容量
        if (size() + n < capacity()){
            const size_type elems_after = finish - pos;
            //为了节省开销,所以需要比较插入的数量和插入点之后原有元素的数量
            if (elems_after > n){
                //如果插入的数量比较少, 那么先后移再插入
                //把末尾的n个元素,初始化copy到end()起始
                uninitialized_copy(finish - n, finish, finish);
                //以finish为终点,使用copy_backward进行copy
                std::copy_backward(pos, finish - n, finish);
                std::fill(pos, pos + n, value);
            } else{
                //插入的数量较多,则没有必要进行copy_backward操作
                //先把末尾进行填充
                uninitialized_fill_n(finish, n - elems_after, value);
                //再把pos之后的原数据,转移到应该在的位置
                uninitialized_copy(pos, finish, pos + n);
                //最后插入
                std::fill(pos, pos + n, value);
            }
            //新的end位置
            finish += n;
        }
        //容积不够,先申请内存,然后在新内存上执行拷贝操作
        else{
            const size_type old_size = size();
            const size_type new_size = (old_size == 0) ? 10 : 2 * old_size;
            iterator new_start = Allocator::allocate(new_size);
            iterator new_finish = new_start;
            //程序员控制释放内存
            try {
                new_finish = uninitialized_copy(start, pos, new_start);
                new_finish = uninitialized_fill_n(new_finish, n, value);
                new_finish = uninitialized_copy(pos, finish, new_finish);
            } catch (...) {
                //先析构再释放内存空间
                //destroy(new_start, new_finish);
                Allocator::deallocate(new_start, new_size);
                throw;
            }
            //无异常则析构并释放原内存
            destroy(start, finish);
            deallocate();
            start = new_start;
            finish = new_finish;
            end_of_storage = start + new_size;
        }
    }

    //vector的swap就是把三个指针进行交换
    template<typename T, typename Allocator>
    void vector<T, Allocator>::swap(vector<T, Allocator> &other) {
        std::swap(start, other.start);
        std::swap(finish, other.finish);
        std::swap(end_of_storage, other.end_of_storage);
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::pop_back() {
        --finish;
        destroy(finish);
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::push_back(const T &value) {
        if (finish != end_of_storage)
            construct(finish, value);
        else
            insert_aux(end(), value);
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::reserve(vector::size_type new_cap) {
        if (capacity() < new_cap){
            iterator new_start = Allocator::allocate(new_cap);
            iterator new_finish = new_start;
            try {
                new_finish = uninitialized_copy(start, finish, new_start);
            } catch(...) {
                //uninitialized_copy负责了析构
                //destroy(new_start, new_finish)
                Allocator::deallocate(new_start, new_cap);
            }
            destroy(start, finish);
            deallocate();
            start = new_start;
            finish = new_finish;
            end_of_storage = start + new_cap;
        }
    }

    //拷贝赋值函数
    template<typename T, typename Allocator>
    vector<T, Allocator> &vector<T, Allocator>::operator=(const vector<T, Allocator> &vec) {
        if (&vec != this){
            size_type new_size = vec.capacity();
            //如果要拷贝的容积大于现有的容积,则要重新分配内存
            if (new_size > capacity()){
                iterator new_start = Allocator::allocate(new_size);
                end_of_storage = uninitialized_copy(vec.begin(), vec.end(), new_start);
                destroy(start,finish);
                deallocate();
                start = new_start;
            }
            //如果比现在的size还行,那么拷贝后还需要清楚原来剩余的内容
            else if (new_size < size()){
                iterator it = std::copy(vec.begin(), vec.end(), start);
                destroy(it, finish);
            }
            //size() < new_size < capacity()
            else {
                std::copy(vec.begin(), vec.begin() + size(), start);
                uninitialized_copy(vec.begin() + size(), vec.end(), finish);
            }
            finish = start + new_size;
        }
        return *this;
    }

    //从初始化列表的拷贝赋值函数,这里使用swap一个局部临时变量的方法,来对原内存空间进行析构.
    template<typename T, typename Allocator>
    vector<T, Allocator> &vector<T, Allocator>::operator=(std::initializer_list<T> rhs) {
        vector<T, Allocator> temp(rhs.begin(), rhs.end());
        swap(temp);
        return *this;
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::fill_initialize(vector::size_type n, const T &value) {
        //分配n个value_type的内存
        start = Allocator::allocate(n);
        //维护内存分配与释放
        //uninitialized_fill_n处理的了析构的异常情况，在这里我们需要处理内存分配的情况
        try {
            uninitialized_fill_n(start, n, value);
            finish = start + n;
            end_of_storage = finish;
        } catch (...) {
            Allocator::deallocate(start, n);
        }
    }

    template<typename T, typename Allocator>
    template<typename InputIterator>
    void vector<T, Allocator>::copy_initialize(InputIterator first, InputIterator last) {
        size_type n = last - first;
        start = Allocator::allocate(n);
        try {
            uninitialized_copy(first, last, start);
            finish = start + n;
            end_of_storage = finish;
        } catch (...) {
            Allocator::deallocate(start, n);
        }
    }

    template<typename T, typename Allocator>
    void vector<T, Allocator>::insert_aux(vector::iterator position, const T &value) {
        //还有容量
        //先将最后一个元素在end的位置构造
        if (finish != end_of_storage){
            construct(finish, *(finish-1));
            ++finish;
            //使用stl算法，将position起的元素逐后copy一位
            std::copy_backward(position, finish-2, finish - 1);
            //插入
            *position = value;
        } else{
        //内存不足
        //新分配2倍内存，然后把旧的copy到新内存，然后把旧存析构并释放
            const size_type old_size = size();
            const size_type new_size = (old_size == 0) ? 10 : 2 * old_size;
            iterator new_start = Allocator::allocate(new_size);
            iterator new_finish = new_start;
            //程序员控制释放内存
            try {
                new_finish = uninitialized_copy(start, position, new_start);
                construct(new_finish++, value);
                new_finish = uninitialized_copy(position, finish, new_finish);
            } catch (...) {
            //先析构再释放内存空间
                //destroy(new_start, new_finish);
                Allocator::deallocate(new_start, new_size);
                throw;
            }
            //无异常则析构并释放原内存
            destroy(start, finish);
            deallocate();
            start = new_start;
            finish = new_finish;
            end_of_storage = start + new_size;
        }
    }

}

#endif //MYSTL_VECTOR_H
