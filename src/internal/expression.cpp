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

        std::string left_format = "\n" + (left == nullptr)? (*left).format(format_spec):"None";
        std::string right_format = "\n" + (right == nullptr)?(*right).format(format_spec):"None";
        
        return std::format("[{}AST:\n {}token:{}\n {}left:{}\n {}right:{}]",
                       format_spec, format_spec, token.toString(),
                       format_spec, left_format,
                       format_spec, right_format);
    };


const std::shared_ptr<Token> Lexer::next_token(){
    if (current == expression.end())
        return nullptr;
    else
    {
        current ++;
        if (current == expression.end())
            current_token = nullptr;
        else
            current_token = std::make_shared<Token>(tokenize(*current));
        return current_token;
    }
}


const Token Lexer::tokenize(char c){
    switch (c)
    {
    case '*':
    case '+':
    case '?':
        return Token(TokenType::UNARY_POSTFIX_OPERATOR, c);
    case '\\':
        return Token(TokenType::UNARY_PREFIX_OPERATOR, c);
    case '|':
        return  Token(TokenType::INFIX_OPERATOR, c);
    case '(':
    case ')':
        return Token(TokenType::GROUP, c);
    case '[':
    case ']':
        return Token(TokenType::BRACKET, c);
    case '.':
        return Token(TokenType::WILDCARD, c);
    default:
        return Token(TokenType::CHARACTER, c);
    }
}
