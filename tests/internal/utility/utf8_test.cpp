#include <string>
#include <utility>
#include <cstdint>

#include <gtest/gtest.h>

#include "utf8.hpp"

TEST(decode_codepoint_test, base_test){
    std::string s = "abc";
    std::string_view sv = s;
    DecodeResult r = decode_codepoint(sv.begin(), sv.end());
    EXPECT_EQ(r.first, 97);
    EXPECT_EQ(r.second, sv.begin() + 1);

    std::string chinese_string = "中文";
    std::string_view chinese_sv = chinese_string;
    DecodeResult r2 = decode_codepoint(chinese_sv.begin(), chinese_sv.end());
    EXPECT_EQ(r2.first, 20013);
    DecodeResult r3 = decode_codepoint(chinese_sv.begin() + 1, chinese_sv.end());
    EXPECT_EQ(r3.first, 0xFFFD);
    EXPECT_EQ(r3.second, chinese_sv.begin() + 2);
    DecodeResult r4 = decode_codepoint(chinese_sv.begin(), chinese_sv.begin() + 2);
    EXPECT_EQ(r4.first, 0xFFFD);
    EXPECT_EQ(r4.second, chinese_sv.begin() + 2);
}