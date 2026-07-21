#include <string>
#include <utility>
#include <cstdint>

#include <gtest/gtest.h>

#include "utf8.hpp"

TEST(decode_codepoint_test, base_test){
    std::string s = "abc";
    DecodeResult r = decode_codepoint(s.c_str(), &s.back());
    EXPECT_EQ(r.first, 97);
    EXPECT_EQ(r.second, s.c_str()+1);

    std::string chinese_string = "中文";
    DecodeResult r2 = decode_codepoint(chinese_string.c_str(), &chinese_string.back());
    EXPECT_EQ(r2.first, 20013);
    DecodeResult r3 = decode_codepoint(chinese_string.c_str()+1, &chinese_string.back());
    EXPECT_EQ(r3.first, 0xFFFD);
    EXPECT_EQ(r3.second, chinese_string.c_str()+2);
    DecodeResult r4 = decode_codepoint(chinese_string.c_str(), chinese_string.c_str()+2);
    EXPECT_EQ(r4.first, 0xFFFD);
    EXPECT_EQ(r4.second, chinese_string.c_str()+2);
}