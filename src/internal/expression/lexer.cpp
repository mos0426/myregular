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


Lexer::Lexer(std::string_view expression):
    expression(expression){
        assert(!expression.empty());
        current = expression.cbegin();
        build_token();
};


void Lexer::next_token(){
    if (!is_end())
    {   
        build_token();
    }
    else
    {
        current_token = Token{Strcutural::END, 0};
    }
};


void Lexer::build_token(){
    switch (*current)
    {
    case '*': current_token = Token{PostfixOperator::STAR, '*'}; current++; break;
    case '+': current_token = Token{PostfixOperator::PLUS, '+'}; current++; break;
    case '?': current_token = Token{PostfixOperator::QUEST, '?'}; current++; break;
    case '(': current_token = Token{Atom::LPARENT, '('}; current++; break;
    case ')': current_token = Token{Strcutural::RPARENT, ')'}; current++; break;
    case '[': current_token = Token{Atom::LBRAKET, '['}; current++; break;
    case ']': current_token = Token{Strcutural::RBRAKET, ']'}; current++; break;
    case '|': current_token = Token{InfixOperator::PIPE, '|'}; current++; break;
    case '.': current_token = Token{Atom::DOT, '.'}; current++; break;
    case '\\': build_escape_token(); break;
    default:{
        auto [codepoint, next] = decode_codepoint(current, expression.cend());
        current_token = Token(Atom::CHAR, codepoint);
        current = next;
    }
    }
};


void Lexer::build_escape_token(){
    assert(*current == '\\');

    if ((current+1)==expression.end()) throw LexerError("无效的转义序列：表达式以反斜杠结尾", expression, get_pos());
    
    switch (*(current+1))
    {
    case 'w': current_token = Token(Escape::WORD, 'w'); current += 2; break;
    case 'W': current_token = Token(Escape::NON_WORD, 'W'); current += 2; break;
    case 'd': current_token = Token(Escape::DIGIT, 'd'); current += 2; break;
    case 'D': current_token = Token(Escape::NON_DIGIT, 'D'); current += 2; break;
    case 'a': current_token = Token(Escape::ALPHABETIC, 'a'); current += 2; break;
    case 's': current_token = Token(Escape::WHITESPACE, 's'); current += 2; break;
    case 'S': current_token = Token(Escape::NON_WHITESPACE, 'S'); current += 2; break;
    case 'l': current_token = Token(Escape::LOWERCASE, 'l'); current += 2; break;
    case 'u': current_token = Token(Escape::UPPERCASE, 'u'); current += 2; break;
    default: {
        auto [codepoint, next] = decode_codepoint(current, expression.cend());
        current_token = Token(Atom::CHAR, codepoint);
        current = next;
    }
    }
};

