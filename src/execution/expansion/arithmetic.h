/* ==================== IFJ25 COMPILER ==================== */
/**
 *@headerfile   precedence.h
 *@author       Kristian Luptak <xluptak00>
 *@date         Created at: 17.10.2025
 *              Updated at: 23.11.2025
 *@brief        declaration of macros for parser, functions 
 *              for precedence parsing
 */
/* ======================================================== */

#ifndef PREC_PRECEDENCE_H
#define PREC_PRECEDENCE_H

// moves to next token if token is EOL
#define CHECK_EOL do { \
    if((scanner->current_token.type) == TOKEN_EOL) { \
        GET_TOKEN(scanner); \
    } \
} while(0)

// moves to next token, if TOKEN_ERROR is found, return the corresponding error
#define GET_TOKEN(scanner) do { \
    get_token(scanner); \
    if((((scanner)->current_token).type) == TOKEN_ERROR) { \
        if(strcmp((((scanner)->current_token).value), "1") == 0) { \
            return ERROR_LEXICAL; \
        } \
        return ERROR_INTERNAL; \
    } \
} while(0) 


#define CHECK_ERR(a) do { \
    if(a) { \
        return a; \
    } \
} while(0)

// enum for all the rules of precedence parsing
typedef enum {
    RULE_ADD,   // E -> E + E
    RULE_SUB,   // E -> E - E
    RULE_MUL,   // E -> E * E
    RULE_DIV,   // E -> E / E
    RULE_OR,    // E -> E || E
    RULE_AND,   // E -> E && E
    RULE_NOT,   // E -> ! E
    RULE_IS,    // E -> E is E <- here the E must be one of Num | Null | String | Bool
    RULE_ITE,   // E -> E ? E : E
    RULE_EQ,    // E -> E == E
    RULE_NEQ,   // E -> E != E
    RULE_GEQ,   // E -> E >= E
    RULE_LEQ,   // E -> E <= E
    RULE_LOW,   // E -> E < E
    RULE_GRE,   // E -> E > E
    RULE_NEG,   // E -> - E
    RULE_BRA,   // E -> ( E )
    RULE_I,     // E -> i
    RULE_INVALID // invalid rule
} RULE_TYPE;

/* magical constant which holds the maximum size of array in which stack will pop
    operands and operators based on which rule will be created */
# define RULE_MAX_SIZE 5 

// enum for all the operatios of precedence parsing
typedef enum {
    SHIFT,              // <
    REDUCE,             // >
    EQ,                 // ==
    ERROR               // '  '
} ACTION_TYPE;


RETURN_TYPE parse_expression(ASTNodePtr node, Scanner_ptr scanner);

OPERATOR get_operator_from_token(Token_ptr current_token);

int get_table_index_from_op(OPERATOR op);

RETURN_TYPE reduce_expression(PrecStackPtr op_stack, NodeStackPtr node_stack);

RETURN_TYPE reduce_tree(NodeStackPtr node_stack, char* operation, NodeType type, int count);

RETURN_TYPE shift_expression(PrecStackPtr op_stack, 
                                    NodeStackPtr node_stack, 
                                    Scanner_ptr scanner);

RULE_TYPE get_rule(OPERATOR* arr, char size);

StatusEnum expandArithmetic(ExpanderPtr exp);

#endif