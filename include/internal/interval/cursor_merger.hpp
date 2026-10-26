#pragma once

#include "interval_cursor.hpp"
#include "dfa_interval_cursor.hpp"

template<typename T>
class CursorMerger;

using IntervalCursorMerger = CursorMerger<IntervalCursor>;
using DFAIntervalCursorMerger = CursorMerger<DFAIntervalCursor>;


template<typename T>
class CursorMerger{
public:
    CursorMerger() = default;

    CursorMerger(const std::vector<T> &cursors){
        for (auto cursor: cursors){
            cursors_.push_back(cursor);
            shift_up(cursors_.size()-1);
        }
        if (cursors_.empty()) has_more_ = false;
        else{
            has_more_ = true;
            next();
        }
    };
    
    // 返回下一个端点和标签；
    // 确保 more() 为 true 才可执行 next()
    // 若出现码点值相同的端点，优先输入端点属性为终点的端点(即 Endpoint.is_start == false)
    std::pair<Endpoint, size_t> next(){
        assert(more());
        std::pair<Endpoint, size_t> result = std::move(buffer_);
        if (cursors_.empty()){
            has_more_ = false;
            return result;
        }
        buffer_ = std::make_pair(cursors_[0].current(), cursors_[0].label());
        if (cursors_[0].more()){
            cursors_[0].next();
            shift_down(0);
        }
        else{
            cursors_[0] = std::move(cursors_.back());
            cursors_.pop_back();
            if (!cursors_.empty()) shift_down(0);
        }
        return result;
    };

    std::pair<Endpoint, size_t> get_next() const {
        return buffer_;
    }

    bool more(){return has_more_;};
private:
    // cursors_ 为一个最小堆
    // cursors_ 的数据索引方式为完全二叉树 (Complete Binary Tree)
    std::vector<T> cursors_;
    // 缓存 next() 的结果
    // pair.second 为端点的标签 (label)
    std::pair<Endpoint, size_t> buffer_;
    bool has_more_;
    
    // 上浮 cursors_[i]， 直到 cursors_[i] 大于它的父节点 
    void shift_up(size_t i){

        if (i == 0) return;
        // 父节点索引
        size_t p = (i - 1) / 2;

        if (cursors_[i] < cursors_[p]){
            std::swap(cursors_[i], cursors_[p]);
            return shift_up(p);
        }
        return;
    };

    // 下沉 cursors_[i], 直到 cursors_[i] 大于它的所有子节点
    void shift_down(size_t i){
        if ((cursors_.size() - 1) <= (i*2)) return ;
        // 左子节点和右子节点的索引
        size_t l = i*2 + 1;
        // 最小子节点的索引
        size_t min_children;
        if ((cursors_.size() - 1) <= (i*2+1)){
            min_children = l;
        }
        else {
            size_t r = i*2 + 2;
            min_children = cursors_[l] < cursors_[r] ? l : r;
        }
        if (cursors_[min_children] < cursors_[i]){
            std::swap(cursors_[i], cursors_[min_children]);
            return shift_down(min_children);
        }
        return ;
    };
};
