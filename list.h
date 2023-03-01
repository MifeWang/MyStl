
#ifndef MYSTL_LIST_H
#define MYSTL_LIST_H
#include "iterator.h"
#include "pool_allocator.h"
#include "construct.h"
#include "initializer_list"
namespace MyStl{
    //双向节点的封装
    template<typename T>
    struct list_node{
        list_node<T> *next;
        list_node<T> *prev;
        T data;

        list_node(const T& value = T())
        :data(value)
        ,next(nullptr)
        ,prev(nullptr){}
    };

    //迭代器封装，实现链表指针的自增自减，以及比较等操作
    //这里的模板主要还是参考了标准库。
    //List_const_iterator和List_iterator的最大区别是node是不是const的,又因为模板类型推导时会忽略const
    //所以stl中是直接创建两个结构体来解决的，我们这里尝试使用traits技巧来实现
    template<typename T>
    struct link_node_traits{
        using link_node = list_node<T>;
        using point = T*;
        using reference = T&;
//        int test = 1;
    };
    //偏特化
    template<typename T>
    struct link_node_traits<const T>{
        using link_node = const list_node<T>;
        using point = const T*;
        using reference = const T&;
//        int test = 2;
    };

    template<typename T>
    struct list_iterator{
        using traits_type = link_node_traits<T>;

        using _link_node = typename traits_type::link_node;
        using iterator = list_iterator<T>;
        using self = list_iterator<T>;

        using iterator_category = bidirectional_iterator_tag;
        using value_type = T;
        using pointer = typename traits_type::point;
        using reference = typename traits_type ::reference ;
        using difference_type = ptrdiff_t;
        using size_type = size_t;

        //node是指向list_node的指针
        _link_node* node;
        //构造函数
        list_iterator():node(){}
        list_iterator(_link_node* x):node(x){}
        list_iterator(const iterator& x):node(x.node){}
        //比较操作符
        bool operator==(const list_iterator &rhs) const {
            return node == rhs.node;
        }

        bool operator!=(const list_iterator &rhs) const {
            return !(node == rhs.node);
        }
        //解引用操作符
        reference operator*(){
            return (*node).data;
        }
        pointer operator->() const {
            return &(operator*());
        }
        //加减操作符
        //左+
        self& operator++(){
            node = node->next;
            return *this;
        }
        //右+
        self operator++(int ){
            auto temp = *this;
            ++*this;
            return temp;
        }
        self& operator--() {
            node = (*node).prev;
            return *this;
        }
        self operator--(int) {
            self tmp = *this;
            --*this;
            return tmp;
        }
        //标准库中并没有+n的操作，但list中有，所以直接在这里实现
        self operator+(difference_type n) const{
            self tmp = *this;
            if(n > 0)
                for(; n != 0; --n, ++tmp);
            else
                for(; n != 0; ++n, --tmp);
            return tmp;
        }
        self operator-(difference_type n) const {
            self tmp = *this;
            difference_type i = -n;
            return (tmp + i);
        }
    };

    template<typename T, typename Allocator = MyStl::pool_alloc<list_node<T>>>
    class list{
    protected:
        using link_node = list_node<T>;
    public:
        //类型别名
        using iterator = list_iterator<T>;
        using const_iterator = list_iterator<const T>;
        using reverse_iter = reverse_iterator<iterator>;
        using const_reverse_iter = reverse_iterator<const_iterator>;

        using value_type = T;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using node_pointer = link_node*;
    protected:
        node_pointer node;
        //内部函数
        /*
         * 分配、释放、构造、析构一个节点内存及对象并返回
         */
        //分配，调用分配器的allocate()函数，在内存池中分配一块内存
        node_pointer get_node() {return Allocator::allocate(); }
        //释放, 将这块内存重新放回内存池中对应链表的表头
        void put_node(node_pointer p) { Allocator::deallocate(p);}
        //构造和析构
        node_pointer create_node(const value_type& value);
        void destroy_node(node_pointer p);
        //创建一个空链表
        void empty_initialize();
        //填充一个链表，(n,value)
        void fill_initialize(size_type n, const T& value = T());
        //使用迭代器填充
        template <typename InputIterator>
        void copy_initialize(InputIterator first, InputIterator last);
        //Moves the elements from [first,last) before pos
        void transfer(iterator pos, iterator first, iterator last);
    public:
        //成员函数
        //构造与析构函数
        list() {empty_initialize();}
        explicit list(size_type n) { fill_initialize(n);}
        list(size_type n, const value_type& value) { fill_initialize(n, value); }
        list(int n, const value_type& value) { fill_initialize(size_type(n), value); }
        list(long n, const value_type& value) { fill_initialize(size_type(n), value); }
        template <typename InputIterator>
        list(InputIterator first, InputIterator last) { copy_initialize(first, last); }

        //拷贝构造构造必须是深拷贝
        list(const list<T,Allocator>& rhs){ copy_initialize(rhs.begin(), rhs.end()); }
        list(std::initializer_list<value_type> rhs) { copy_initialize(rhs.begin(), rhs.end());}

        ~list(){
            clear();
            put_node(node);
        }

        list<T, Allocator>& operator=(const list<T, Allocator>& rhs);
        list<T, Allocator>& operator=(std::initializer_list<T> rhs);

        //元素访问
        reference front() { return node->next->data; }
        const_reference front() const { return node->next->data; }
        reference back() { return node->prev->data; }
        const_reference back() const { return node->prev->data; }

        //迭代器
        iterator begin() noexcept { return node->next; }
        const_iterator begin() const noexcept { return node->next; }
        const_iterator cbegin() const noexcept { return begin(); }
        iterator end() noexcept { return node; }
        const_iterator end() const noexcept { return node; }
        const_iterator cend() const noexcept { return end(); }

        reverse_iter rbegin() noexcept { return reverse_iter(end()); }
        const_reverse_iter rbegin() const noexcept {
            return const_reverse_iter(end());
        }
        reverse_iter rend() noexcept { return reverse_iter(begin()); }
        const_reverse_iter rend() const noexcept {
            return const_reverse_iter(begin());
        }

        //容量
        bool empty() const noexcept{return node->next == node;}
        size_type size() const noexcept{ return distance(begin(),end());}
        size_type max_size() const noexcept{ return Allocator::max_size();}

        //修改器
        void clear() { erase(begin(),end());}
        iterator insert(iterator pos, const value_type& value);
        iterator insert(iterator pos, size_type n, const value_type& value);
        void insert(iterator pos, int n, const T& value){ insert(pos, size_type(n), value);}
        void insert(iterator pos, long n, const T& value){ insert(pos, size_type(n), value);}
        template<typename InputIterator>
        void insert(iterator pos, InputIterator first, InputIterator last);
        iterator erase(iterator pos);
        iterator erase(iterator first, iterator last);
        void pop_back() { erase(--end());}
        void pop_front() { erase(begin());}
        void push_back(const value_type& value) { insert(end(),value);}
        void push_front(const value_type& value) { insert(begin(),value);}
        void resize(size_type new_size, const T& value = T());
        void swap(list<T, Allocator>& rhs) { std::swap(node, rhs.node); }

        //操作
        //merge 将other合并到this身上，前提是两个list已经递增排序好了
        void merge(list& other);
        //从一个 list 转移元素给另一个.不复制或移动元素，仅重指向链表结点的内部指针。
        //所以要用到transfer函数
        // 从 other 转移所有元素到 *this 中。
        void splice(iterator pos, list& other );
        //从 other 转移 it 指向的元素到 *this。元素被插入到 pos 指向的元素之前。
        void splice(iterator pos, list& other , iterator it);
        //从 other 转移范围 [first, last) 中的元素到 *this。
        void splice(iterator pos, list& other , iterator first, iterator last);
        //移除所有满足特定标准的元素。第一版本移除所有等于 value 的元素
        void remove(const T& value );
        //逆转容器中的元素顺序。不非法化任何引用或迭代器。
        void reverse() noexcept;
        //从容器移除所有相继的重复元素。只留下相等元素组中的第一个元素。若选择的比较器不建立等价关系则行为未定义。
        void unique();
        //以升序排序元素。保持相等元素的顺序。第一版本用 operator< 比较元素
        //list不能使用STL的排序算法，因为STL的sort只接受随机迭代器
        void sort();

    };

    template<typename T, typename Allocator>
    list<T, Allocator> &list<T, Allocator>::operator=(std::initializer_list<T> rhs) {
        if (&rhs != this) {
            iterator first1 = begin();
            iterator last1 = end();
            const_iterator first2 = rhs.cbegin();
            const_iterator last2 = rhs.cend();
            for (; first1 != last1 && first2 != last2; ++first1, ++first1)
                *first1 = *first2;
            if (first1 == last1)
                insert(last1, first2, last2);
            else
                erase(first1, last1);
        }
        return *this;
    }


    //等于操作符，多退少补策略，使用等于号进行赋值。补的时候使用insert深拷贝，退的时候使用erase析构数据并释放内存
    template<typename T, typename Allocator>
    list<T, Allocator>& list<T, Allocator>::operator=(const list<T, Allocator> &rhs) {
        if (&rhs != this){
            iterator first1 = begin();
            iterator last1 = end();
            const_iterator first2 = rhs.cbegin();
            const_iterator last2 = rhs.cend();
            for (; first1 != last1 && first2 != last2; ++first1, ++first1)
                *first1 = *first2;
            if (first1 == last1)
                insert(last1, first2, last2);
            else
                erase(first1, last1);
        }
        return *this;
    }

    template<typename T, typename Allocator>
    template<typename InputIterator>
    void list<T, Allocator>::copy_initialize(InputIterator first, InputIterator last) {
        empty_initialize();
        try {
            insert(begin(), first, last);
        } catch (...) {
            clear();
            put_node(node);
        }
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::fill_initialize(list::size_type n, const T &value) {
        empty_initialize();
        try {
            insert(begin(), n, value);
        } catch (...){
            clear();
            put_node(node);
        }
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::sort() {
        //链表空或者只有一个节点则不需要比较
        if (node->next == node || node->next->next == node)
            return;
        list carry;
        //使用一个数组来保存排序后的链表，数组大小64，也就是最多可以排序2^63长度的链表
        list tmp[64];
        //fill表示已排序链表的下一个储存位置，所以fill所指的链表应该始终为空
        list* fill = tmp;
        list* counter;
        while (!empty()){
            //使用splice从待排序链表表头取一个节点到carry
            carry.splice(carry.begin(),*this, begin());
            counter = tmp;
            while (counter != fill && !counter->empty()){
                counter->merge(carry);
                carry.swap(*counter);
                ++counter;
            }
            carry.swap(*counter);
            //counter == fill则表明tmp中的所有节点都已经排序完毕，并转移到了fill的位置
            if (counter == fill)
                ++fill;
        }
        for (counter = tmp + 1; counter != fill ; ++counter) {
            counter->merge(*(counter-1));
        }
        swap(*(fill - 1));
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::unique() {
        if (empty())
            return;
        iterator first = begin();
        iterator last = end();
        iterator next = first;
        while (++next != last){
            if (*first == *next){
                first = erase(first);
            } else{
                first = next;
            }
        }
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::reverse() noexcept {
        if (node->next == node || node->next->next == node)
            return;
        iterator first = begin();
        iterator last = end();
        ++first;
        while (first != last){
            iterator old = first;
            ++first;
            transfer(begin(),old,first);
        }
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::remove(const T &value) {
        iterator first = begin();
        iterator last = end();
        while (first != last){
            if (*first == value)
                erase(first++);
        }
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::splice(list::iterator pos, list &other, list::iterator first, list::iterator last) {
        if (first == last)
            return;
        transfer(pos, first, last);

    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::splice(list::iterator pos, list &other, list::iterator it) {
        auto its = it;
        ++its;
        if (pos == its || pos == it)
            return;
        transfer(pos, it, its);
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::splice(list::iterator pos, list &other) {
        if (other.empty())
            return;
        transfer(pos, other.begin(), other.end());
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::merge(list &other) {
        if (other.empty())
            return;
        iterator first1 = begin();
        iterator last1 = end();
        iterator first2 = other.begin();
        iterator last2 = other.end();
        while (first1 != last1 && first2 != last2){
            if (*first2 < *first1){
                splice(first1, other, first2);
                first2 = other.begin();
            } else
                ++first1;
        }
        if (first1 == last1)
            splice(last1, other, first2, last2);
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::resize(list::size_type new_size, const T &value) {
        auto old_size = size();
        if (old_size < new_size){
            insert(end(), new_size - old_size, value);
        } else{
            auto new_end = end();
            for (; new_size != old_size; --old_size, --new_end)
                ;
            erase(new_end, end());
        }
    }

    template<typename T, typename Allocator>
    typename list<T, Allocator>::iterator list<T, Allocator>::erase(list::iterator first, list::iterator last) {
        while (first!=last)
            erase(first++);
        return last;
    }

    template<typename T, typename Allocator>
    typename list<T, Allocator>::iterator list<T, Allocator>::erase(list::iterator pos) {
        auto temp = pos;
        ++temp;
        pos.node->prev->next = temp.node;
        temp.node->prev = pos.node->prev;
        destroy_node(pos.node);
        return temp;
    }

    template<typename T, typename Allocator>
    template<typename InputIterator>
    void list<T, Allocator>::insert(list::iterator pos, InputIterator first, InputIterator last) {
        for (; first != last; ++first)
            insert(pos, *first);
    }

    template<typename T, typename Allocator>
    typename list<T, Allocator>::iterator list<T, Allocator>::insert(list::iterator pos, list::size_type n, const value_type &value) {
        for (; n > 0; --n) {
            insert(pos,value);
        }
    }

    template<typename T, typename Allocator>
    typename list<T, Allocator>::iterator list<T, Allocator>::insert(list::iterator pos, const value_type &value) {
        auto temp = create_node(value);
        pos.node->prev->next = temp;
        temp->prev = pos.node->prev;
        temp->next = pos.node;
        pos.node->prev = temp;
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::transfer(list::iterator pos, list::iterator first, list::iterator last) {
        if (pos != last){
            last.node->prev->next = pos.node;
            first.node->prev->next = last.node;
            pos.node->prev->next = first.node;
            node_pointer temp = pos.node->prev;
            pos.node->prev = last.node->prev;
            last.node->prev = first.node->prev;
            first.node->prev = temp;
        }
    }


    template<typename T, typename Allocator>
    void list<T, Allocator>::empty_initialize() {
        node = get_node();
        node->next = node;
        node->prev = node;
    }

    template<typename T, typename Allocator>
    void list<T, Allocator>::destroy_node(list::node_pointer p) {
        destroy(&p->data);
        put_node(p);
    }

    //先分配内存，再构造
    template<typename T, typename Allocator>
    typename list<T, Allocator>::node_pointer list<T, Allocator>::create_node(const value_type &value) {
        auto p = get_node();
        try {
            construct(&p->data, value);
        } catch (...) {
            put_node(p);
        }
        return p;
    }


}
#endif //MYSTL_LIST_H
