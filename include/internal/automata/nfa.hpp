#include <cstdint>
#include <set>
#include <vector>

#include "charset.hpp"


struct NFATransition
{
    size_t target_state; // 目标状态
    CharSet char_set; // 转移条件，使用 CharSet 表示字符集
    bool has_epsilon = false; // 是否存在空转移 
};


class NFA {
    // 非确定有穷状态机 (NFA)
    // 初始状态为 '0'
    // 此对象下出现的所有状态集（即所有带 state_set 后缀的变量）在所有操作均保证为正向排列无重复元素的序列

public:
    NFA(): current_state_set_({0}){transition_table_.emplace_back();}; 
    ~NFA() = default;

    //根据给出的状态集合和输入码点，计算 NFA 的下一状态集合
    std::vector<size_t> move(const std::vector<size_t> &stateset, uint32_t codepoint) const;

    // 计算给定状态集合的 epsilon 闭包
    std::vector<size_t> epsilon_closure(const std::vector<size_t> &stateset) const;

    // 消费一个码点
    void consume(uint32_t codepoint){
        std::vector<size_t> new_state_set = move(current_state_set_, codepoint);
        current_state_set_.swap(new_state_set);
    };

    // 增加一个转移(transition), 要求 state 和 target_state(或target_state_set) 都是 this 已存在的状态
    void add_transition(uint32_t codepoint, size_t state, size_t target_state);
    void add_transition(uint32_t codepoint, size_t state, std::vector<size_t> target_states){
        for (size_t target_state: target_states) add_transition(codepoint, state, target_state);
    };
    // 增加一个空转移 (epsilon transition)
    void add_transition(size_t state, size_t target_state);
    void add_transition(size_t state, std::vector<size_t> traget_states){
        for (size_t target_state: traget_states) add_transition(state, target_state);
    };

    size_t new_state(){
        std::size_t new_state = transition_table_.size();
        transition_table_.emplace_back();
        return new_state;
    };

    void add_final_state(size_t final_state);

    void set_final_state_set(std::vector<size_t> final_state_set){final_state_set_.swap(final_state_set);};

    std::vector<size_t> get_current_state_set() {return current_state_set_;};

    // 禁止拷贝构造和拷贝赋值
    NFA(const NFA&) = delete;
    NFA& operator=(const NFA&) = delete;

    // 允许移动构造和移动赋值
    NFA(NFA&&) noexcept = default;
    NFA& operator=(NFA&&) noexcept = default;

private:

    // 状态转移表，索引为当前状态，值为该状态的所有转移条件和目标状态
    // 每个状态中同一个目标状态的转移函数 (NFATransition) 只会出现一个;
    std::vector<std::vector<NFATransition>> transition_table_;
    std::vector<size_t> current_state_set_;
    std::vector<size_t> final_state_set_;
};
