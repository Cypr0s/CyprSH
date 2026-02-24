#include "../utils/error.h"

typedef enum {

} FSMStates;


typedef enum {
    TOKEN_WORD,         //
    TOKEN_NEWLINE,      // \n
    TOKEN_IO_NUM,       // number representing a file descriptor
    TOKEN_EOF,

    // control operators
    TOKEN_SEMI,         // ;
    TOKEN_PIPE,         // |
    TOKEN_AND_IF,       // &&
    TOKEN_OR_IF,        // ||
    TOKEN_DOUBLE_SEMI,  // ;;
    TOKEN_CLOBBER,      // >|
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_BANG,         // !
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_BG,           // &

    // redirectors
    TOKEN_LESS,         // <
    TOKEN_GREAT,        // >
    TOKEN_DLESS,        // <<
    TOKEN_DGREAT,       // >>
    TOKEN_LESSAND,      // <&
    TOKEN_GREATAND,     // >&
    TOKEN_LESSGREAT,    // <>
    TOKEN_DLESSDASH,    // <<-
    TOKEN_TLESS,        // <<<
    
    // special
    TOKEN_ERROR,
}TokenTypeEnum; 

typedef struct token {
    TokenTypeEnum type;
    char* value;

} Token, TokenPtr;