#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.unite_update(32, 64);
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));

    cs.unite_update(CharSet(48, 72, true));
    ASSERT_TRUE(cs.contains(84));
    ASSERT_TRUE(cs.contains(28));
    ASSERT_FALSE(cs.contains(68));

    cs.unite_wildcard_update();
    ASSERT_TRUE(cs.contains(68));
};


TEST(CharSet, Unite){
    // 除了测试 CharSet::unite, 间接测试端点集的交并差接口:
    //  - endpoint_set_union;
    //  - endpoint_set_intersection
    //  - endpoint_set_difference 

    // 测试区间相离，相交，包含，被包含， negate = true / false
    {
        
        CharSet charset1 = CharSet(3, 9), charset1_ = CharSet(3, 9);
        CharSet charset2 = CharSet(5,13), charset2_ = CharSet(5, 13);
        charset1_.negation_update(), charset2_.negation_update();
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

    // 间接测试 endpoint_set_union
    // 测试区间多个同时被同一个区间被包含关系
    {
        CharSet cs1 = CharSet(1, 100), cs2 = CharSet(1, 9);
        cs2.unite_update(15, 50), cs2.unite_update(70, 100);

        auto new_cs = cs1.unite(cs2);
        ASSERT_TRUE(new_cs.contains(18));
        ASSERT_TRUE(new_cs.contains(60));
    }

    // 间接测试 endpoint_set_intersection
    { 
        // 相离
        CharSet cs1 = CharSet(1, 3), cs2 = CharSet(4, 6);
        // 相交
        cs1.unite_update(7, 10), cs2.unite_update(8, 12);
        // 包含
        cs1.unite_update(20, 30), cs2.unite_update(20, 28);
        // 多个同时包含
        cs1.unite_update(40, 60), cs2.unite_update(43, 45), cs2.unite_update(49, 53), cs2.unite_update(57, 60);

        cs1.negation_update(), cs2.negation_update();

        auto new_cs = cs1.unite(cs2);

        // 相离
        ASSERT_TRUE(new_cs.contains(2) && new_cs.contains(4));
        // 相交
        ASSERT_TRUE(new_cs.contains(7) && !new_cs.contains(9) && new_cs.contains(11));
        // 包含
        ASSERT_TRUE(!new_cs.contains(20) && !new_cs.contains(27) && new_cs.contains(30));
        // 多个同时包含
        ASSERT_TRUE(
            new_cs.contains(40) && !new_cs.contains(44) && new_cs.contains(45) && !new_cs.contains(49)
            && new_cs.contains(55) && !new_cs.contains(58) && new_cs.contains(61)
        );

    }

    // 间接测试 endpoint_set_difference
    {
                // 相离
        CharSet cs1 = CharSet(1, 3), cs2 = CharSet(4, 6);
        // 相交
        cs1.unite_update(7, 10), cs2.unite_update(7, 8), cs2.unite_update(9, 12);
        // 包含
        cs1.unite_update(20, 30), cs2.unite_update(20, 28);
        // 多个同时包含
        cs1.unite_update(40, 60), cs2.unite_update(43, 45), cs2.unite_update(49, 53), cs2.unite_update(57, 60);

        cs1.negation_update();

        auto new_cs = cs1.unite(cs2);

        // 相离
        ASSERT_TRUE(!new_cs.contains(2) && new_cs.contains(4));
        // 相交
        ASSERT_TRUE(new_cs.contains(7) && !new_cs.contains(8) && new_cs.contains(11));
        // 包含
        ASSERT_TRUE(new_cs.contains(20) && new_cs.contains(27) && !new_cs.contains(29));
        // 多个同时包含
        ASSERT_TRUE(
            !new_cs.contains(40) && new_cs.contains(44) && !new_cs.contains(45) && new_cs.contains(49)
            && !new_cs.contains(55) && new_cs.contains(58) && new_cs.contains(61)
        );
    }
}