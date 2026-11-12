#include <gtest/gtest.h>

#include <vector>

#include "subset_interval_merger_cursor.hpp"
#include "interval.hpp"
#include "subset_interval_cursor.hpp"
#include "cursor_merger.hpp"


TEST(SubsetIntervalMergerCursorTest, base_test){
    SubsetIntervalSet i1 = {{1, 4, {1, 2, 3}}, {7, 10, {1, 2, 3}}};
    SubsetIntervalSet i2 = {{1, 6, {4, 5}}, {8, 12, {4, 5}}};
    SubsetIntervalSet i3 = {{4, 7, {1, 2, 3}}, {8, 10, {1, 2, 3}}};
    auto c1 = SubsetIntervalCursor(i1, 1);
    auto c2 = SubsetIntervalCursor(i2, 2);
    auto c3 = SubsetIntervalCursor(i3, 3);
    std::vector<SubsetIntervalCursor> cursors = {c1, c2, c3};
    auto merger = SubsetIntervalCursorMerger(cursors);
    auto merger_cursor = SubsetIntervalMergerCursor(merger);
    SubsetIntervalSet new_interval_set;
    while (merger_cursor.more()){
        new_interval_set.push_back(merger_cursor.next());
    }
    SubsetIntervalSet expected_interval_set = {
        {1, 6, {1, 2, 3, 4, 5}},
        {6, 8, {1, 2, 3}},
        {8, 10, {1, 2, 3, 4, 5}},
        {10, 12, {4, 5}}
    };
    ASSERT_EQ(new_interval_set, expected_interval_set);
}
