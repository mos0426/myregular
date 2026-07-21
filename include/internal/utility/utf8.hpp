#include <utility>
#include <cstdint>


// 解码返回的结果，结果形式为 pair<码点， 指向下一个字符的首字节>
using DecodeResult = std::pair<const uint32_t, const char*>;


// 从 [first, last) 区间中解码第一个 utf-8 的码点
DecodeResult decode_codepoint(const char *first, const char *last);