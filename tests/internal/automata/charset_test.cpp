#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.unite_update(32, 64);
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));
    
    cs.unite_update(CODEPOINT_MIN, 48);
    cs.unite_update(72, CODEPOINT_MAX);
    ASSERT_TRUE(cs.contains(84));
    ASSERT_TRUE(cs.contains(28));
    ASSERT_FALSE(cs.contains(68));

    cs.unite_wildcard_update();
    ASSERT_TRUE(cs.contains(68));
};
