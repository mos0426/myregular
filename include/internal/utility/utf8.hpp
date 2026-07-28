#include <utility>
#include <cstdint>
#include <string_view>


// 解码返回的结果，结果形式为 pair<码点， 指向下一个字符的首字节>
using DecodeResult = std::pair<const uint32_t, std::string_view::const_iterator>;


// 从 [first, last) 区间中解码第一个 utf-8 的码点
DecodeResult decode_codepoint(std::string_view::const_iterator first, std::string_view::const_iterator last);