
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
        void fill_initialize(size_type n, const value_type& value);
        template <typename InputIterator>
        void copy_initialize(InputIterator first, InputIterator last);


    };

    template<typename T>
    void deque<T>::reallocate_map(deque::size_type nodes_to_add, bool add_at_front) {
        size_type old_nodes_num = finish.node - start.node + 1;
        size_type new_nodes_num = old_nodes_num + nodes_to_add;
        map_pointer new_nstart;

        /* 如果旧map的大于2倍实际需要的缓冲区节点数（即map有一半还没有用到） */
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
            deallocate_node(temp);
        }
        map_alloc::deallocate(map, map_size);
    }

    template<typename T>
    void deque<T>::create_map_nodes(deque::size_type num_element) {
        //节点的个数
        size_type num_nodes = num_element / buffer_size() + 1;
        map_size = std::max(init_map_size(), num_nodes + 2);
        map = malloc_alloc::allocate(map_size);

        //令 nstart 和 nfinish 指向map所拥有的全部节点的最中间，使得两端的可扩充区域一致
        map_pointer nstart = map + (map_size - num_nodes) / 2;
        map_pointer nfinish = nstart + num_nodes - 1;
        map_pointer cur;
        try {
            for (cur = nstart;  cur != nfinish ; ++cur) {
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
