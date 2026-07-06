#include "expression.hpp"



std::string tokenTypeToString(TokenType type){
    switch (type)
    {
    case TokenType::CHARACTER: return "CHARACTER";
    case TokenType::UNARY_POSTFIX_OPERATOR: return "UNARY_POSTFIX_OPERATOR";
    case TokenType::UNARY_PREFIX_OPERATOR: return "UNARY_PREFIX_OPERATOR";
    case TokenType::INFIX_OPERATOR: return "INFIX_OPERATOR";
    case TokenType::GROUP: return "GROUP";
    case TokenType::BRACKET: return "BRACKET";
    case TokenType::WILDCARD: return "WILDCARD";
    default: return "UNKNOW";
    }
}


std::string Token::toString() const {
        return value + tokenTypeToString(type);
    };


std::ostream &operator<<(std::ostream &os, const Token &token){
        return os<<token.toString();
    }


std::string AST::format(const std::string &format_spec) const {

        std::string left_format = "\n" + left.has_value()?left.value().format(format_spec):"None";
        std::string right_format = "\n" + right.has_value()?right.value().format(format_spec):"None";
        
        return std::format("[{}AST:\n {}token:{}\n {}left:{}\n {}right:{}]",
                       format_spec, format_spec, token.toString(),
                       format_spec, left_format,
                       format_spec, right_format);
    };