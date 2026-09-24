#pragma once

#include "nfa.hpp"
#include "parse.hpp"
#include "dfa.hpp"

// 将 ast 编译成 NFA
// 参考 Thompson's construction
NFA compile_to_nfa(const AST &ast);

// 将 NFA 转换为 DFA, 并可选择是否进行最小化
// 参考 Subset construction
DFA nfa_to_dfa(const NFA &nfa, bool minimize=true);

// 将 dfa 进行最下化， 最小化后的 dfa 在状态数和边数上都会达到最小
void dfa_minimization(DFA &dfa);