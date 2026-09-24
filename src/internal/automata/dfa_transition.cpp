#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>


#include "dfa_transition.hpp"

size_t DFATransition::next_state(uint32_t codepoint) const{
    for (auto interval: intervals_){
        if (codepoint >= interval.start && codepoint < interval.end){
            return interval.target_state;
        }
    }
    return 0; // 转移到"死"状态
};


void DFATransition::add_interval(uint32_t start, uint32_t end, size_t target_state){
    assert(start < end);
    assert(target_state != 0);

    if (intervals_.empty()){
        intervals_.push_back({start, end, target_state});
        return;
    }

    auto revese_it = intervals_.rbegin();
    while (true){
        if (revese_it == intervals_.rend()){
            intervals_.insert(intervals_.begin(), {start, end, target_state});
            return;
        }

        // 两个码点区间在不同目标状态的情况允许终点和起点重叠
        if (revese_it->end <= start){
            if (revese_it->end == start && revese_it->target_state == target_state){
                // 两个码点区间在不同目标状态的情况允许终点和起点重叠
                revese_it->end = end;
                return;
            }
            intervals_.insert(revese_it.base(), {start, end, target_state});
            return;
        }
        
        if (revese_it->start < end){
            // 处理区间重叠的情况, 将重叠的区间合并为一个区间，并将目标状态设置为相同的目标状态
            DFAInterval merged_interval;
            merged_interval.end = std::max(revese_it->end, end);
            merged_interval.target_state = target_state;
            // 记录重叠的区间个数
            int counter = 1; 
            ++revese_it;
            while (true){
                if (revese_it == intervals_.rend()) break;
                if (revese_it->start >= end) break;
                ++counter;
                ++revese_it;
            }
            --revese_it;
            merged_interval.start = std::min(revese_it->start, start);
            intervals_.erase(revese_it.base() - 1, revese_it.base() - 1 + counter);
            intervals_.push_back(merged_interval);
            return ;
        }
        ++revese_it;
    }
}

