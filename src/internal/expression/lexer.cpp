#include <string_view>
#include <cassert>
#include <utility>

#include "lexer.hpp"
#include "utf8.hpp"
#include "error.hpp"


Lexer::Lexer(std::string_view &expression):
    expression(expression){
        assert(!expression.empty());
        current = expression.cbegin();
        current_token = nullptr;
};


const std::shared_ptr<Token> Lexer::next_token(){
    if (current<expression.cend())
    {   
        auto [token, next] = build_token();
        current_token = std::make_shared<Token>(token);
    }
    else
    {
        current = expression.cend();
        current_token = nullptr;
    }
    return current_token;
};


std::pair<Token,std::string_view::const_iterator> Lexer::build_token(){
    switch (*current)
    {
    case '*': return {Token{PostfixOperator::STAR, 0}, current+1};
    case '+': return {Token{PostfixOperator::PLUS, 0}, current+1};
    case '?': return {Token{PostfixOperator::QUEST, 0}, current+1};
    case '(': return {Token{Atom::LPARENT, 0}, current+1};
    case ')': return {Token{Strcutural::RPARENT, 0}, current+1};
    case '[': return {Token{Atom::LBRAKET, 0}, current+1};
    case ']': return {Token{Strcutural::RBRAKET, 0}, current+1};
    case '|': return {Token{InfixOperator::PIPE, 0}, current+1};
    case '.': return {Token{Atom::DOT, 0}, current+1};
    case '\\': return build_escape_token();
    default:{
        auto [codepoint, next] = decode_codepoint(current, expression.cend());
        return {Token(Atom::CHAR, codepoint), next};
    }
    }
};


std::pair<Token, std::string_view::const_iterator> Lexer::build_escape_token(){
    assert(*current == '\\');

    if ((current+1)==expression.end()) throw LexerError("无效的转义序列：表达式以反斜杠结尾", expression, get_pos());
    
    switch (*(current+1))
    {
    case 'w': return {Token(Escape::WORD, 0), current+2};
    case 'W': return {Token(Escape::NON_WORD, 0), current+2};
    case 'd': return {Token(Escape::DIGIT, 0), current+2};
    case 'D': return {Token(Escape::NON_DIGIT, 0), current+2};
    case 'a': return {Token(Escape::ALPHABETIC, 0), current+2};
    case 's': return {Token(Escape::WHITESPACE, 0), current+2};
    case 'S': return {Token(Escape::NON_WHITESPACE, 0), current+2};
    case 'l': return {Token(Escape::LOWERCASE, 0), current+2};
    case 'u': return {Token(Escape::UPPERCASE, 0), current+2};
    default: {
        auto [codepoint, next] = decode_codepoint(current, expression.cend());
        return {Token(Atom::CHAR, codepoint), next};
    }
    }
};

