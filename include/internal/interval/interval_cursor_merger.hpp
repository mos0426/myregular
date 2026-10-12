#pragma once

#include <interval_cursor.hpp>


class IntervalCursorMerger{
public:
    IntervalCursorMerger() = delete;

    IntervalCursorMerger(const std::vector<IntervalCursor> &cursors);
    
    // 返回下一个端点和标签；
    // 确保 more() 为 true 才可执行 next()
    // 若出现码点值相同的端点，优先输入端点属性为终点的端点(即 Endpoint.is_start == false)
    std::pair<Endpoint, size_t> next();

    std::pair<Endpoint, size_t> get_next() const {
        return buffer_;
    }

    bool more(){return has_more_;};
private:
    // cursors_ 为一个最小堆
    // cursors_ 的数据索引方式为完全二叉树 (Complete Binary Tree)
    std::vector<IntervalCursor> cursors_;
    // 缓存 next() 的结果
    // pair.second 为端点的标签 (label)
    std::pair<Endpoint, size_t> buffer_;
    bool has_more_;
    
    // 上浮 cursors_[i]， 直到 cursors_[i] 大于它的父节点 
    void shift_up(size_t i);

    // 下沉 cursors_[i], 直到 cursors_[i] 大于它的所有子节点
    void shift_down(size_t i);
};