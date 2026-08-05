#include <string_view>
#include <memory>
#include <variant>
#include <cstdint>
#include <utility>


enum class Atom: uint8_t {
    CHAR = 0, 
    LPARENT = 1,    // (
    DOT = 2,        // .
    LBRAKET = 3,    // [
};


enum class PostfixOperator: uint8_t{
    STAR = 0,       // *
    PLUS = 1,       // +
    QUEST = 2,      // ?
};


enum class InfixOperator: uint8_t{
    PIPE = 0,       //  |
};


enum class Escape: uint8_t{
    WORD = 0,           // \w -> [a-zA-Z0-9_]
    NON_WORD = 1,       // \W -> [^a-zA-Z0-9_]
    DIGIT = 2,          // \d -> [0-9]
    NON_DIGIT = 3,      // \D -> [^0-9]
    ALPHABETIC = 4,     // \a -> [a-zA-Z]
    WHITESPACE = 5,     // \s -> [ \t\r\n\v\f]
    NON_WHITESPACE = 6, // \S -> [^ \t\r\n\v\f]
    LOWERCASE = 7,      // \l -> [a-z]
    UPPERCASE = 8,      // \u -> [A-Z]    
};


// 结构控制符，用于 parse 的流向控制
enum class Strcutural: uint8_t{
    RPARENT = 0,    // )
    RBRAKET = 1,    // ]
    LBRACE = 2,     // {
    RBRACE = 3,     // }
    END = 4,        // 表达式的结尾标志
};


using TokenType = std::variant<
    Atom,
    PostfixOperator,
    InfixOperator,
    Escape,
    Strcutural
>;


struct Token{
    TokenType type;
    uint32_t value; 
};


class Lexer{
public:

    explicit Lexer(std::string_view expression); 

    void next_token();

    inline Token get_current_token() const noexcept {return current_token_;};

    inline std::size_t  get_pos() const noexcept {return static_cast<std::size_t>(current_ - expression_.begin());};

    inline bool is_end() const noexcept {return current_ == expression_.cend();};

    inline std::string_view get_expression() const noexcept {return expression_;};


private:
    std::string_view expression_;
    // current 指向的是下一个 token 的首字节位置，如果已经完全解析完，则等于 expression 的尾后迭代器
    std::string_view::const_iterator current_;
    Token current_token_;

    // 对 *current 进行 tokonize
    void build_token();
    void build_escape_token();
};
