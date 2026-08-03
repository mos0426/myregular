#include <string>
#include <string_view>
#include <stdexcept>


#include "error.hpp"


ExpressionError::ExpressionError(
    const std::string &message, 
    const std::string_view &pattern, 
    std::size_t pos,
    const std::string &prefix
    ):
    std::runtime_error(""), message_(message), pattern_(pattern), pos_(pos){
    
        full_message_.append("[" + prefix + "]: ");
        full_message_.append(message_);
        full_message_.append("\n");
        full_message_.append(pattern_);
        full_message_.append("\n");

        // 显示错误位置
        full_message_.append(std::string(pos_-1, ' '));
        full_message_.append("^");
    }