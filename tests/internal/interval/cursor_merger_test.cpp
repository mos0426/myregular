#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include "cursor_merger.hpp"
#include "interval_cursor.hpp"
#include "interval.hpp"



TEST(IntervalCursorMergerTest, base_test){
    IntervalSet is1 = {{1, 5}, {7, 9}};
    IntervalSet is2 = {{2, 3}, {4, 7}, {8, 10}};
    IntervalSet is3 = {{2, 5}, {6, 9}};
    
    std::vector<IntervalCursor> cursors = {IntervalCursor(is2, 2)};
    cursors.emplace_back(IntervalCursor(is1, 1));
    cursors.emplace_back(IntervalCursor(is3, 3));

    IntervalCursorMerger merger(cursors);

    // merger.next() 按照{1, 2, 2, 3, 4, 5, 5, 6, 7, 7, 8, 9, 9, 10} 依次输出

    ASSERT_TRUE(merger.more());

    auto r1 = merger.next();
    ASSERT_EQ(r1.first.codepoint, 1);
    ASSERT_EQ(r1.first.is_start, true);
    ASSERT_EQ(r1.second, 1);

    auto r2 = merger.next();
    ASSERT_EQ(r2.first.codepoint, 2);
    ASSERT_EQ(r2.first.is_start, true);
    
    auto r3 = merger.next();
    ASSERT_EQ(r3.first.codepoint, 2);
    ASSERT_EQ(r3.first.is_start, true);  

    auto r4 = merger.next();
    ASSERT_EQ(r4.first.codepoint, 3);
    ASSERT_EQ(r4.first.is_start, false);
    ASSERT_EQ(r4.second, 2);

    auto r5 = merger.next();
    ASSERT_EQ(r5.first.codepoint, 4);
    ASSERT_EQ(r5.first.is_start, true);
    ASSERT_EQ(r5.second, 2);

    auto r6 = merger.next();
    ASSERT_EQ(r6.first.codepoint, 5);
    ASSERT_EQ(r6.first.is_start, false);

    auto r7 = merger.next();

    auto r8 = merger.next();
    ASSERT_EQ(r8.first.codepoint, 6);
    ASSERT_EQ(r8.first.is_start, true);
    ASSERT_EQ(r8.second, 3);

    auto r9 = merger.next();
    ASSERT_EQ(r9.first.codepoint, 7);
    ASSERT_EQ(r9.first.is_start, false);
    ASSERT_EQ(r9.second, 2);

    auto r10 = merger.next();
    ASSERT_EQ(r10.first.codepoint, 7);
    ASSERT_EQ(r10.first.is_start, true);
    ASSERT_EQ(r10.second, 1);

    auto r11 = merger.next();
    ASSERT_EQ(r11.first.codepoint, 8);
    ASSERT_EQ(r11.first.is_start, true);
    ASSERT_EQ(r11.second, 2);

    auto r12 = merger.next();
    ASSERT_EQ(r12.first.codepoint, 9);
    ASSERT_EQ(r12.first.is_start, false);

    auto r13 = merger.next();

    auto r14 = merger.next();
    ASSERT_EQ(r14.first.codepoint, 10);
    ASSERT_EQ(r14.first.is_start, false);
    ASSERT_EQ(r14.second, 2);

    ASSERT_FALSE(merger.more());
}