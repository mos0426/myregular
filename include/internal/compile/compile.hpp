#include "nfa.hpp"
#include "parse.hpp"

// 参考 Thompson's construction
NFA compile_to_nfa(const AST &ast);