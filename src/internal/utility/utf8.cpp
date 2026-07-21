#include <utility>
#include <cstdint>

#include "utf8.hpp"


DecodeResult decode_codepoint(const char *first, const char *last){

    unsigned char c = static_cast<unsigned char>(*first);
    uint32_t code_point = 0;
    int extra_bytes = 0;

    // 1 个字节的 utf-8
    if ((c & 0x80) == 0)
    {
        code_point = c;
        extra_bytes = 0;
    }
    // 2 个字节的 utf-8
    else if ((c & 0xE0) == 0xC0)
    {
        code_point = (c & 0x1F);
        extra_bytes = 1;
    }
    // 3 个字节的 utf-8
    else if ((c & 0xF0) == 0xE0)
    {
        code_point = (c & 0x0F);
        extra_bytes = 2;
    }
    // 4 个字节的 utf-8
    else if ((c & 0xF8) == 0xF0)
    {
        code_point = (c & 0x07);
        extra_bytes = 3;
    }
    else
    {
        // 未识别出首字符，返回替换字符 0xFFFD
        return std::pair(0xFFFD, first + 1);
    }

    if ((first + extra_bytes) >= last)
    {   
        // 不完整序列，返回替换字符 0xFFFD
        return std::pair(0xFFFD, last);
    }

    const char *p = first + 1;
    for (int i = 0; i < extra_bytes; i++)
    {   
        c = static_cast<unsigned char>(*p);
        if ((c & 0xC0) == 0x80)
        {
            code_point = (code_point << 6) | (c & 0x3F);
            p++;
        }
        else
        {   // 无效续字节，返回替换字符 0xFFFD
            return std::pair(0xFFFD, ++p);
        }
        
    }

    return std::pair(code_point, p);

};