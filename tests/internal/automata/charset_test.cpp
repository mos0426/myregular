#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.unite_update(32, 64);
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));

    cs.unite_update(48, 72, true);
    ASSERT_TRUE(cs.contains(84));
    ASSERT_TRUE(cs.contains(28));
    ASSERT_FALSE(cs.contains(68));

    cs.unite_wildcard_update();
    ASSERT_TRUE(cs.contains(68));
};


TEST(CharSet, Unite){
    {
        // 测试区间相离，相交，包含，被包含， negate = true / false
        CharSet charset1 = CharSet(3, 9), charset1_ = CharSet(3, 9, true);
        CharSet charset2 = CharSet(5,13), charset2_ = CharSet(5, 13, true);
        charset1.unite_update(10, 11), charset1.unite_update(12, 18), charset1.unite_update(20, 25);

        CharSet new_charset_1 = charset1.unite(charset2);
        CharSet new_charset_2 = charset1.unite(charset2_);
        CharSet new_charset_3 = charset1_.unite(charset2_);
        ASSERT_TRUE(new_charset_1.contains(6) && new_charset_2.contains(6) && !new_charset_3.contains(6));
        ASSERT_TRUE(new_charset_1.contains(3) && new_charset_2.contains(3) && new_charset_3.contains(3));
        ASSERT_TRUE(new_charset_1.contains(12) && new_charset_2.contains(12) && new_charset_3.contains(12));
        ASSERT_TRUE(new_charset_1.contains(23) && new_charset_2.contains(23) && new_charset_3.contains(23));
        ASSERT_TRUE(!new_charset_1.contains(19) && new_charset_2.contains(19) && new_charset_3.contains(19));
    }

    {
        // 测试区间多个同时被同一个区间被包含关系
        CharSet cs1 = CharSet(1, 100), cs2 = CharSet(3, 9);
        cs2.unite_update(15, 50), cs2.unite_update(70, 99);

        auto new_cs = cs1.unite(cs2);
        ASSERT_TRUE(new_cs.contains(18));
        ASSERT_TRUE(new_cs.contains(60));
    }
}