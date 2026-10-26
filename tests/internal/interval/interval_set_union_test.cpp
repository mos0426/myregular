#include <gtest/gtest.h>

#include "interval_set_union.hpp"
#include "interval.hpp"


TEST(IntervalSetUnionTest, base_test){
    IntervalSet is1 = {{64, 75}, {78, 85}};
    IntervalSet is2 = {{24, 36}, {60, 66}, {68, 70}, {71, 72}, {74, 80}};

    auto new_is = interval_set_union(is1, is2);
    ASSERT_EQ(new_is[0].start, 24);
    ASSERT_EQ(new_is[0].end, 36);
    ASSERT_EQ(new_is[1].start, 60);
    ASSERT_EQ(new_is[1].end, 85);
}