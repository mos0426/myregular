#include <gtest/gtest.h>

#include "interval_cursor.hpp"


TEST(IntervalCursorTest, base_test){
    IntervalSet is1;
    is1.emplace_back(Interval{1, 4});
    is1.emplace_back(Interval{5, 8});
    is1.emplace_back(Interval{10, 11});
    IntervalCursor ic1 = IntervalCursor(is1, 1);
    ASSERT_EQ(ic1.current().codepoint, 1);
    ASSERT_EQ(ic1.current().is_start, true);

    ic1.next(); // current.codepoint = 4
    ASSERT_EQ(ic1.current().codepoint, 4);
    ASSERT_EQ(ic1.current().is_start, false);
    ic1.next(); // current.codepoint = 5
    ASSERT_EQ(ic1.current().codepoint, 5);
    ASSERT_EQ(ic1.current().is_start, true);
    ic1.next(); // current.codepoint = 8

    IntervalSet is2;
    is2.emplace_back(Interval{8, 9});
    IntervalCursor ic2 = IntervalCursor(is2, 2);
    ASSERT_TRUE(ic1 < ic2);
    ASSERT_FALSE(ic2 < ic1);
    ic1.next(); // current.codepoint = 10
    ASSERT_TRUE(ic2 < ic1);
    ASSERT_FALSE(ic1 < ic2);
}