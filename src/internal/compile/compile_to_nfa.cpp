#include <vector>
#include <variant>
#include <cassert>
#include <functional>
#include <memory>

#include "compile.hpp"
#include "parse.hpp"

// "{n, m}" 中最大重复次数，即 "m" 的最大值
constexpr int MAX_REPETITION = 1000;


namespace{
    template<typename... Ts>
    struct overloaded : Ts... {
    using Ts::operator()...;
    };

    size_t compile_(const AST &ast, NFA &nfa, size_t input_state=0);

    size_t compile_concatenation(const ConcatenationNode &node, NFA &nfa, size_t input_state){
         return compile_(
            *node.right.get(), nfa,
            compile_(*node.left.get(), nfa, input_state)
         );
    };

    size_t compile_plus(const PlusNode &node, NFA &nfa, size_t input_state){
        size_t left_input_state = nfa.new_state();
        size_t left_output_state = compile_(*node.left.get(), nfa, left_input_state);
        size_t output_state = nfa.new_state();

        nfa.add_epsilon_transition(input_state, left_input_state);
        nfa.add_epsilon_transition(left_output_state, left_input_state);
        nfa.add_epsilon_transition(left_output_state, output_state);

        return output_state;
    };

    size_t compile_star(const StarNode &node, NFA &nfa, size_t input_state){
        size_t left_input_state = nfa.new_state();
        size_t left_output_state = compile_(*node.left.get(), nfa, left_input_state);
        size_t output_state = nfa.new_state();

        nfa.add_epsilon_transition(input_state, left_input_state);
        nfa.add_epsilon_transition(input_state, output_state);
        nfa.add_epsilon_transition(left_output_state, output_state);
        nfa.add_epsilon_transition(left_output_state, left_input_state);
        
        return output_state;
    };

    size_t compile_union(const UnionNode &node, NFA &nfa, size_t input_state){
        size_t output_state = nfa.new_state();

        std::vector<std::reference_wrapper<AST>> union_opereans;
        // 展平连续的 union 节点
        auto flatten = [&union_opereans](auto &&self, const UnionNode &node) -> void {
            if (std::holds_alternative<UnionNode>(node.left->data)){
                self(self, std::get<UnionNode>(node.left->data)); 
            }
            else union_opereans.push_back(*node.left);

            if (std::holds_alternative<UnionNode>(node.right->data)){
                self(self, std::get<UnionNode>(node.right->data));
            }
            else union_opereans.push_back(*node.right);
        };

        flatten(flatten, node);
        
        for (auto operean: union_opereans){
            size_t operean_input_state = nfa.new_state();
            size_t operean_output_state =  compile_(operean.get(), nfa, operean_input_state);
            nfa.add_epsilon_transition(input_state, operean_input_state);
            nfa.add_epsilon_transition(operean_output_state, output_state);
        }

        return output_state;
    };

    size_t compile_repetition(const RepetitionNode &node, NFA &nfa, size_t input_state){
        
        size_t repetition_item_input = input_state;
        size_t repetition_item_output = input_state;
        size_t output_state = nfa.new_state();

        // 处理重复次数下限 min
        if (node.min == 0) {
            nfa.add_epsilon_transition(input_state, output_state);
        }
        else{
            for (auto i = 0; i < node.min; i++){
                repetition_item_input = repetition_item_output;
                repetition_item_output = compile_(*node.left.get(), nfa, repetition_item_input);
            }
        }


        // 处理重复次数上限 max
        if (node.max == INFINITE_REPEAT){
            nfa.add_epsilon_transition(repetition_item_output, repetition_item_input);
        }
        else {
            for (auto i = node.min; i < node.max; i++){
                repetition_item_input =repetition_item_output;
                repetition_item_output = compile_(*node.left.get(), nfa, repetition_item_input);
                nfa.add_epsilon_transition(repetition_item_output, output_state);
            }
        }

        nfa.add_epsilon_transition(repetition_item_output, output_state);

        return output_state;
    };

    size_t compile_charclass(const CharClassNode &node, NFA &nfa, size_t input_state){
        size_t output_state = nfa.new_state();
       
        CharSet charset = CharSet();
        for (auto range: node.ranges){
            charset.unite_update(range.start, range.end);
        }
        if (node.negated) charset.negation_update();
        nfa.add_transition(charset, input_state, output_state);

        return output_state;
    };

    size_t compile_char(const CharNode &node, NFA &nfa, size_t input_state){
        size_t output_state = nfa.new_state();
        nfa.add_transition(node.codepoint, input_state, output_state);
        return output_state;
    };

    size_t compile_(const AST &ast, NFA &nfa, size_t input_state){
       
        return std::visit(overloaded{
            [&nfa, input_state](const UnionNode &node) { return compile_union(node, nfa, input_state); },
            [&nfa, input_state](const PlusNode &node) { return compile_plus(node, nfa, input_state); },
            [&nfa, input_state](const ConcatenationNode &node) { return compile_concatenation(node, nfa, input_state); }, 
            [&nfa, input_state](const StarNode &node) { return compile_star(node, nfa, input_state); },
            [&nfa, input_state](const RepetitionNode &node) { return compile_repetition(node, nfa, input_state);},
            [&nfa, input_state](const CharNode &node) { return compile_char(node, nfa, input_state);},
            [&nfa, input_state](const CharClassNode &node) { return compile_charclass(node, nfa, input_state); }
        }, ast.data);
    };

}


NFA compile_to_nfa(const AST &ast){
    // 参考 Thompson's construction
    NFA nfa;
    size_t final_state = compile_(ast, nfa);
    nfa.set_final_state_set({final_state});
    nfa.reset();
    return nfa;
}
