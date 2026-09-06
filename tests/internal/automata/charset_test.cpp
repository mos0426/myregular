#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.unite_update(32, 64);
    // cs 现在包含的区间为 [24, 64)
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));

    cs.unite_update(CODEPOINT_MIN, 48);
    cs.unite_update(72, CODEPOINT_MAX);
    // cs 现在包含的区间为 [CODEPOINT_MIN, 64), [72, CODEPOINT_MAX)
    ASSERT_TRUE(cs.contains(84));
    ASSERT_TRUE(cs.contains(28));
    ASSERT_FALSE(cs.contains(68));

    cs.negation_update();
    // cs 现在包含的区间为 [64, 72)
    ASSERT_FALSE(cs.contains(84));
    ASSERT_FALSE(cs.contains(28));
    ASSERT_TRUE(cs.contains(68));
    
    // 测试 unite_update(const CharSet &other)
    // 相离，相交，被包含，多个区间同时被包含情况
    CharSet cs2(24, 36);
    cs2.unite_update(60, 66);
    cs2.unite_update(68, 70);
    cs2.unite_update(71, 72);
    // cs2 现在包含的区间为 [24, 36), [60, 66), [68, 70), [71, 72)
    cs.unite_update(cs2);
    // cs 现在包含的区间为 [24, 36), [60, 72)
    ASSERT_TRUE(cs.contains(28));
    ASSERT_TRUE(cs.contains(68));
    ASSERT_FALSE(cs.contains(84));

    cs2.negation_update();
    // cs2 现在包含的区间为 [CODEPOINT_MIN, 24), [36, 60), [66, 68), [70, 71), [72, CODEPOINT_MAX)
    cs.unite_update(cs2);
    // cs 现在包含的区间为 [CODEPOINT_MIN, 72)
    ASSERT_TRUE(cs.contains(28));
    ASSERT_TRUE(cs.contains(68));
    ASSERT_TRUE(cs.contains(84));
    
    cs.unite_wildcard_update();
    ASSERT_TRUE(cs.contains(68));
};
