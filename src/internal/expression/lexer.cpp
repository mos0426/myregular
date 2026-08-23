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
    expression_(expression){
        assert(!expression.empty());
        current_ = expression_.cbegin();
        build_token();
};


void Lexer::next_token(){
    if (!is_end())
    {   
        build_token();
    }
    else
    {
        current_token_ = Token{Strcutural::END, 0};
    }
};


void Lexer::build_token(){
    switch (*current_)
    {
    case '*': current_token_ = Token{PostfixOperator::STAR, '*'}; current_++; break;
    case '+': current_token_ = Token{PostfixOperator::PLUS, '+'}; current_++; break;
    case '?': current_token_ = Token{PostfixOperator::QUEST, '?'}; current_++; break;
    case '(': current_token_ = Token{Atom::LPARENT, '('}; current_++; break;
    case ')': current_token_ = Token{Strcutural::RPARENT, ')'}; current_++; break;
    case '[': current_token_ = Token{Atom::LBRAKET, '['}; current_++; break;
    case ']': current_token_ = Token{Strcutural::RBRAKET, ']'}; current_++; break;
    case '{': current_token_ = Token{Strcutural::LBRACE, '{'}; current_++; break;
    case '}': current_token_ = Token{Strcutural::RBRACE, '}'}; current_++; break;
    case '|': current_token_ = Token{InfixOperator::PIPE, '|'}; current_++; break;
    case '.': current_token_ = Token{Atom::DOT, '.'}; current_++; break;
    case '\\': build_escape_token(); break;
    default:{
        auto [codepoint, next] = decode_codepoint(current_, expression_.cend());
        current_token_ = Token(Atom::CHAR, codepoint);
        current_ = next;
    }
    }
};


void Lexer::build_escape_token(){
    assert(*current_ == '\\');

    if ((current_+1)==expression_.cend()) throw LexerError("无效的转义序列：表达式以反斜杠结尾", expression_, get_pos());
    
    switch (*(current_+1))
    {
    case 'w': current_token_ = Token(Escape::WORD, 'w'); current_ += 2; break;
    case 'W': current_token_ = Token(Escape::NON_WORD, 'W'); current_ += 2; break;
    case 'd': current_token_ = Token(Escape::DIGIT, 'd'); current_ += 2; break;
    case 'D': current_token_ = Token(Escape::NON_DIGIT, 'D'); current_ += 2; break;
    case 'a': current_token_ = Token(Escape::ALPHABETIC, 'a'); current_ += 2; break;
    case 's': current_token_ = Token(Escape::WHITESPACE, 's'); current_ += 2; break;
    case 'S': current_token_ = Token(Escape::NON_WHITESPACE, 'S'); current_ += 2; break;
    case 'l': current_token_ = Token(Escape::LOWERCASE, 'l'); current_ += 2; break;
    case 'u': current_token_ = Token(Escape::UPPERCASE, 'u'); current_ += 2; break;
    default: {
        auto [codepoint, next] = decode_codepoint(current_+1, expression_.cend());
        current_token_ = Token(Atom::CHAR, codepoint);
        current_ = next;
    }
    }
};

