#include <gtest/gtest.h>


#include "charset.hpp"


TEST(CharSetTest, BASIC){
    CharSet cs(24, 32);
    cs.add(32, 64);
    ASSERT_TRUE(cs.contains(32));
    ASSERT_FALSE(cs.contains(64));
};