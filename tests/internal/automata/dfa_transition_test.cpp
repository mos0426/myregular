#include <gtest/gtest.h>

#include "dfa_transition.hpp"


TEST(DFATransitionTest, BASIC){
    DFATransition dfa_transition;
    dfa_transition.add_interval(1, 3, 2);
    ASSERT_EQ(dfa_transition.next_state(1), 2);
    ASSERT_EQ(dfa_transition.next_state(4), 0);

    // 区间相离
    dfa_transition.add_interval(5, 8, 3);
    // 现在的区间为 [1, 3) → 2, [5, 8) → 3
    ASSERT_EQ(dfa_transition.next_state(5), 3);
    ASSERT_EQ(dfa_transition.next_state(4), 0);
    ASSERT_EQ(dfa_transition.next_state(8), 0);

    // 区间相交
    dfa_transition.add_interval(7, 9, 4);
    // 现在的区间为 [1, 3) → 2, [5, 9) → 4
    ASSERT_EQ(dfa_transition.next_state(6), 4);
    ASSERT_EQ(dfa_transition.next_state(8), 4);
    ASSERT_EQ(dfa_transition.next_state(9), 0);

    // 区间被包含
    dfa_transition.add_interval(11, 16, 5);
    dfa_transition.add_interval(20, 25, 6);
    dfa_transition.add_interval(30, 40, 7);
    // 现在的区间为 [1, 3) → 2, [5, 9) → 4, [11, 16) → 5, [20, 25) → 6, [30, 40) → 7
    dfa_transition.add_interval(1, 22,  8);
    // 现在的区间为 [1, 25) → 8, [30, 40) → 7
    ASSERT_EQ(dfa_transition.next_state(1), 8);
    ASSERT_EQ(dfa_transition.next_state(3), 8);
    ASSERT_EQ(dfa_transition.next_state(22), 8);
    ASSERT_EQ(dfa_transition.next_state(30), 7);
    ASSERT_EQ(dfa_transition.next_state(40), 0);

}