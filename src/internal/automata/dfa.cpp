#include "dfa.hpp"
#include "interval.hpp"

size_t DFA::move(size_t state, uint32_t codepoint) const{
    auto &dfa_interval_set = transition_table_[state];
    for (auto interval: dfa_interval_set){
        if (codepoint >= interval.start && codepoint < interval.end){
            return interval.target_state;
        }
    }
    return 0; // 未匹配到任何码点区间，转移到 "死" 状态
};