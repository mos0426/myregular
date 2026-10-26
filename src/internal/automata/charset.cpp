#include <cstdint>
#include <vector>
#include <utility>
#include <cassert>

#include "charset.hpp"
#include "interval.hpp"
#include "interval_set_union.hpp"



bool CharSet::contains(uint32_t codepoint) const {
    // 检查给定的码点是否在字符集范围内

    auto it = interval_set_.begin();

    while (it != interval_set_.end()){
        if (codepoint < it->end){
            if (codepoint >= it->start) return true;
            else break;
        }
        ++it;
    }
    return false;
}


CharSet CharSet::unite(const CharSet &other) const {
    IntervalSet new_interval_set = interval_set_union(interval_set_, other.get_intervals());
    CharSet ans = CharSet();
    ans.interval_set_ = new_interval_set;
    return ans;
}


void CharSet::unite_update(uint32_t start, uint32_t end){
    // 增加一个码点区间
    // 针对连续正序添加区间的情况优化的接口，如 [[1, 3], [5, 8], [10, 20]....] 之类的

    assert(start < end);
    if (interval_set_.empty()){
        interval_set_.push_back({start, end});
        return;
    }

    Interval &last_interval = interval_set_.back();
    if (last_interval.end < start){
        interval_set_.push_back({start, end});
        return;
    }

    auto it = interval_set_.begin();
    while (it != interval_set_.end()){
        if (it->end < start){
            ++it;
            continue;
        }

        if (it->start > end){
            interval_set_.insert(it, {start, end});
            return;
        }

        // 合并区间
        it->start = std::min(it->start, start);
        it->end = std::max(it->end, end);
        return;
    }
}


void CharSet::negation_update(){
    // 取反字符集
    // 例如，字符集为 [1, 3), [5, 8) 时，取反后为:
    // [CODEPOINT_MIN, 1), [3, 5), [8, CODEPOINT_MAX)

    if (interval_set_.empty()){
        interval_set_.push_back({CODEPOINT_MIN, CODEPOINT_MAX});
        return;
    }

    std::vector<Interval> negated_intervals;
    auto it = interval_set_.begin();
    uint32_t pred_end;

    if (it->start != CODEPOINT_MIN) pred_end = CODEPOINT_MIN;
    else{
        pred_end = it->end;
        ++it;
    }

    while (it != interval_set_.end() - 1){
        negated_intervals.push_back({pred_end, it->start});
        pred_end = it->end;
        ++it;
    }
    negated_intervals.push_back({pred_end, it->start});
    if (it->end != CODEPOINT_MAX) negated_intervals.push_back({it->end, CODEPOINT_MAX});
    
    interval_set_.swap(negated_intervals);
}