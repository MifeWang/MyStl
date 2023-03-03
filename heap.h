
#ifndef MYSTL_HEAP_H
#define MYSTL_HEAP_H

//heap.h不属于标准库的部分，为建堆和维护最大堆的相关算法，为设计优先队列打下基础
namespace MyStl {

    //向上调整法
    template <typename T, typename RandomAccessIterator, typename Distance>
    void push_heap_aux(RandomAccessIterator first,                  //容器起始位置
                       Distance holeIndex,                          //洞号，及子节点的位置
                       Distance topIndex,                           //父节点的位置
                       const T& value) {                            //子节点的值
        //找到父节点
        Distance parent = (holeIndex - 1) / 2;
        //如果还未到达顶端，并且父节点的值小于子节点，那么就交换
        while(holeIndex > topIndex && *(first + parent) < value) {
            *(first + holeIndex) = *(first + parent);
            holeIndex = parent;
            parent = (holeIndex - 1) / 2;
        }
        //子节点进无可进，说明已经满足了最大堆的条件，父节点大于子节点。
        *(first + holeIndex) = value;
    }

    //向下调整法
    template <typename T, typename RandomAccessIterator, typename Distance>
    void pop_heap_aux(RandomAccessIterator first,
                      Distance holeIndex,           //需要向下调整的节点
                      Distance len,                 //堆的长度
                      const T& value) {
        //找到右子节点
        Distance largeChild = holeIndex * 2 + 2;
        while (largeChild < len ){
            //找到左右子节点种最大的
            if (*(first + largeChild) < *(first + largeChild - 1))
                --largeChild;
            //比较父子，如果满足大堆，则退出。否则交换
            if (*(first + largeChild) < value)
                break;
            *(first + holeIndex) = *(first + largeChild);
            holeIndex = largeChild;                 //交换完毕, 大子节点成为新的洞值
            largeChild = holeIndex * 2 + 2;         //计算新的右子节点
        }
        //因为largeChile必为右节点，所以当 == len的时候，即为右子节点为空
        //这时候需要比较洞值和子节点
        if (largeChild == len) {
            if (value < *(first + largeChild - 1)){
                *(first + holeIndex) = *(first + largeChild - 1);
                holeIndex = largeChild - 1;
            }
        }
        //以上流程结束后，洞值下无可下，填充洞值
        *(first + holeIndex) = value;
    }

     /*heap算法接口*/
     //heap只是一种维护随机访问迭代器容器内部数据的一种方式，

     //push_heap， 使得容器尾元素加入最大堆
     //此函数被调用时，新元素已经插入底部容器尾部 last-1 位置，且 [first, last-1) 满足heap要求，否则结果不可预期
     template<typename RandomAccessIterator>
     void push_heap(RandomAccessIterator first,
                    RandomAccessIterator last) {
         auto value = *(last - 1);
         auto holeIndex = (last - first) - 1;
         decltype(holeIndex) topIndex = 0;
         push_heap_aux(first, holeIndex, topIndex, value);
     }

     //pop_heap 使top元素放到容器末尾，同时剩余数据满足最大堆结构
     //调用此函数时，[first, last) 内的序列应已满足堆要求，否则结果未可预期
     //它只从heap中移除，但不从底层容器中移除
     //具体操作是使用向下调整法，将容器首位元素互换，然后重新自上向下调整，使得重新满足最大堆
     template<typename RandomAccessIterator>
     void pop_heap(RandomAccessIterator first,
                    RandomAccessIterator last) {
         //这里的-1和push的-1是不一样的，后者是下标，这里是长度；这里-1的目的是将top元素从堆种去除
         auto len = (last - first) - 1;
         decltype(len) holeIndex = 0;
         //首位元素互换，top的位置为洞号
         auto value = *(last - 1);
         *(last - 1) = *first;
         pop_heap_aux(first, holeIndex, len, value);
     }

    //make_heap 使堆的底层容器满足堆的要求
    //其本质过程为：从最大的非叶节点开始，对每个非叶节点执行一次下溯程序.
    template<typename RandomAccessIterator>
    void make_heap(RandomAccessIterator first,
                  RandomAccessIterator last) {
        auto len = last - first;
        if (len < 2) return; //只有一个节点就不需要调整
        //找到第一个非叶子节点的下标。
        decltype(len) holeIndex = (len - 2) / 2;
        while (holeIndex >= 0) {
            //len指的是最大下标
            pop_heap_aux(first, holeIndex, len, *(first + holeIndex));
            --holeIndex;
        }
    }

    //sort_heap
    //不断pop_heap,直到heap为空，这样容器中的元素将按照从大到小排列
    template <typename RandomAccessIterator>
    void sort_heap(RandomAccessIterator first,
                   RandomAccessIterator last) {
        while (last - first > 1){
            pop_heap(first, last);
            --last;
        }
    }

}
#endif //MYSTL_HEAP_H
