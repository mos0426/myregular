#pragma once

#include "nfa.hpp"
#include "parse.hpp"
#include "dfa.hpp"

// 参考 Thompson's construction
NFA compile_to_nfa(const AST &ast);

// 将 NFA 转换为 DFA, 并可选择是否进行最小化
// 参考 Subset construction
DFA nfa_to_dfa(const NFA &nfa, bool minimize=true);