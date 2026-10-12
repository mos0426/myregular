#include <utility>
#include <cassert>

#include "interval_cursor_merger.hpp"


IntervalCursorMerger::IntervalCursorMerger(const std::vector<IntervalCursor> &cursors){
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


std::pair<Endpoint, size_t> IntervalCursorMerger::next(){
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
}


void IntervalCursorMerger::shift_up(size_t i){

    if (i == 0) return;
    // 父节点索引
    size_t p = (i - 1) / 2;

    if (cursors_[i] < cursors_[p]){
        std::swap(cursors_[i], cursors_[p]);
        cursors_[i] = cursors_[p];
        return shift_up(p);
    }
    return;
}


void IntervalCursorMerger::shift_down(size_t i){
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
}