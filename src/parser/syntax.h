#include "lexer/lexer.h"
#include "data_structures/ast.h"
#include "error.h"


typedef struct {
    LexerPtr lexer;
    Token current_token;
    Token peek_token;
} Parser, *ParserPtr;

StatusEnum parserCtor(ParserPtr parser, LexerPtr lexer);

StatusEnum analyze(ParserPtr parser, ASTNodePtr ast_root);

void parserDtor(ParserPtr parser);

void parserReset(ParserPtr parser);