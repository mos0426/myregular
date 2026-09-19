#pragma once

#include <vector>
#include <cstdint>

#include "dfa_transition.hpp"

class NFA;

class DFA{
    // 确定有穷状态机 (DFA)
    // 初始状态为 '1'
    // "死" 状态为 '0', 即不可转移到任意其他状态，且不为最终状态
public:
    DFA(): current_state_(1){transition_table_.emplace_back(), transition_table_.emplace_back();};
    ~DFA() = default;

    //根据给出的状态集合和输入码点，计算 DFA 的下一状态
    size_t move(size_t state, uint32_t codepoint){
        const DFATransition &dfa_transition = transition_table_[state];
        return dfa_transition.next_state(codepoint);
    };

    // 消耗一个码点
    void consume(uint32_t codepoint){
        current_state_ = move(current_state_, codepoint);
    };

    // 重置 DFA, current_state_ 重置回初始状态
    void reset(){current_state_ = 1;};

    // 检查目前状态是否到达最终状态
    bool check(){
        auto it = final_state_set_.begin();
        while (it != final_state_set_.end()){
            if (*it < current_state_) ++it;
            else if (*it > current_state_) return false;
            // *it == current_state_
            else return true;
        }
        return false;
    };

    friend DFA nfa_to_dfa(const NFA &nfa, bool minimize);

    // 禁止拷贝构造和拷贝赋值
    DFA(const DFA&) = delete;
    DFA& operator=(const DFA&) = delete;

    // 允许移动构造和移动赋值
    DFA(DFA&&) noexcept = default;
    DFA& operator=(DFA&&) noexcept = default;
private:
    // 状态转移表，索引为当前状态，值记录了该状态的所有转移条件和目标状态
    std::vector<DFATransition> transition_table_;
    size_t current_state_;
    std::vector<size_t> final_state_set_;

    size_t new_state(){
        transition_table_.emplace_back();
        return transition_table_.size()-1;
    };

    void add_final_state(size_t state){
        auto it = final_state_set_.begin();

        while (it != final_state_set_.end()){
            if (*it < state) ++it;
            else if (*it > state){
                final_state_set_.insert(it, state);
                return ;
            }
            else return ;
        }
        final_state_set_.push_back(state);
        return ;
    };
};