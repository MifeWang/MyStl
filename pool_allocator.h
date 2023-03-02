
#ifndef MYSTL_POOL_ALLOCATOR_H
#define MYSTL_POOL_ALLOCATOR_H

#include "move.h"

namespace MyStl{
    //按照现在stl说法，当size > _S_max_bytes时，也应该直接使用new进行创建，也就是new_allocator
    //但是我们还是从练习角度出发，设计SIG中的二级分配器。
    //一级分配器是基于malloc()、free()、realloc()来实现allocate()、deallocate()、reallocate()等
    //二级分配器即使采用内存池技术的

    //1、在SIG的实现中，两个分配器都是类模板；但我们这里直接设计为两个普通类，原因是，在不考虑多线程的情况下，模板参数实际上是用不到的
    //2、此外，SIG中在包装两个分配器时使用了带两个参数的模板类simple_alloc，使用时还比较麻烦，
    //本项目中，直接采用pool_allocator的思想，使用一个参数的模板类，并且直接调用二级分配器，并且为该模板按照new_allocator补充标准库接口
    //命名为pool_alloc;


    //一级分配器直接使用malloc()分配内存，命名为malloc_alloc
    class malloc_alloc {
    private:
        // 一个函数指针类型
        using FunPtr = void (*)();

    public:
        //分配空间使用void指针，在最后包装的模板类中再进行转换。
        static void* allocate(size_t);
        static void deallocate(void* ptr) { free(ptr);}
        static void* reallocate(void*, size_t , size_t new_sz);
        static FunPtr set_malloc_handler(FunPtr f);

    private:
        // 用于处理OOM时的辅助函数
        static void* oom_malloc(size_t);
        static void* oom_realloc(void*, size_t);
        // 函数指针类型的静态成员。
        static FunPtr malloc_alloc_oom_handler;
    };

    //使用malloc分配内存
    void *malloc_alloc::allocate(size_t n) {
        void *result = malloc(n);
        if (result == nullptr) {
            result = malloc_alloc::oom_malloc(n);
        }
        return result;
    }

    void *malloc_alloc::reallocate(void *ptr, size_t old_sz, size_t new_sz) {
        void *result = realloc(ptr, new_sz);
        if (result == nullptr){
            result = malloc_alloc::oom_realloc(ptr, new_sz);
        }
        return result;
    }

    //handler机制的实现
    //静态成员初始化
    malloc_alloc::FunPtr malloc_alloc::malloc_alloc_oom_handler = nullptr;

    //set_malloc_handler其实就是给代表handler的静态成员赋值，然后返回原来的handler函数
    typename malloc_alloc::FunPtr malloc_alloc::set_malloc_handler(FunPtr f) {
        FunPtr old = malloc_alloc_oom_handler;
        malloc_alloc_oom_handler = f;
        return old;
    }

    //OOM的处理
    void *malloc_alloc::oom_malloc(size_t n) {
        if (malloc_alloc_oom_handler == nullptr){
            std::cerr << "Out Of Memory" << std::endl;
            //exit(1)表示程序异常退出
            exit(1);
        }
        //如果handler机制存在，那么不断尝试调用handler函数，使用malloc分配内存，直到分配成功
        void *result;
        for (;;){
            malloc_alloc_oom_handler();
            result = malloc(n);
            if (result)
                return result;
        }
    }

    void* malloc_alloc::oom_realloc(void* ptr, size_t new_sz) {
        if (malloc_alloc_oom_handler == nullptr) {
            std::cerr << "out of memory" << std::endl;
            exit(1);
        }
        //同oom_malloc函数，不断尝试
        void* result;
        for (;;) {
            malloc_alloc_oom_handler();
            result = realloc(ptr, new_sz);
            if (result)
                return result;
        }
    }


    //可以看到一级分配器非常简单，除去oom处理的部分，分配和释放都是直接调用了malloc和free函数
    //一级分配器不做使用，仅为练习
    //后续默认使用二级分配器，所以二级分配器命名为default_alloc
    //这部分初学还是有点复杂的，主要参考了STL中的__pool_alloc_base和__pool_alloc
    class default_alloc{
    private:
        enum { align = 8 };   //内存池区块的调整边界
        enum { max_bytes = 128 };   //内存池区块的上限
        enum { free_list_size = (size_t) max_bytes / (size_t) align  }; // free_list节点的个数

        union obj {
            union obj* next_free_list_link;    //指向下一个内存块
            char client_data[1];               //本块内存首地址，是客户识别这块内存的标志信息
        };

        static obj* volatile free_list[free_list_size]; //指针数组，指向16个空闲队列列表的起始位置
        static char*         start_free;                //未归空闲队列的堆内存起始地址
        static char*         end_free;                  //未归空闲队列的堆内存末端地址
        static size_t        heap_size;                 //申请的堆内存大小
    private:
        //将想要获取的内存向上取整到8
        static size_t round_up(size_t bytes) {
            return ((bytes + (size_t)align - 1) & ~((size_t)align - 1));    //向上取整的一种方法
        }
        //根据bytes的大小，决定使用free_list的几号区块
        static obj* volatile* get_free_list(size_t bytes) {
            size_t i = (bytes + (size_t)align - 1) / (size_t)align - 1;
            return free_list + i;
        };

        //allocate()中调用，返回大小为size的空间地址,并可能将多个大小为size的其它区块填充到free_list中
        //n需为8的倍数
        static void* refill(size_t n);

        //refill()中调用，尝试申请n_nodes个n字节大小的内存块。如果空间不够，n_nodes可能会降低
        static char* chunk_alloc(size_t size, int& n_nodes);

    public:
        static void* allocate(size_t n);
        static void deallocate(void* ptr, size_t n);
        static void* reallocate(void* ptr, size_t old_sz, size_t new_sz);
    };

    //静态成员初始化
    char* default_alloc::start_free = nullptr;
    char* default_alloc::end_free = nullptr;
    size_t default_alloc::heap_size = 0;
    default_alloc::obj* volatile default_alloc::free_list[free_list_size] = {
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    };

    void *default_alloc::allocate(size_t n) {
        if (n > max_bytes) return malloc_alloc::allocate(n);
        //小于128，使用内存池。找到相应链表，注意这是一个指向链表头指针的指针
        obj* volatile* my_free_list = get_free_list(n);
        obj* result = *my_free_list;
        //如果链表为空，就调用 refill 填充链表，refill会直接返回一个相应大小的空间供用户使用
        if (result == nullptr){
            //需要先将n上调至8的倍数，然后填充free_list
            return refill(round_up(n));
        }
        //如果列表不空，那么使表头变为下一个节点(删除了取走的那个区块)，然后使用强制类型转换将当前节点转换为void*，即所分配的内存位置，然后返回
        *my_free_list = (*my_free_list)->next_free_list_link;
        return static_cast<void*>(result);
    }

    void default_alloc::deallocate(void *ptr, size_t n) {
        if (n > max_bytes){
            malloc_alloc::deallocate(ptr);
            return;
        }
        //对内存池的释放方法：回收对应的链表
        obj* p = static_cast<obj*>(ptr);
        //找到所要回收对应的链表
        obj* volatile* my_free_list = get_free_list(n);
        //注意：这里其实并没有释放内存，只是把p指向的这块内存重新放到内存池对应的链表中，重复使用
        p->next_free_list_link = *my_free_list;
        *my_free_list = p;
    }

    void *default_alloc::reallocate(void *ptr, size_t old_sz, size_t new_sz) {
        //如果新旧size都大于内存池最大容量，使用malloc_alloc的realloc
        if (old_sz > max_bytes && new_sz > max_bytes){
            return malloc_alloc::reallocate(ptr,old_sz,new_sz);
        }
        //同在一块内存池的小区间，则无需调整
        if (round_up(old_sz) == round_up(new_sz)){
            return ptr;
        }
        // 都不是的话，需要模拟一下ralloc的操作
        // 开辟新内存，复制原来的部分，最后释放原内存
        void *result = allocate(new_sz);
        size_t copy_sz = new_sz > old_sz ? new_sz : old_sz;
        memcpy(result, ptr, copy_sz);
        deallocate(ptr, old_sz);
        return result;
    }

    char *default_alloc::chunk_alloc(size_t size, int &n_nodes) {
        //total_bytes是总申请的内存大小，bytes_left是剩余的内存空间
        char* result;
        size_t total_bytes = size * n_nodes;
        size_t bytes_left = end_free - start_free;

        //情况1，剩余内存足够，直接划分，start_free后移
        if (bytes_left > total_bytes){
            result = start_free;
            start_free += total_bytes;
            return result;
        } else if (bytes_left >= size){
            //情况2，剩余内存不够total_bytes，但是还有结余能分一块，此时能返回多少返回多少，先用着
            n_nodes = (int) (bytes_left / size);
            total_bytes = size * n_nodes;
            result = start_free;
            start_free += total_bytes;
            return result;
        } else{
            //情况3，一块也不够分配
            //将剩下的内存全部归入其它列表，因为申请的都是8的倍数的内存，所以必定可以全部归入
            if (bytes_left > 0){
                obj* volatile* my_free_list = get_free_list(bytes_left);
                //将剩余内存放到对应内存列表的头
                ((obj*)start_free)->next_free_list_link = *my_free_list;
                *my_free_list = (obj*)start_free;
            }

            //否则使用malloc请求（当前需求*2+历史需求/16）的大小
            size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);
            //这里也可以使用operator new，不过这时需要使用try catch机制去处理异常，
            //如果是使用malloc的话，分配失败会返回NULL
            start_free = (char *)malloc(bytes_to_get);
            if (start_free == nullptr){
                //malloc失败，说明系统内存空间不足；转而向上一级链表寻求空闲空间
                //提前分配变量，避免多次创建
                obj* volatile* my_free_list;
                obj* ptr = *my_free_list;
                for (size_t i = size; i <= max_bytes ; i += align) {
                    // 向上逐级找，找到有空闲内存块的链表，找到后，该内存块作为新的大块内存，
                    my_free_list = get_free_list(i);
                    ptr = *my_free_list;
                    //该链表有剩余空间
                    if (ptr != nullptr){
                        *my_free_list = ptr->next_free_list_link;
                        start_free = (char *)ptr;
                        end_free = start_free + i;
                        return chunk_alloc(size, n_nodes);
                    }
                }
                //如果更大的链表也没有空间了，使用低一级分配器的new_handler机制
                end_free = nullptr;
                start_free = (char *)malloc_alloc::allocate(bytes_to_get);
                //如果没有new_handler函数，则会直接报错“Out Of memory“然后结束程序。
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            //如果malloc成功，则进行递归
            return chunk_alloc(size, n_nodes);
        }
    }

    void *default_alloc::refill(size_t n) {
        //申请的小内存链表的节点数量
        int n_node = 20;
        char *chunk = chunk_alloc(n, n_node);
        //如果只传回了一个节点大小的空间，那么直接返回；这里n_node对应的形参是引用
        if (n_node == 1){
            return static_cast<void*>(chunk);
        }
        //如果有多余的，那么需要把剩下的节点添加到列表中
        obj* volatile* my_free_list;
        obj*           current_obj;
        obj*           next_obj;

        //先找到对应内存大小在内存池链表中的链表头
        my_free_list = get_free_list(n);

        //返回第一块内存
        void* result = static_cast<void*>(chunk);
        //空闲链表指向第二块内存
        *my_free_list = next_obj = (obj*)(chunk + n);
        //循环构建内存链表，每个节点指向一块n大小的内存块，最后一个节点指向null
        for (int i = 1; ;++i) {
            current_obj = next_obj;
            next_obj = (obj*)((char *)next_obj + n);
            if (n_node - 1 == i){
                current_obj->next_free_list_link = nullptr;
                break;
            } else
            current_obj->next_free_list_link = next_obj;
        }
        return result;
    }



    template<typename T>
    class pool_alloc{
    public:
        //STL的类别别名
        using value_type        = T;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using const_reference   = const T&;
        using size_type         = size_t;
        using difference_type   = ptrdiff_t;

    public:
        //对外公共接口参考new_allocator.h
        static pointer allocate() {
            return static_cast<pointer>(default_alloc::allocate(sizeof(value_type)));
        }

        static pointer allocate(size_type n) {
            return n == 0 ? 0 : static_cast<pointer>(default_alloc::allocate(n * sizeof(value_type)));
        }

        // 负责释放内存
        static void deallocate(pointer ptr) {
            if (ptr)
                default_alloc::deallocate((void *)ptr, sizeof(value_type));
        }

        static void deallocate(pointer ptr, size_type n) {
            if (n != 0)
                default_alloc::deallocate((void *)ptr, n * sizeof(value_type));
        }

        // 负责构造对象
        template<typename Up, typename... Args>
        inline void construct(Up* p, Args&&... args) noexcept {
            new((void *)p) Up(forward<Args>(args)...);
        }

        // 负责析构对象
        template<typename UP>
        static inline void destroy(UP* ptr) {
            ptr->~UP();
        }

        // 获取某对象的地址
        static pointer
        address(reference x) {
            return  pointer(&x);
        }
        static const_pointer
        address(const_reference x) {
            return  pointer(&x);
        }
        // 获取可配置T类型对象的最大数目
        static size_type max_size() {
            return size_type(-1) / sizeof (value_type);
        }

        //使T类型的allocator可以为T1类型的对象分配内存
        template <typename T1>
        struct rebind {
            using other = pool_alloc<T1>;
        };
    };
}

#endif //MYSTL_POOL_ALLOCATOR_H
