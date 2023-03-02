
#ifndef MYSTL_DEQUE_H
#define MYSTL_DEQUE_H
#include "iterator.h"
#include "pool_allocator.h"
#include "construct.h"
#include "uninitialized.h"
#include "initializer_list"
#include "iostream"
namespace MyStl{
    //缓存区辅助函数，buf_size是用户设计的size，如果用户没设置，则使用默认的内存空间大小(512字节)
    //这里的size_t指代的是元素的个数，val_size是单个元素所占的内存空间
    inline size_t deque_buf_size(size_t buf_size, size_t val_size) {
        return (buf_size != 0) ? buf_size
                               : ( (val_size < 512) ? size_t(512 / val_size) : size_t(1) );
    }

    //和list一样，deque也需要自己设置迭代器，因为
    template <typename T, typename Ref, typename Ptr>
    struct deque_iterator {

        using iterator_category = random_access_iterator_tag;
        using value_type = T;
        using pointer = Ptr;
        using reference = Ref;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using map_pointer = T**;

        using iterator = deque_iterator<T, T&, T*>;
        using const_iterator = deque_iterator<T, const T&, const T*>;
        using self = deque_iterator;

        //内部指针
        //cur, first, last用来控制缓冲区；和vector类似
        //对于finish，cur指向的是填充下一个元素的位置
        //对于start，cur指向的是首元素的位置
        T* cur;
        T* first;
        T* last;
        map_pointer node;  //指向中控器中当前迭代器所指节点

        //返回默认分配内存能够容纳的元素个数
        static size_t buffer_size() { return deque_buf_size(0, sizeof(T)); }
        //set_node实现中控器节点的跳转，跳转之后node指向新的节点，其余三个指针也指向新的缓冲区
        void set_node(map_pointer new_node) {
            node = new_node;
            first = *new_node;
            last = first + difference_type(buffer_size());
        }

        //构造函数
        deque_iterator(T* value_ptr, map_pointer node_ptr)
                : cur(value_ptr),
                  first(*node_ptr),
                  last(*node_ptr + buffer_size()),
                  node(node_ptr) {}
        deque_iterator() : cur(), first(), last(), node() {}
        deque_iterator(const iterator& iter) :cur(iter.cur), first(iter.first), last(iter.last), node(iter.node) {}

        //操作符重载
        //随机访问迭代器类型需要提供两个迭代器相减的重载操作
        difference_type operator-(const self& iter) const {
            return difference_type(buffer_size()) * (node - iter.node - 1) +
                   (cur - first) + (iter.last - iter.cur);
        }

        deque_iterator& operator=(const deque_iterator&) = default;

        reference operator*() { return *cur;}
        pointer operator->() { return &(operator*());}

        self& operator++() noexcept{
            ++cur;
            if (cur == last){
                set_node(node + 1);
                cur = first;
            }
            return *this;
        }

        self operator++(int ) noexcept{
            self temp = *this;
            ++*this;
            return temp;
        }

        self& operator--() noexcept{
            if (cur == first){
                set_node(node - 1);
                cur = last;
            }
            --cur;
            return *this;
        }

        self operator--(int ) noexcept{
            self temp = *this;
            --*this;
            return temp;
        }

        self& operator+=(difference_type n) {
            const difference_type offset = n + (cur - first);
            if (offset >= 0 && offset < difference_type(buffer_size())){
                cur += n;
            } else {
                const difference_type node_offset = offset > 0 ? difference_type(offset / buffer_size())
                        : -difference_type((-offset - 1) / buffer_size()) - 1;
                set_node(node + node_offset);
                cur = first + (offset - node_offset * difference_type(buffer_size()));
            }
            return *this;
        }

        //使用类内成员函数的形式重载 + 运算符。注意：+ 运算符不改变 this 指针
        self operator+(difference_type n) const {
            self tmp = *this;
            return tmp += n;
        }

        self& operator-=(difference_type n) { return *this += -n; }

        self operator-(difference_type n) const {
            self tmp = *this;
            return tmp -= n;
        }

        reference operator[](difference_type n) const { return *(*this + n);}

        //关系操作符
        bool operator==(const self& iter) const { return cur == iter.cur; }
        bool operator!=(const self& iter) const { return !(*this == iter); }
        bool operator<(const self& iter) const {
            return (node == iter.node) ? (cur < iter.cur) : (node < iter.node);
        }
    };

    template<typename T>
    class deque{
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using reference = T&;
        using const_reference = const T&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using iterator = deque_iterator<T, T&, T*>;
        using const_iterator = deque_iterator<T, const T&, const T*>;
        using reverse_iter = reverse_iterator<iterator>;
        using const_reverse_iter = reverse_iterator<const_iterator>;

    protected:
        using map_pointer = pointer*;
        //deque不同的地方在于需要分配两块内存，一块是中控器map，一块是缓冲区
        using data_alloc = pool_alloc<value_type>;
        using map_alloc = pool_alloc<pointer>;

        //deque内部成员
        //vector中end_of_storage相对于deque中的map_size，记录最大容积
        map_pointer map = 0;
        size_type map_size = 0;
        iterator start;
        iterator finish;

    protected:
        //辅助函数
        static size_type buffer_size() { return deque_buf_size(0, sizeof(T)); }
        static size_type init_map_size() { return 8; }

        /*内存操作
         * 相比标准库少了allocate_map和deallocate_map，我们直接调用map_alloc，所以省略
         * 其实allocate_node也可以省略
         */
        //申请缓冲区内存
        pointer allocate_node() { return data_alloc::allocate(buffer_size());}
        void deallocate_node(pointer ptr) { data_alloc::deallocate(ptr); }
        //负责产生和回收map结构，不设初值
        void create_map_nodes(size_type num_element);
        //destroy_map_nodes相对于分别调用调用data_alloc和map_alloc的deallocate函数，释放内存
        void destroy_map_nodes();
        //产生新的map，复制旧map，然后摧毁;相对于扩容操作，所以很有用
        void reallocate_map(size_type nodes_to_add, bool add_at_front);


        /*初始化操作*/
        //因为deque缓冲区为连续空间，所以我们可以使用uninitialized函数进行填充
        //我们在create_map_nodes中只是分配和划分了内存，而在空白内存上拷贝新元素，通常使用构造函数的方法
        void fill_initialize(size_type n, const value_type& value = value_type());
        template <typename InputIterator>
        void copy_initialize(InputIterator first, InputIterator last);

        /*内调函数*/
        //插入的内调函数
        void insert_aux(iterator pos, size_type n, const value_type& value);
        //新分配缓冲区。在中控器的前后节点制造出一块可分配空间来，大小为n个元素对象，data_alloc。
        iterator reserve_elements_at_front(size_type n);
        iterator reserve_elements_at_back(size_type n);
        //分配元素失败，则要进行deallocate
        void destroy_nodes_at_front(iterator before_start);
        void destroy_nodes_at_back(iterator after_finish);
        //新分配map区
        void reserve_map_at_back(size_type nodes_to_add = 1) {
            if (nodes_to_add > map_size - (finish.node - map + 1))
                // 如果 map 尾端的节点备用空间不足
                // 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
                reallocate_map(nodes_to_add, false);
        }

        void reserve_map_at_front(size_type nodes_to_add = 1) {
            if (nodes_to_add > size_type(start.node - map))
                // 如果 map 前端的节点备用空间不足
                // 符合以上条件则必须重换一个map（配置更大的，拷贝原来的，释放原来的）
                reallocate_map(nodes_to_add, true);
        }

    public:
        /*构造与析构*/
        deque(){ create_map_nodes(0);}
        explicit deque(size_type n) { fill_initialize(n); }
        deque(size_type n, const value_type& value) { fill_initialize(n, value); }
        deque(int n, const value_type& value) { fill_initialize(n, value); }
        deque(long n, const value_type& value) { fill_initialize(n, value); }
        template <typename InputIterator>
        deque(InputIterator first, InputIterator last) { copy_initialize(first, last); }

        //拷贝构造
        deque(const deque& x) { copy_initialize(x.begin(), x.end()); }
        deque(const std::initializer_list<T>& il) { copy_initialize(il.begin(), il.end());}

        //拷贝赋值
        deque& operator=(const deque& rhs);

        //析构
        ~deque(){
            destroy(start, finish);
            destroy_map_nodes();
        }


    public:
        //元素访问
        reference operator[](size_type n) { return start[difference_type(n)];}
        const_reference operator[](size_type n) const {
            return start[difference_type(n)];
        }
        reference front() { return *start;}
        const_reference front() const { return *start;}
        reference back() {
            auto temp = finish;
            --temp;
            return *temp;
        }
        const_reference back() const{
            auto temp = finish;
            --temp;
            return *temp;
        }

        //迭代器
        iterator begin() { return start; }
        iterator end() { return finish; }
        const_iterator begin() const { return start; }
        const_iterator end() const { return finish; }

        const_iterator cbegin() const { return begin(); }
        const_iterator cend() const { return end(); }

        reverse_iter rbegin() noexcept { return reverse_iter(end()); }
        const_reverse_iter rbgein() const noexcept {
            return const_reverse_iter(end());
        }
        reverse_iter rend() noexcept { return reverse_iter(begin()); }
        const_reverse_iter rend() const noexcept {
            return const_reverse_iter(begin());
        }

        //容量
        size_type size() const { return finish - start; }
        bool empty() const { return finish == start; }
        size_type max_size() const { return data_alloc::max_size(); }

        //修改器
        void swap(deque& deq);
        void push_back(const value_type& value);
        void push_front(const value_type& value);
        void pop_back();
        void pop_front();
        iterator insert(iterator pos, const value_type& value = T());
        iterator insert(iterator pos, size_type n, const value_type& value);
        void insert(iterator pos, int n, const T& value){ insert(pos, size_type(n), value);}
        void insert(iterator pos, long n, const T& value){ insert(pos, size_type(n), value);}
        template<typename InputIterator>
        void insert(iterator pos, InputIterator first, InputIterator last);
        iterator erase(iterator pos);
        iterator erase(iterator first, iterator last);
        void clear();
        void resize(size_type new_size, const value_type& value);
        void resize(size_type new_size) { resize(new_size, T()); }
    };

    template<typename T>
    void deque<T>::resize(deque::size_type new_size, const value_type &value) {
        size_type len = size();
        if (new_size < size())
            erase(start + new_size, finish);
        else
            insert(finish, new_size - size(), value);
    }

    template<typename T>
    void deque<T>::clear() {
        //clear与析构不同的地方在于，clear并不会把map给释放掉，只是析构和释放缓冲区
        //析构所有元素
        destroy(start, finish);
        //除了头缓冲区，其余缓冲区全部释放
        for (map_pointer temp = start.node + 1; temp <= finish.node; ++temp) {
            deallocate_node(*temp);
        }
        start.cur = start.first;
        finish = start;
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::erase(deque::iterator first, deque::iterator last) {
        if (first == start && last == finish) {
            clear();
            return finish;
        } else {
            difference_type n = last - first;
            difference_type elems_before = first - start;
            if (elems_before < (size() - n) / 2) {  //前面的元素比较少
                std::copy_backward(start, first, last);
                iterator new_start = start + n;
                destroy(start, new_start);
                //将多余缓冲区释放
                for (map_pointer temp = start.node; temp < new_start.node; ++temp) {
                    deallocate_node(*temp);
                }
                start = new_start;
            } else {  //后面元素比较少
                std::copy(last, finish, first);
                iterator new_finish = finish - n;
                destroy(new_finish, finish);
                for (map_pointer temp = new_finish.node + 1; temp <= finish.node; ++temp) {
                    deallocate_node(*temp);
                }
                finish = new_finish;
            }
            return start + elems_before;
        }
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::erase(deque::iterator pos) {
        iterator next = pos;
        ++next;
        difference_type index = pos - start;
        if (index < (size() / 2)) {
            std::copy_backward(start, pos, next);
            pop_front();
        } else {
            std::copy(next, finish, pos);
            pop_back();
        }
        return start + index;
    }

    template<typename T>
    template<typename InputIterator>
    void deque<T>::insert(deque::iterator pos, InputIterator first, InputIterator last) {
        std::copy(first, last, std::inserter(*this, pos));
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::insert(deque::iterator pos, deque::size_type n, const value_type &value) {
        if (pos.cur == start.cur) {
            iterator new_start = reserve_elements_at_front(n);
            uninitialized_fill(new_start, start, value);
            start = new_start;
        } else if (pos.cur == finish.cur) {
            iterator new_finish = reserve_elements_at_back(n);
            uninitialized_fill(finish, new_finish, value);
            finish = new_finish;
        } else
            insert_aux(pos, n, value);
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::insert(deque::iterator pos, const value_type &value) {
        //先检查插入点是不是前后端
        if (pos.cur == start.cur) {
            push_front(value);
            return start;
        } else if (pos.cur == finish.cur) {
            push_back(value);
            iterator tmp = finish;
            --tmp;
            return tmp;
        } else {
            //和insert_aux类似的操作
            difference_type index = pos - start;
            if (index < difference_type(size() / 2)){
                //操作pos之前的数据
                push_front(front());
                iterator old_start = start + 1;
                iterator old_second = old_start + 1;
                pos = start + index;
                std::copy(old_second, pos + 1, old_start);
            } else{
                //操作pos后面的数据
                push_back(back());
                iterator old_finish = finish - 1;
                iterator old_finish2 = old_finish - 1;
                pos = start + index;
                std::copy_backward(pos, old_finish2, old_finish);
            }
            //因为是已经构造好的空间，所以直接使用operator=即可
            *pos = value;
        }
    }

    template <typename T>
    void deque<T>::pop_back() {
        //要先判断最后一个缓冲区有没有元素，如果没有则需要负责释放最后的缓冲区
        if (finish.cur != finish.first) {
            --finish.cur;
            destroy(finish.cur);
        } else {
            deallocate_node(finish.first);
            finish.set_node(finish.node - 1);
            finish.cur = finish.last - 1;
            destroy(finish.cur);
        }
    }

    template <typename T>
    void deque<T>::pop_front() {
        destroy(start.cur);
        if (start.cur != start.last - 1) {
            ++start.cur;
        } else {
            deallocate_node(start.first);
            start.set_node(start.node + 1);
            start.cur = start.first;
        }
    }

    template<typename T>
    void deque<T>::push_front(const value_type &value) {
        if (start.cur != start.first) {
            --start;
            construct(start.cur, value);
        } else {
            //和push_back的实现相比更底层的写法，相对于把reserve_elements_at_back重写了一遍
            reserve_map_at_front();
            *(start.node - 1) = allocate_node();
            try {
                start.set_node(start.node - 1);
                start.cur = start.last - 1;
                construct(start.cur, value);
            } catch (...) {
                //注意push_front的catch部分和push_back的不同
                //因为错误只能发生在construct的过程种，而这个时候start已经set_node了，所以我们需要先回退一步
                start.set_node(start.node + 1);
                start.cur = start.first;
                deallocate_node(*(start.node - 1));
            }
        }
    }

    template<typename T>
    void deque<T>::push_back(const value_type &value) {
        if (finish.cur != finish.last - 1) {
            construct(finish.cur, value);
            ++finish;
        } else {
            //insert_aux(end(), 1, value);
            //直接用insert_aux或者
//            reserve_elements_at_back(1);
//            construct(finish.cur, value);
//            ++finish;
            //但其实上面的方法都会创建不必要的临时变量，最好的是把reserve_elements_at_back重写了一遍
            reserve_map_at_back();
            *(finish.node + 1) = allocate_node();
            try {
                construct(finish.cur, value);
                finish.set_node(finish.node + 1);
                finish.cur = finish.first;
            } catch (...) {
                deallocate_node(*(finish.node + 1));
            }
        }
    }

    template<typename T>
    void deque<T>::swap(deque &deq) {
        std::swap(map, deq.map);
        std::swap(map_size, deq.map_size);
        std::swap(start, deq.start);
        std::swap(finish, deq.finish);
    }

    template<typename T>
    deque<T> &deque<T>::operator=(const deque &rhs) {
        if (&rhs != this){
            const size_type len = size();
            if (len >= rhs.size()) {
                erase(std::copy(rhs.begin(), rhs.end(), start), finish);
            } else {
                const_iterator mid = rhs.begin() + difference_type(len);
                std::copy(rhs.begin(), mid, start);
                insert(finish, mid, rhs.end());
            }
        }
        return *this;
    }

    template<typename T>
    template<typename InputIterator>
    void deque<T>::copy_initialize(InputIterator first, InputIterator last) {
        create_map_nodes(0);
        for (; first != last; ++first)
            push_back(*first);
    }

    template<typename T>
    void deque<T>::destroy_nodes_at_back(deque::iterator after_finish) {
        for (map_pointer n = after_finish.node; n > finish.node; --n)
            deallocate_node(*n);
    }

    template<typename T>
    void deque<T>::destroy_nodes_at_front(deque::iterator before_start) {
        for (map_pointer n = before_start.node; n < start.node; ++n)
            deallocate_node(*n);
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::reserve_elements_at_back(deque::size_type n) {
        size_type remain = finish.last - finish.cur;
        if (n > remain) {
            size_type new_elements = n - remain;
            size_type new_nodes = (new_elements - 1) / buffer_size() + 1;
            reserve_map_at_back(new_nodes);
            size_type i;
            try {
                for (i = 1; i <= new_nodes; ++i) {
                    *(finish.node + 1) = allocate_node();
                }
            } catch (...) {
                for (size_type j = 1; j < i ; ++j) {
                    deallocate_node(*(finish.node + j));
                }
                throw;
            }
        }
        return finish + difference_type(n);
    }

    template<typename T>
    typename deque<T>::iterator deque<T>::reserve_elements_at_front(deque::size_type n) {
        //start缓冲区空余的位置
        size_type remain = start.cur - start.first;
        //如果需要创建的元素数目比空余的多，那么就需要分配多的map节点
        if (n > remain) {
            //new_elements是花掉start缓冲区空余的位置之后，需要新分配缓存区的数目
            size_type new_elements = n - remain;
            //减1是因为，如果多的元素数刚好一个缓冲区够放，那么只需要一个新的缓冲区
            size_type new_nodes = (new_elements - 1) / buffer_size() + 1;
            //start和finish指向新的start和finish,保证待分配的中控器节点足够
            reserve_map_at_front(new_nodes);
            //以上工作准备完毕，接下来就是分配缓冲区了
            size_type i;
            try {
                for (i = 1; i <= new_nodes; ++i) {
                    *(start.node - 1) = allocate_node();
                }
            } catch (...) {
                for (size_type j = 1; j < i ; ++j) {
                    deallocate_node(*(start.node - j));
                }
                throw;
            }
        }
        //分配完缓冲区后，把start向前移动n个元素
        return start - difference_type(n);
    }

    template<typename T>
    void deque<T>::insert_aux(deque::iterator pos, deque::size_type n, const value_type &value) {
        const difference_type elems_before = pos - start;
        size_type length = size();
        //如果pos之前的元素数目比较少，那么就从前面开始插入，否则从后面开始
        if (elems_before < length / 2) {
            n = difference_type(n);
            iterator new_start = reserve_elements_at_front(n);
            iterator old_start = start;
            pos = start + elems_before;
            try {
                //需要填充的数目n小于elems_before，所以我们不需要把pos之前的元素全部移到new_start去
                if (elems_before >= n){
                    iterator start_n = start + n;
                    //因为新分配的缓冲区是未构造的，所以要使用uninitialized_copy进行构造
                    uninitialized_copy(start, start_n, new_start);
                    //对于原有的已构造缓冲区，则只需要copy
                    start = new_start;
                    std::copy(start_n, pos, old_start);
                    //完成插入
                    std::fill(pos - n, pos, value);
                } else {
                    iterator mid = uninitialized_copy(start, pos, new_start);
                    uninitialized_fill(mid, start, value);
                    start = new_start;
                    uninitialized_fill(old_start, pos, value);
                }
            } catch (...) {
                //分配失败则要释放内存，析构已经在uninitialized函数中实现了
                destroy_nodes_at_front(new_start);
            }
        } else {
            iterator new_finish = reserve_elements_at_back(n);
            iterator old_finish = finish;
            n = difference_type(n);
            const difference_type elems_after =
                    difference_type(length) - elems_before;
            pos = finish - elems_after;
            try {
                if (elems_after > n) {
                    iterator finish_n = finish - n;
                    uninitialized_copy(finish_n, finish, finish);
                    finish = new_finish;
                    std::copy_backward(pos, finish_n, old_finish);
                    std::fill(pos, pos + n, value);
                } else {
                    uninitialized_fill(finish, pos + n, value);
                    uninitialized_copy(pos, finish, pos + n);
                    finish = new_finish;
                    std::fill(pos, old_finish, value);
                }
            } catch (...) {
                destroy_nodes_at_back(new_finish);
            }

        }
    }

    template<typename T>
    void deque<T>::reallocate_map(deque::size_type nodes_to_add, bool add_at_front) {
        size_type old_nodes_num = finish.node - start.node + 1;
        size_type new_nodes_num = old_nodes_num + nodes_to_add;
        map_pointer new_nstart;

        /* 如果旧map的size大于2倍实际需要的缓冲区节点数（即map有一半还没有用到） */
        if (map_size > 2 * new_nodes_num){
            //将当前map重新规划，使得已经构造好的node放在map的中间，使得两边可扩充的区域重新一致
            new_nstart = map + (map_size - new_nodes_num) / 2 +
                         (add_at_front ? nodes_to_add : 0);
            //如果新起点在原起点的前面，说明是不断push_back引发的扩容
            if (new_nstart < start.node)
                std::copy(start.node, finish.node + 1, new_nstart);
            else
                //否则是push_front引发的扩容
                std::copy_backward(start.node, finish.node + 1,
                                   new_nstart + new_nodes_num);
        } else {
            /*真的空间不够了，重新分配一块map内存，并把节点copy过去*/
            //如果自定义加的node空间比原空间少，那么就扩容到原来的两倍
            size_type new_map_size =
                    map_size + std::max(map_size, nodes_to_add) + 2;
            map_pointer new_map = map_alloc::allocate(new_map_size);
            new_nstart = new_map + (new_map_size - new_nodes_num) / 2 +
                         (add_at_front ? nodes_to_add : 0);

            //创建好新map之后就是把原来map的node拷贝过来, 然后释放map空间
            std::copy(start.node, finish.node + 1, new_nstart);
            map_alloc::deallocate(map,map_size);
            map = new_map;
            map_size = new_map_size;
        }
        //更新strat和finish
        //set_node保留了原来的cur信息。而且cur信息只会在迭代器+=和初分配create_map_nodes时操作
        start.set_node(new_nstart);
        finish.set_node(new_nstart + old_nodes_num - 1);
    }


    template<typename T>
    void deque<T>::fill_initialize(deque::size_type n, const value_type &value) {
        //allocate内存，创建map结构
        create_map_nodes(n);
        map_pointer cur;
        try {
            for (cur = start.node; cur < finish.node ; ++cur)
                uninitialized_fill(*cur, *cur + buffer_size(), value);
            uninitialized_fill(finish.first, finish.cur, value);
        } catch (...){
            //如果出现异常，cur当前的uninitialized_fill会处理该缓冲区的析构问题
            //所以我们需要做的是析构已经构造好的缓冲区，然后释放空间
            for (map_pointer n = start.node; n < cur; ++n)
                destroy(*n, *n + buffer_size());
            destroy_map_nodes();
        }
    }

    template<typename T>
    void deque<T>::destroy_map_nodes() {
        //因为分配的时候是一个缓冲区一个缓冲区分配的，所以摧毁的时候也是同理
        //先摧毁缓冲区，再摧毁map
        for (map_pointer temp = start.node; temp <= finish.node; ++temp) {
            deallocate_node(*temp);
        }
        map_alloc::deallocate(map, map_size);
    }

    template<typename T>
    void deque<T>::create_map_nodes(deque::size_type num_element) {
        //节点的个数
        size_type num_nodes = num_element / buffer_size() + 1;
        map_size = std::max(init_map_size(), num_nodes + 2);
        map = map_alloc::allocate(map_size);

        //令 nstart 和 nfinish 指向map所拥有的全部节点的最中间，使得两端的可扩充区域一致
        map_pointer nstart = map + (map_size - num_nodes) / 2;
        map_pointer nfinish = nstart + num_nodes - 1;
        map_pointer cur;
        try {
            for (cur = nstart;  cur <= nfinish ; ++cur) {
                *cur = allocate_node();
            }
        } catch (...) {
            for (map_pointer tmp = nstart; tmp < cur; ++tmp)
                deallocate_node(*tmp);
            map_alloc::deallocate(map, map_size);
            throw;
        }
        //注意，deque的迭代器没有=操作符，必须使用set_node()函数进行设置
        //因为缓冲区也是新分配的，所以迭代器的cur则要专门设置
        start.set_node(nstart);
        finish.set_node(nfinish);
        start.cur = start.first;
        finish.cur = finish.first + (num_element % buffer_size());
    }
}


#endif //MYSTL_DEQUE_H
