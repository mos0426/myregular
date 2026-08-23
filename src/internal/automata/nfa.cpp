#include <cstdint>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>

#include "charset.hpp"
#include "nfa.hpp"


namespace{
    inline void sort_and_unique(std::vector<size_t> &state_set){
        std::sort(state_set.begin(), state_set.end());
        auto last = std::unique(state_set.begin(), state_set.end());
        state_set.erase(last, state_set.end());
    }
}


std::vector<size_t> NFA::move(const std::vector<size_t> &state_set, uint32_t codepoint) const{
    //根据给出的状态和输入码点，计算 NFA 的下一状态集合
    //state_set 必须为正向排列无重复元素的序列
    std::vector<size_t> result;
        
    for (size_t state: state_set){
        const std::vector<NFATransition> &transitions = transition_table_[state];
        for (auto it = transitions.begin(); it != transitions.end(); it++){
            if (it->char_set.contains(codepoint)) result.push_back(it->target_state);
        }
    }
    sort_and_unique(result);
     
    // epsilon_closure 的输入状态集
    std::vector<size_t> source;
    std::set_union(state_set.begin(), state_set.end(), result.begin(), result.end(), std::back_inserter(source));
    std::vector<size_t> closure(epsilon_closure(source));
    
    // 整合 result 和 closure
    std::vector<size_t> temp;
    std::set_union(result.begin(), result.end(), closure.begin(), closure.end(), std::back_inserter(temp));
    result.swap(temp);

    return result;
};


std::vector<size_t> NFA::epsilon_closure(const std::vector<size_t> &state_set) const{
    // 计算给定状态集合的 epsilon 闭包
    //state_set 必须为正向排列无重复元素的序列

    std::vector<size_t> result;
    std::vector<size_t> visited;
    std::vector<size_t> pending(state_set);

    while (true){
        std::vector<size_t> current_targets;
        for (size_t state: pending){
            const std::vector<NFATransition> &transitions = transition_table_[state];
            for (auto it = transitions.begin(); it != transitions.end(); it++){
                if (it->has_epsilon) current_targets.push_back(it->target_state);
            }
        }

        sort_and_unique(current_targets);
        std::vector<size_t> diff;
        std::set_difference(
            current_targets.begin(), current_targets.end(), visited.begin(), visited.end(), std::back_inserter(diff)
        );

        if (diff.empty()){
            // 结果排序去重并返回
            sort_and_unique(result);
            return result;
        } 
        else{
            // 更新 visited, result 和 pending
            std::vector<size_t> temp;
            std::set_union(
                visited.begin(), visited.end(), diff.begin(), diff.end(), std::back_inserter(temp)
            );
            visited.swap(temp);
            for (size_t item: diff) result.push_back(item);
            pending.swap(diff);
        }       
    }
}


bool NFA::check(){
    auto c_it = current_state_set_.begin(), f_it = final_state_set_.begin();

    while ((c_it != current_state_set_.end()) && (f_it != final_state_set_.end())){
        if (*c_it < *f_it) c_it++;
        else if (*c_it > *f_it) f_it++;
        else return true; // *c_it == *f_it
    }
    
    return false;
}


void NFA::reset(){
    std::vector<size_t> initial_epsilon_closure = epsilon_closure({0});
    initial_epsilon_closure.push_back(0);
    sort_and_unique(initial_epsilon_closure);
    current_state_set_ = initial_epsilon_closure;
}


void NFA::add_transition(uint32_t start, uint32_t end, size_t state, size_t target_state, bool is_negated){
    // 增加一个转移(transition), 要求 state 和 target_state 都是 this 已存在的状态

    std::vector<NFATransition> &transitions = transition_table_[state];
    for (auto i = transitions.begin(); i != transitions.end(); i++){
        if (i->target_state == target_state){
            i->char_set.add(start, end, is_negated);
            return;
        }
    }
    transitions.emplace_back(NFATransition{target_state, CharSet(start, end)});
}


void NFA::add_wildcard_transition(size_t state, size_t target_state){
    
    std::vector<NFATransition> &transitions = transition_table_[state];
    for (auto i = transitions.begin(); i!= transitions.end(); i++){
        if (i->target_state == target_state){
            i->char_set.add_wildcard();
            return;
        }
    }
    transitions.emplace_back(NFATransition{target_state, CharSet()});
    transitions.back().char_set.add_wildcard();
}


void NFA::add_epsilon_transition(size_t state, size_t target_state){
    // 增加一个空转移 (epsilon transition)

    std::vector<NFATransition> &transitions = transition_table_[state];
    for (auto i = transitions.begin(); i != transitions.end(); i++){
        if (i->target_state == target_state){
            i->has_epsilon = true;
            return;
        }
    }
    transitions.emplace_back(NFATransition{target_state, CharSet(), true});
}


void NFA::add_final_state(size_t final_state){
    final_state_set_.push_back(final_state);
    sort_and_unique(final_state_set_);
}


void NFA::set_final_state_set(std::vector<size_t> final_state_set){
    sort_and_unique(final_state_set);
    final_state_set_.swap(final_state_set);
}


