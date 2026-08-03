#include <variant>
#include <memory>
#include <string>
#include <cstdint>

#include "lexer.hpp"
#include "parse.hpp"
#include "error.hpp"
#include "constants.hpp"


static std::unique_ptr<AST> parse_expression(Lexer &lexer);
static std::unique_ptr<AST> _parse_expression(Lexer &lexer, std::unique_ptr<AST> left);
static std::unique_ptr<AST> parse_concatenation(Lexer &lexer);
static std::unique_ptr<AST> _parse_concatenation(Lexer &lexer, std::unique_ptr<AST> left);
static std::unique_ptr<AST> parse_factor(Lexer &lexer);
static std::unique_ptr<AST> parse_atom(Lexer &lexer);
static std::unique_ptr<AST> parse_repetition(Lexer &lexer, std::unique_ptr<AST> left);
static uint32_t parse_number(Lexer &lexer);
static std::unique_ptr<AST> parse_especial(Lexer &lexer);
static std::unique_ptr<AST> parse_char_class(Lexer &lexer);
static inline std::vector<CharRange> parse_char_class_content(Lexer &lexer);


std::unique_ptr<AST> parse(std::string_view &expression){
    Lexer lexer(expression);
    return parse_expression(lexer);
}


static std::unique_ptr<AST> parse_expression(Lexer &lexer){
    // 解析一个正则表达式
    // expression <- concatenation ('|' concatenation)*
    std::unique_ptr<AST> left = parse_concatenation(lexer);    
    return _parse_expression(lexer, std::move(left));
}

static std::unique_ptr<AST> _parse_expression(Lexer &lexer, std::unique_ptr<AST> left){
    // parse_expression 的递归辅助函数，用于处理 '|' 操作符
    // 默认情况下，lexrer 已经消耗了左操作数，当前 token 是 '|' 或是结束标识符 ')' 或 'END'
    Token token = lexer.get_current_token();
    if (std::holds_alternative<InfixOperator>(token.type) && std::get<InfixOperator>(token.type) == InfixOperator::PIPE)
    {
        lexer.next_token(); // 消耗 '|'
        std::unique_ptr<AST> right = parse_concatenation(lexer);
        return _parse_expression(lexer, std::make_unique<AST>(UnionNode{std::move(left), std::move(right)}));
    }
    else if (
        std::holds_alternative<Strcutural>(token.type) &&
        (std::get<Strcutural>(token.type) == Strcutural::RPARENT || std::get<Strcutural>(token.type) == Strcutural::END)
    )
    {
        // 遇到 ')' 或 'END'，返回左操作数，交给上层处理
        return left;
    }
    else
    {
        throw ParserError("无效的表达式", lexer.get_expression(), lexer.get_pos());
    }
    
}


static std::unique_ptr<AST> parse_concatenation(Lexer &lexer){
    // 解析一个连接表达式
    // 解析到 '|' 或 'End' 或 ')' 为止
    // concatenation <- factor (factor)*

    std::unique_ptr<AST> left = parse_factor(lexer);
    return _parse_concatenation(lexer, std::move(left));
}

static std::unique_ptr<AST> _parse_concatenation(Lexer &lexer, std::unique_ptr<AST> left){

    Token token = lexer.get_current_token();
    std::unique_ptr<AST> right = nullptr;
    if (std::holds_alternative<Strcutural>(token.type) && (std::get<Strcutural>(token.type) == Strcutural::RPARENT || std::get<Strcutural>(token.type) == Strcutural::END)){
        // 遇到 ')' 或者 END，返回左操作数，交给上层处理
        return left;
    }
    else if (std::holds_alternative<InfixOperator>(token.type) && std::get<InfixOperator>(token.type) == InfixOperator::PIPE){
        // 遇到 '|'，返回左操作数，交给上层处理
        return left;
    }
    else{
        // 继续解析右操作数
        right = parse_factor(lexer);
    } 
    return _parse_concatenation(lexer, std::make_unique<AST>(ConcatenationNode{std::move(left), std::move(right)}));
}


static std::unique_ptr<AST> parse_factor(Lexer &lexer){
    // 解析一个因子，因子可以是原子（atom）或者带有后缀操作符的原子
    // factor <- atom [* | + | ? | repetition]
    std::unique_ptr<AST> atom = parse_atom(lexer);

    Token token = lexer.get_current_token();

    // 处理后缀操作符
    if (auto *postfix = std::get_if<PostfixOperator>(&token.type)){
        switch (*postfix)
        {
        case PostfixOperator::STAR:
            lexer.next_token(); // 消耗当前 token
            return std::make_unique<AST>(StarNode{std::move(atom)});
        case PostfixOperator::PLUS:
            lexer.next_token(); // 消耗当前 token
            return std::make_unique<AST>(PlusNode{std::move(atom)});
        case PostfixOperator::QUEST:
            lexer.next_token(); // 消耗当前 token
            return std::make_unique<AST>(RepetitionNode{std::move(atom), 0, 1});
        default:
            throw ParserError("无效的后缀操作符", lexer.get_expression(), lexer.get_pos());
        }
    }

    // 处理重复操作符 {m,n}
    if (auto *lbrace = std::get_if<Strcutural>(&token.type)){
        if (*lbrace == Strcutural::LBRACE){
            return parse_repetition(lexer, std::move(atom));
        }
    }
    
    // 无任何量词修饰，直接返回原子节点
    return atom;
}


static std::unique_ptr<AST> parse_atom(Lexer &lexer){
    // 解析一个原子，原子可以是字符、转义字符、括号表达式、字符类等
    // atom <- CHAR | ESCAPE | '(' expression ')' | '[' char_class ']' | '.'
    Token token = lexer.get_current_token();
    if (auto *atom = std::get_if<Atom>(&token.type)){
        switch (*atom)
        {
        case Atom::CHAR:
            lexer.next_token(); // 消耗当前 token
            return std::make_unique<AST>(CharNode{token.value});
        case Atom::LPARENT:{
            lexer.next_token(); // 消耗 '('
            std::unique_ptr<AST> expr = parse_expression(lexer);
            token = lexer.get_current_token();
            if ( !std::holds_alternative<Strcutural>(token.type) || std::get<Strcutural>(token.type) != Strcutural::RPARENT){
                throw ParserError("缺少 ')'", lexer.get_expression(), lexer.get_pos());
            }
            lexer.next_token(); // 消耗 ')'
            return expr;
        }
        case Atom::LBRAKET:
            return parse_char_class(lexer);
        case Atom::DOT:
            lexer.next_token(); // 消耗 '.'
            return std::make_unique<AST>(CharClassNode{{
                CharRange{}},
                true // DOT 表示匹配任意字符，取反的字符类
            } 
            );
        default:
            throw ParserError("无效的原子类型", lexer.get_expression(), lexer.get_pos());
        }
    }
    else if (std::holds_alternative<Escape>(token.type)){
        return parse_especial(lexer);
    }
    else{
        throw ParserError("无效的原子类型", lexer.get_expression(), lexer.get_pos());
    }
};


static std::unique_ptr<AST> parse_repetition(Lexer &lexer, std::unique_ptr<AST> left){
    // 解析重复操作符 {m,n}
    // repetition <- '{' NUMBER [',' NUMBER] '}'
    lexer.next_token(); // 消耗 '{'
    unsigned int min = 0, max = 0;
    try{
        min = parse_number(lexer);
    }
    catch (const ParserError &){
        throw ParserError("无效的重复操作符：缺少最小重复次数", lexer.get_expression(), lexer.get_pos());
    }

    Token token = lexer.get_current_token();
    if (token.value == ','){
        lexer.next_token(); // 消耗 ','
        max = parse_number(lexer);
    }
    else{
        max = min; // 如果没有 ',', 则表示 {m}，即 min = max, {m, m}
    }

    // 检查是否有 '}'
    token = lexer.get_current_token();
    if (std::holds_alternative<Strcutural>(token.type) && std::get<Strcutural>(token.type) == Strcutural::RBRACE){
        lexer.next_token(); // 消耗 '}'
    }
    else{
        throw ParserError("缺少 '}'", lexer.get_expression(), lexer.get_pos());
    }

    return std::make_unique<AST>(RepetitionNode{std::move(left), min, max});
};


static uint32_t parse_number(Lexer &lexer){
    // 解析一个数字
    // NUMBER <- [0-9]+
    Token token = lexer.get_current_token();
    
    if (auto *atom = std::get_if<Atom>(&token.type)){
        if (*atom == Atom::CHAR && token.value >= '0' && token.value <= '9'){
            uint32_t number = token.value - '0';
            lexer.next_token(); // 消耗数字
            
            while (true)
            {
                token = lexer.get_current_token();
                if (lexer.is_end()) return number; // 没有更多的 token，返回当前数字
                if (auto *atom2 = std::get_if<Atom>(&token.type)){
                    if (*atom2 == Atom::CHAR && token.value >= '0' && token.value <= '9'){
                        number = number * 10 + (token.value - '0');
                        lexer.next_token(); // 消耗数字
                        continue;
                    }
                }
                return number; // 遇到非数字 token，返回当前数字
            }
            
        }
    }
    throw ParserError("无效的数字", lexer.get_expression(), lexer.get_pos());
    
}


static std::unique_ptr<AST> parse_especial(Lexer &lexer){
    // 解析转义字符
    Token token = lexer.get_current_token();

    if (auto *escape = std::get_if<Escape>(&token.type)){
        lexer.next_token(); // 消耗当前 token
        switch (*escape)
        {
            case Escape::WORD:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{DIGIT_RANGE.first, DIGIT_RANGE.second},
                    CharRange{ALPHA_RANGE.first, ALPHA_RANGE.second},
                    CharRange{ALPHA_LOWER_RANGE.first, ALPHA_LOWER_RANGE.second},
                    CharRange{UNDERSCORE, UNDERSCORE + 1}}}
                );
            case Escape::NON_WORD:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{DIGIT_RANGE.first, DIGIT_RANGE.second},
                    CharRange{ALPHA_RANGE.first, ALPHA_RANGE.second},
                    CharRange{ALPHA_LOWER_RANGE.first, ALPHA_LOWER_RANGE.second},
                    CharRange{UNDERSCORE, UNDERSCORE + 1}},
                true}
                );
            case Escape::DIGIT:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{DIGIT_RANGE.first, DIGIT_RANGE.second}}}
                );
            case Escape::NON_DIGIT:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{DIGIT_RANGE.first, DIGIT_RANGE.second}},
                    true}
                );
            case Escape::ALPHABETIC:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{ALPHA_RANGE.first, ALPHA_RANGE.second}}}
                );
            case Escape::WHITESPACE:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{WHITESPACE_START, WHITESPACE_END + 1},
                    CharRange{SPACE, SPACE + 1}}}
                );
            case Escape::NON_WHITESPACE:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{WHITESPACE_START, WHITESPACE_END + 1},
                    CharRange{SPACE, SPACE + 1}},
                    true}
                );
            case Escape::LOWERCASE:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{ALPHA_LOWER_RANGE.first, ALPHA_LOWER_RANGE.second}}}
                );
            case Escape::UPPERCASE:
                lexer.next_token(); // 消耗当前 token
                return std::make_unique<AST>(CharClassNode{{
                    CharRange{ALPHA_RANGE.first, ALPHA_RANGE.second}}}
                );
            default:
                throw ParserError("无效的转义字符", lexer.get_expression(), lexer.get_pos());
        }
    }
    else{
        throw ParserError("无效的转义字符", lexer.get_expression(), lexer.get_pos());
    }
}


static std::unique_ptr<AST> parse_char_class(Lexer &lexer){
    // 解析一个字符集
    // char_class <- '[' '^'? char_class_content ']'
    lexer.next_token(); // 消耗 '['
    Token token = lexer.get_current_token();
    bool negated = false;

    if (std::holds_alternative<Atom>(token.type) && std::get<Atom>(token.type) == Atom::CHAR && token.value == '^'){
        // 取反的字符类
        lexer.next_token(); // 消耗 '^'
        negated = true;
    }
    return std::make_unique<AST>(CharClassNode{parse_char_class_content(lexer), negated});
}


static inline std::vector<CharRange> parse_char_class_content(Lexer &lexer){
    // 解析字符类的内容，直到遇到 ']' 为止, 返回 CharClassNode
    // 这里仅有'-'，'\' 和 ']' 视为特殊字符，其他字符都视为普通字符
    //  char_class_content <- (CHAR | CHAR '-' CHAR | ESCAPE)* 
    std::vector<CharRange> ranges;
    Token token = lexer.get_current_token();
    lexer.next_token(); // 消耗当前 token
    Token next_token = lexer.get_current_token();
    
    while (true)
    {
        if (next_token.value == '-'){
            // 遇到 '-'，解析范围情况, 如 [a-z]
            lexer.next_token(); // 消耗 '-'
            Token end_token = lexer.get_current_token();

            if (end_token.value == ']'){
                // 如果 '-' 后面紧跟 ']'，则 '-' 被视为普通字符
                ranges.push_back(CharRange{token.value, token.value + 1});
                ranges.push_back(CharRange{'-', '-' + 1});
                lexer.next_token(); // 消耗 ']'
                return ranges;
            }
            else if (std::holds_alternative<Strcutural>(end_token.type) && std::get<Strcutural>(end_token.type) == Strcutural::END){
                // 如果 '-' 后面紧跟 'End'，则抛出异常，表示无效的字符范围
                throw ParserError("无效的字符范围", lexer.get_expression(), lexer.get_pos());
            }

            // 检查范围是否合法，范围必须是升序的，即 start < end
            if (token.value > end_token.value) throw ParserError("无效的字符范围, 范围必须是升序的，即 start < end", lexer.get_expression(), lexer.get_pos());
            
            ranges.push_back(CharRange{token.value, end_token.value + 1});
            lexer.next_token(); // 消耗 end_token            
        }
        else if (next_token.value == ']'){
            // 遇到 ']'，结束字符类的解析
            ranges.push_back(CharRange{token.value, token.value + 1});
            lexer.next_token(); // 消耗 ']'
            return ranges;
        }
        else if (std::holds_alternative<Strcutural>(token.type) && std::get<Strcutural>(token.type) == Strcutural::END){
            // 遇到 'End'，抛出异常，表示字符类没有正确闭合
            throw ParserError("字符类没有正确闭合", lexer.get_expression(), lexer.get_pos());
        }
        else{
            // 普通字符，直接加入 ranges
            ranges.push_back(CharRange{token.value, token.value + 1});
        }

        token = lexer.get_current_token();
        lexer.next_token();
        next_token = lexer.get_current_token();
    }
    
}


