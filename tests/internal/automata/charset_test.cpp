#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.add(32, 64);
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));

    cs.add(48, 72, true);
    ASSERT_TRUE(cs.contains(84));
    ASSERT_TRUE(cs.contains(28));
    ASSERT_FALSE(cs.contains(68));

    cs.add_wildcard();
    ASSERT_TRUE(cs.contains(68));
};