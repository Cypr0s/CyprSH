/**
 * @file        syntax.h
 * @author      Kristian Luptak <kristian.luptak@outlook.com>
 * @version     1.0.1
 * @date        2026-06-29
 * @copyright   Copyright (c) 2026
 * 
 * @brief   Syntax analyzer (parser) for shell grammar
 */

#ifndef SYNTAX_H
#define SYNTAX_H

#include "lexer/lexer.h"
#include "data_structures/ast.h"
#include "error.h"


typedef struct {
    LexerPtr lexer;
    Token current_token;
    Token peek_token;
} Parser, *ParserPtr;

/** @brief Initialize parser with lexer
 *  @param parser Parser to initialize
 *  @param lexer Source lexer
 *  @return Status code
 */
StatusEnum parserCtor(ParserPtr parser, LexerPtr lexer);

/** @brief Perform syntax analysis and build AST
 *  @param parser Source parser
 *  @param ast_root Root node for AST
 *  @return Status code
 */
StatusEnum analyze(ParserPtr parser, ASTNodePtr ast_root);

/** @brief Destroy parser and free resources
 *  @param parser Parser to destroy
 */
void parserDtor(ParserPtr parser);

/** @brief Reset parser for new input
 *  @param parser Parser to reset
 */
void parserReset(ParserPtr parser);

#endif // SYNTAX_H