#include <string>
#include <iostream>
#include <format>
#include <memory>


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
    const char value;
    
    Token(TokenType t, char c):
        type(t), value(c) {};
    std::string toString() const;
    friend std::ostream &operator<<(std::ostream &os, const Token &token);
};

class AST{
public:
    Token &token;
    std::shared_ptr<AST> left;
    std::shared_ptr<AST> right;
    
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

    AST(Token &token, AST *left, AST *right, int precedence):
        token(token), left(left), right(right), precedence(precedence){};

    std::string format(const std::string &format_spec="-") const;

};


class  Lexer{
public:
    std::string &expression;

    Lexer(std::string &expression):
    expression(expression){
        current = expression.begin();
        pos = 0;
        current_token = nullptr;
    };

    const std::shared_ptr<Token> next_token();

    const std::shared_ptr<Token> get_curent_token(){
        return current_token;
    }

    const int get_pos(){
        return pos;
    }

private:
    std::string::iterator current;
    int pos;
    std::shared_ptr<Token> current_token;

    const Token tokenize(char c);
};