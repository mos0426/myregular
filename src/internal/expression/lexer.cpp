#include <cassert>
#include <utility>
#include <string_view>
#include <memory>
#include <variant>
#include <cstdint>
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
        current_token = std::make_shared<Token>(Token{Strcutural::END, 0});
    }
    return current_token;
};


std::pair<Token,std::string_view::const_iterator> Lexer::build_token(){
    switch (*current)
    {
    case '*': return {Token{PostfixOperator::STAR, '*'}, current+1};
    case '+': return {Token{PostfixOperator::PLUS, '+'}, current+1};
    case '?': return {Token{PostfixOperator::QUEST, '?'}, current+1};
    case '(': return {Token{Atom::LPARENT, '('}, current+1};
    case ')': return {Token{Strcutural::RPARENT, ')'}, current+1};
    case '[': return {Token{Atom::LBRAKET, '['}, current+1};
    case ']': return {Token{Strcutural::RBRAKET, ']'}, current+1};
    case '|': return {Token{InfixOperator::PIPE, '|'}, current+1};
    case '.': return {Token{Atom::DOT, '.'}, current+1};
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
    case 'w': return {Token(Escape::WORD, 'w'), current+2};
    case 'W': return {Token(Escape::NON_WORD, 'W'), current+2};
    case 'd': return {Token(Escape::DIGIT, 'd'), current+2};
    case 'D': return {Token(Escape::NON_DIGIT, 'D'), current+2};
    case 'a': return {Token(Escape::ALPHABETIC, 'a'), current+2};
    case 's': return {Token(Escape::WHITESPACE, 's'), current+2};
    case 'S': return {Token(Escape::NON_WHITESPACE, 'S'), current+2};
    case 'l': return {Token(Escape::LOWERCASE, 'l'), current+2};
    case 'u': return {Token(Escape::UPPERCASE, 'u'), current+2};
    default: {
        auto [codepoint, next] = decode_codepoint(current, expression.cend());
        return {Token(Atom::CHAR, codepoint), next};
    }
    }
};

