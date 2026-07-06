#include <string>
#include <iostream>
#include <optional>
#include <format>

enum class TokenType{
    CHARACTER,
    UNARY_POSTFIX_OPERATOR,
    UNARY_PREFIX_OPERATOR,
    INFIX_OPERATOR,  // "|" 和 连接 都是中缀操作符，其中连接是是隐式的, 比如 ab 表示 'a 连接 b'，其中连接符号隐去
    GROUP,
    BRACKET,
    WILDCARD
};


class Token{
public:
    const TokenType type;
    const std::string &value;

    Token(TokenType t, std::string &s):
        type(t), value(s) {};
    std::string toString() const;
    friend std::ostream &operator<<(std::ostream &os, const Token &token);
};

class AST{
public:
    Token &token;
    std::optional<AST> &left;
    std::optional<AST> &right;
    
    //优先级， 数值越高优先级越高
    // OR = 0   |
    //ANCHORING = 2   ^ $
    //CONNECT = 3   连接是是隐式的, 比如 ab 表示 'a 连接 b'，其中连接符号隐去
    //DUPLICATION = 4   * + ? {m,n}
    //GROUPING = 5   ()
    //BRACKET = 6   []
    //ESCAPED = 7   \<special character>
    //COLLATION_RELATED_BRACKET_SYMBOLS = 8   [==] [::] [..]
    //CHARACTER = 9   <character> .
    int precedence;

    AST(Token &t, std::optional<AST> &l, std::optional<AST> &r, int p):
        token(t), left(l), right(r), precedence(p){};

    std::string format(const std::string &format_spec="-") const;

};


// class  Lexer{
// public:
//     std::string &expression;

// private:
//     int length;
    
//     int pos;
//     std::optional<Token> &current_token;
// }