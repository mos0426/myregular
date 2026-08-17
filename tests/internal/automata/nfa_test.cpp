#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "nfa.hpp"

TEST(NFATest, Basic){
    NFA nfa;
    for (int i = 1; i <= 5; i++) nfa.new_state();
    nfa.add_transition('a', 0, {1, 4});
    nfa.add_transition('a', 1, 2);
    nfa.add_transition('a', 2, 3);
    nfa.add_transition('a', 4, 5);
    nfa.add_transition('a', 5, 4);
    nfa.set_final_state_set({3, 5});
    std::string a_str = "aaa";
    for (auto c: a_str){
        nfa.consume(c);
    };
    ASSERT_EQ(nfa.get_current_state_set(), std::vector<size_t>({3, 4}));
    ASSERT_TRUE(nfa.check());

    NFA nfa2;
    for (int i = 1; i <= 5; i++) nfa2.new_state();
    nfa2.add_transition('a', 0, {1, 4});
    nfa2.add_transition('a', 1, 2);
    nfa2.add_transition('a', 2, 3);
    nfa2.add_transition('a', 4, 5);
    nfa2.add_transition('a', 5, 4);
    nfa2.add_transition('a', 3, {1, 2, 3});
    nfa2.add_transition('a', 4, 1);
    nfa2.set_final_state_set({5, 2});
    ASSERT_EQ(nfa2.move({3}, 'a'), std::vector<size_t>({1, 2, 3}));
    ASSERT_EQ(nfa2.move({4}, 'a'), std::vector<size_t>({1, 5}));
    nfa2.consume('a');
    ASSERT_FALSE(nfa2.check());

    // 空转移 (epsilon transition) 测试
    NFA nfa3;
    for (int i = 1; i<= 3; i++) nfa3.new_state();
    nfa3.add_transition('a', 0, 1);
    nfa3.add_transition(1, {2, 3});
    nfa3.add_transition(2, {0});
    nfa3.add_transition(3, {1});
    nfa3.set_final_state_set({3});
    ASSERT_EQ(nfa3.epsilon_closure({1}), std::vector<size_t>({0, 1, 2, 3}));
    nfa3.consume('a');
    ASSERT_TRUE(nfa3.check());
}