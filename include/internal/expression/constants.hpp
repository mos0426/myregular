#include <cstdint>
#include <utility>


constexpr uint32_t DIGIT_START = 0x30; // '0'
constexpr uint32_t DIGIT_END = 0x39;   // '9'
constexpr uint32_t ALPHA_UPPER_START = 0x41; // 'A'
constexpr uint32_t ALPHA_UPPER_END = 0x5A;   // 'Z'
constexpr uint32_t ALPHA_LOWER_START = 0x61; // 'a'
constexpr uint32_t ALPHA_LOWER_END = 0x7A;   // 'z'
constexpr uint32_t UNDERSCORE = 0x5F; // '_'
constexpr uint32_t WHITESPACE_START = 0x09; // '\t'
constexpr uint32_t WHITESPACE_END = 0x0D;   // '\r'
constexpr uint32_t SPACE = 0x20; // ' '


constexpr std::pair<uint32_t, uint32_t> DIGIT_RANGE = {DIGIT_START, DIGIT_END + 1}; // [0-9]
constexpr std::pair<uint32_t, uint32_t> ALPHA_UPPER_RANGE = {ALPHA_UPPER_START, ALPHA_UPPER_END + 1}; // [A-Z]
constexpr std::pair<uint32_t, uint32_t> ALPHA_LOWER_RANGE = {ALPHA_LOWER_START, ALPHA_LOWER_END + 1}; // [a-z]