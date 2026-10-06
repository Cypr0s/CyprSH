/* ==================== IFJ25 COMPILER ==================== */
/**
 *@file     precedence.c  
 *@author   Kristian Luptak <xluptak00>
 *@date     Created at: 17.10.2025
 *          Updated at: 23.11.2025
 *@brief    implementation of expression parsing using 
 *          precedence table
 */
/* ======================================================== */

#include "precedence.h"

/** 
 * @brief               converts current_token TOKEN type to its corresponding operator type
 * @param current_token token to be converted
 * @return              OPERATOR(int) number
 */
OPERATOR get_operator_from_token(Token_ptr current_token) {
    switch(current_token->type) {
        case TOKEN_PLUS:
            return OP_PLUS;
        case TOKEN_MINUS:
            return OP_MINUS;
        case TOKEN_MUL:
            return OP_MUL;
        case TOKEN_DIV:
            return OP_DIV;
        case TOKEN_EQUAL:
            return OP_EQ;
        case TOKEN_NOT_EQUAL:
            return OP_NEQ;
        case TOKEN_LESSER_EQUAL:
            return OP_LEQ;
        case TOKEN_GREATER_EQUAL:
            return OP_GEQ;
        case TOKEN_GREATER:
            return OP_GRE;
        case TOKEN_LESSER:
            return OP_LOW;
        case TOKEN_EXCL_MARK:
            return OP_NOT;
        case TOKEN_OR:
            return OP_OR;
        case TOKEN_AND:
            return OP_AND;
        case TOKEN_QUESTION_MARK:
            return OP_QUE;
        case TOKEN_COLON:
            return OP_COL;
        case TOKEN_BRACKET_OP:
            return OP_BRACKET_OP;
        case TOKEN_BRACKET_CL:
            return OP_BRACKET_CL;
        case TOKEN_IS:
            return OP_IS;

        case TOKEN_ID:
        case TOKEN_STR_LIT:
        case TOKEN_DEC_LIT:
        case TOKEN_INT_LIT:
        case TOKEN_NULL_TYPE:
        case TOKEN_TRUE:
        case TOKEN_FALSE:
        case TOKEN_BOOL:
        case TOKEN_STRING:
        case TOKEN_NUM:
        case TOKEN_NULL:
            return OP_I;
        default:
            return OP_$;
    }
}

/** 
 * @brief               gets index of the precedence table based on input operator
 * @param op            operator from which well get the index
 * @return              int
 */
int get_table_index_from_op(OPERATOR op) {
    switch(op) {
        case OP_PLUS:
        case OP_MINUS:
            return 0;

        case OP_MUL:
        case OP_DIV:
            return 1;

        case OP_EQ:
        case OP_NEQ:
            return 2;
    
        case OP_LEQ:
        case OP_GEQ:
        case OP_GRE:
        case OP_LOW:
            return 3;

        case OP_NOT:
            return 4;

        case OP_OR:
            return 5;

        case OP_AND:
            return 6;

        case OP_QUE:
        case OP_COL:
            return 7;

        case OP_BRACKET_OP:
            return 8;
        
        case OP_BRACKET_CL:
            return 9;
        
        case OP_IS:
            return 10;

        case OP_I:
            return 11;
        case OP_NEG:
            return 12;
        default:
            return 13;
    }
}


/** 
 * @brief           parse expression based on precedence table, shifts rules and reduces
 * @param node      node in which the created expresion node will be inserted
 * @param scanner   scanner structure which holds current token and next token (for peek_token())
 * @return          0 valid syntax, 1 lexical error, 2 syntax error, 99 internal error  
 */
RETURN_TYPE parse_expression(ASTNodePtr node, Scanner_ptr scanner) {
    // stack for created subtrees or operands, operators
    NodeStack node_stack;
    PrecStack op_stack;
    // init both stacks
    node_stack_init(&node_stack);
    prec_stack_init(&op_stack);


    if(prec_stack_push(&op_stack, OP_$)){
        return ERROR_INTERNAL;
    }
    RETURN_TYPE err;
    OPERATOR stack_op;
    OPERATOR input_op;

    // precedence table based on which we decide if we shift or reduce
    const ACTION_TYPE actions_table[14][14] = {
    //             +,- |  *,/  | ==,!= | cmp   |   !  |  '||' |  &&   |   ?:  |   (  |   )   |   is  |   i  |  neg |   $
    /*  +,-  */ {REDUCE, SHIFT , REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /*  *,/  */ {REDUCE, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /* ==,!= */ {SHIFT , SHIFT , REDUCE, SHIFT , SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, SHIFT , SHIFT, SHIFT, REDUCE},
    /*  cmp  */ {SHIFT , SHIFT , REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /*   !   */ {REDUCE, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /*   ||  */ {SHIFT , SHIFT , SHIFT , SHIFT , SHIFT, REDUCE, SHIFT , REDUCE, SHIFT, REDUCE, SHIFT , SHIFT, SHIFT, REDUCE},
    /*   &&  */ {SHIFT , SHIFT , SHIFT , SHIFT , SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, SHIFT , SHIFT, SHIFT, REDUCE},
    /*   ?:  */ {SHIFT , SHIFT , SHIFT , SHIFT , SHIFT, SHIFT , SHIFT , SHIFT , SHIFT, REDUCE, SHIFT , SHIFT, SHIFT, REDUCE},
    /*   (   */ {SHIFT , SHIFT , SHIFT , SHIFT , SHIFT, SHIFT , SHIFT , SHIFT , SHIFT,   EQ  , SHIFT , SHIFT, SHIFT, ERROR },
    /*   )   */ {REDUCE, REDUCE, REDUCE, REDUCE,REDUCE, REDUCE, REDUCE, REDUCE, ERROR, REDUCE, REDUCE, SHIFT,REDUCE, REDUCE},
    /*   is  */ {SHIFT , SHIFT , REDUCE, SHIFT , SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /*   i   */ {REDUCE, REDUCE, REDUCE, REDUCE, ERROR, REDUCE, REDUCE, REDUCE, ERROR, REDUCE, REDUCE, ERROR, ERROR, REDUCE},
    /*  neg  */ {REDUCE, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, REDUCE, SHIFT, REDUCE, REDUCE, SHIFT, SHIFT, REDUCE},
    /*   $   */ {SHIFT , SHIFT , SHIFT , SHIFT , SHIFT, SHIFT , SHIFT , SHIFT , SHIFT, ERROR , SHIFT , SHIFT, SHIFT, ERROR }
    };

    // parse the expression
    do {
        // get operators from stack
        stack_op = prec_stack_top_terminal(&op_stack);
        if(stack_op == OP_INVALID) return ERROR_INTERNAL;
        stack_op = get_table_index_from_op(stack_op);

        // handle unary -
        if(scanner->current_token.type == TOKEN_MINUS &&
          (prec_stack_top(&op_stack) != OP_I && 
           prec_stack_top(&op_stack) != NON_TERMINAL && 
           prec_stack_top(&op_stack) != OP_BRACKET_CL)) {
            input_op = OP_NEG;
        }
        // get operator from token
        else {
            input_op = get_operator_from_token(&(scanner->current_token));
        }

        // condition that expression has ended
        if(input_op == OP_BRACKET_CL && 
          (prec_stack_count_of(&op_stack, OP_BRACKET_OP) - 
           prec_stack_count_of(&op_stack, OP_BRACKET_CL) == 0)) {
            input_op = OP_$;
        }  

        if(scanner->current_token.type == TOKEN_ERROR) {
            prec_stack_destroy(&op_stack);
            node_stack_destroy(&node_stack);
            if(strcmp(scanner->current_token.value, "1") == 0) {
                return ERROR_LEXICAL;
            }
            return ERROR_INTERNAL;
        }

        switch(actions_table[stack_op][get_table_index_from_op(input_op)]){
            // shifting input token 
            case SHIFT:
                err = shift_expression(&op_stack, &node_stack, scanner);
                if(err) {
                    prec_stack_destroy(&op_stack);
                    node_stack_destroy(&node_stack);
                    return err;
                }
                break;
            // equal tokens on top of the stack and in input
            case EQ:
                err = prec_stack_push(&op_stack, input_op);
                if(err) {
                    prec_stack_destroy(&op_stack);
                    node_stack_destroy(&node_stack);
                    return err;
                }
                // get new token
                get_token(scanner);
                if(scanner->current_token.type == TOKEN_ERROR) {
                    prec_stack_destroy(&op_stack);
                    node_stack_destroy(&node_stack);
                    if(strcmp(scanner->current_token.value, "1") == 0) {
                        return ERROR_LEXICAL;
                    }
                    return ERROR_INTERNAL;
                }
                break;

            // reducing expression
            case REDUCE:
                err = reduce_expression(&op_stack, &node_stack);
                if(err) {
                    prec_stack_destroy(&op_stack);
                    node_stack_destroy(&node_stack);
                    return err;
                }
                break;
            // invalid syntax of tokens
            case ERROR:

                prec_stack_destroy(&op_stack);
                node_stack_destroy(&node_stack);
                return ERROR_SYNTAX;
        }
    } while(input_op != OP_$ || prec_stack_size(&op_stack) != 2);
    // parsing successful create expression node, insert created expression into it
    ASTNodePtr created_expression = node_stack_pop(&node_stack);
    if(created_expression == NULL) {
        prec_stack_destroy(&op_stack);
        node_stack_destroy(&node_stack);
        return ERROR_INTERNAL;
    }

    ASTNodePtr expression_node = create_node(EXPRESSION_NODE, NULL, NULL);
    if(expression_node == NULL) {
        free_tree(created_expression);
        prec_stack_destroy(&op_stack);
        node_stack_destroy(&node_stack);
        return ERROR_INTERNAL;
    }
    // insert into expression node
    if(insert_node(expression_node, created_expression)) {
        free_tree(created_expression);
        prec_stack_destroy(&op_stack);
        node_stack_destroy(&node_stack);
        return ERROR_INTERNAL;
    }
    // insert into input node
    if(insert_node(node, expression_node)) {
        free_tree(expression_node);
        prec_stack_destroy(&op_stack);
        node_stack_destroy(&node_stack);
        return ERROR_INTERNAL;
    }


    prec_stack_destroy(&op_stack);
    node_stack_destroy(&node_stack);
    return RETURN_OK;
}


/** 
 * @brief               reduces the expression (gets the rule base on operator, creates a new node)
 * @param op_stack      stack which may hold all the things defined in precedence table based on 
 *                      the current expression
 * @param node_stack    stack which holds currently created nodes some of which will be merged into
 *                      one
 * @return              0 valid syntax, 2 syntax error, 99 internal error  
 */
RETURN_TYPE reduce_expression(PrecStackPtr op_stack, NodeStackPtr node_stack) {
    OPERATOR arr[RULE_MAX_SIZE]; // longest rule is E ? E : E 
    OPERATOR op;

    // pop operators
    char size = 0;
    for(int i = 0; i < 6; i++, size++) {
        op = prec_stack_pop(op_stack);
        if(op == OP_INVALID) {
            return ERROR_SYNTAX;
        }
        if(op == NON_OP_STOP) break;
        arr[i] = op;
    }
    // get rule
    RULE_TYPE rule = get_rule(arr, size);

    RETURN_TYPE err = 0;
    switch(rule) {
        // replace "i" with E
        case RULE_I:
            break;
        // just replace ( E ) with E
        case RULE_BRA:
            break;
        // unary
        case RULE_NOT:
            err = reduce_tree(node_stack, "!", UNARY_OPERATOR_NODE, 1);
            break;
        case RULE_NEG:
            err = reduce_tree(node_stack, "-", UNARY_OPERATOR_NODE, 1);
            break;
        // binary
        case RULE_ADD:
            err = reduce_tree(node_stack, "+", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_SUB:
            err = reduce_tree(node_stack, "-", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_MUL:
            err = reduce_tree(node_stack, "*", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_DIV:
            err = reduce_tree(node_stack, "/", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_OR:
            err = reduce_tree(node_stack, "||", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_AND:
            err = reduce_tree(node_stack, "&&", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_EQ:
            err = reduce_tree(node_stack, "==", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_NEQ:
            err = reduce_tree(node_stack, "!=", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_GEQ:
            err = reduce_tree(node_stack, ">=", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_LEQ:
            err = reduce_tree(node_stack, "<=", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_LOW:
            err = reduce_tree(node_stack, "<", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_GRE:
            err = reduce_tree(node_stack, ">", BINARY_OPERATOR_NODE, 2);
            break;
        case RULE_IS:
            err = reduce_tree(node_stack, "is", BINARY_OPERATOR_NODE, 2);
            break;
        // ternary
        case RULE_ITE:
            err = reduce_tree(node_stack, "?:", TERNARY_OPERATOR_NODE, 3);
            break;

        case RULE_INVALID:
            return ERROR_SYNTAX;
        default:
            return ERROR_SYNTAX;
    }
    CHECK_ERR(err);
    // push the EXPR
    if(prec_stack_push(op_stack, NON_TERMINAL)) {
        return ERROR_INTERNAL;
    }
    return RETURN_OK;
}


/** 
 * @brief               helper function which reduces the node_stack (merges multiple nodes)
 * @param node_stack    stack which contains the subtrees which will be merged
 * @param operation     operation (eg + - * / )
 * @param type          operation type eg binary unary ternaty(how many operands it needs)
 * @param count         count of operands
 * @return              0 valid syntax, 99 internal error  
 */
RETURN_TYPE reduce_tree(NodeStackPtr node_stack, char* operation, NodeType type, int count){
    // create new_subtree(which type is operator)
    ASTNodePtr new_subtree = create_node(type, NULL, operation);
    if(new_subtree == NULL) {
        return ERROR_INTERNAL;
    }

    // pop nodes
    ASTNodePtr operands[count];
    for(int i = count - 1; i >= 0; i--) {
        operands[i] = node_stack_pop(node_stack);
        if(operands[i] == NULL) {
            free(new_subtree);
            for(int j = i + 1; j < count; j++) {
                free_tree(operands[j]);
            }
            return ERROR_INTERNAL;
        }
    }
    // insert nodes
    for(int i = 0; i < count; i++) {
        if(insert_node(new_subtree, operands[i])) {
            free_tree(new_subtree);
            for(int j = i; j < count; j++) {
                free_tree(operands[j]);
            }
            return ERROR_INTERNAL;
        }
    
    }
    // push back onto node stack
    if(node_stack_push(node_stack, new_subtree)) {
        free_tree(new_subtree);
        return ERROR_INTERNAL;
    }
    return RETURN_OK;
}

/** 
 * @brief               shifts current token on top of the stack, creating a node on top of the
 *                      node stack if its an operand, advances to next token
 * @param op_stack      stack that contains the operators and operands
 * @param node_stack    stack that constains subtrees or nodes
 * @param scanner       scanner holds current token
 * @return              0 valid syntax, 99 internal error  
 */
RETURN_TYPE shift_expression(PrecStackPtr op_stack, 
                             NodeStackPtr node_stack, 
                             Scanner_ptr scanner) {
    ASTNodePtr new_node;
    RETURN_TYPE err;
    int is_term = 0;
    // creating new node if its an operand
    switch(scanner->current_token.type) {
        case TOKEN_STR_LIT:
            new_node = create_node(STRING_LIT_NODE, scanner->current_token.value, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_ID:
            new_node = create_node(ID_NODE, scanner->current_token.value, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_DEC_LIT:
            new_node = create_node(FLOAT_LIT_NODE, scanner->current_token.value, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_TRUE:
            new_node = create_node(BOOL_LIT_NODE, "true", NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_FALSE:
            new_node = create_node(BOOL_LIT_NODE, "false", NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_NULL_TYPE:
            new_node = create_node(NULL_TYPE_NODE, NULL, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_INT_LIT:
            new_node = create_node(INT_LIT_NODE, scanner->current_token.value, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_BOOL:
            new_node = create_node(BOOL_TYPE_NODE, NULL, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            break;
        case TOKEN_STRING:
            new_node = create_node(STRING_TYPE_NODE, NULL, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_NUM:
            new_node = create_node(NUM_TYPE_NODE, NULL, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        case TOKEN_NULL:
            new_node = create_node(NULL_LIT_NODE, scanner->current_token.value, NULL);
            if(new_node == NULL) return ERROR_INTERNAL;
            is_term = 1;
            break;
        default:
            break;
    }
    // if its term operand(i) push it onto node stack 
    if(is_term) {
        if(new_node == NULL) {
            return ERROR_INTERNAL;
        }
        err = node_stack_push(node_stack, new_node);
        if(err) {
            free(new_node);
            return err;
        }
    }
    OPERATOR stack_push;
    // handle negation
    if(scanner->current_token.type == TOKEN_MINUS && 
      (prec_stack_top(op_stack) != OP_I &&
       prec_stack_top(op_stack) != NON_TERMINAL &&
       prec_stack_top(op_stack) != OP_BRACKET_CL)) {
        stack_push = OP_NEG;
    }
    else {
        stack_push = get_operator_from_token(&(scanner->current_token));
    }
    

    // colon : does not a get a < before it (its not needed) its a continuation of '?'
    if(scanner->current_token.type != TOKEN_COLON) {
        err = prec_insert_before_terminal(op_stack, NON_OP_STOP);
        CHECK_ERR(err);
    }
    // push operator
    err = prec_stack_push(op_stack, stack_push);
    CHECK_ERR(err);

    GET_TOKEN(scanner);
    // newline after operator
    if(scanner->current_token.type == TOKEN_EOL &&
      (prec_stack_top_terminal(op_stack) != OP_I &&
       prec_stack_top_terminal(op_stack) != OP_BRACKET_OP &&
        prec_stack_top_terminal(op_stack) != OP_BRACKET_CL)) {
        GET_TOKEN(scanner);
    }
    return RETURN_OK;
}


/** 
 * @brief       returns rule by which node stack should be reduced based on operator which is
 *              stored in arr
 * @param arr   arr that contains the operands and operators based on which the rule 
 *              will be created
 * @param size  size of the arr
 */
RULE_TYPE get_rule(OPERATOR* arr, char size) {
    // E -> i
    if(size == 1) {
        if(arr[0] == OP_I) {
            return RULE_I;
        }
    }
    // unary operators
    else if(size == 2) {
        if(arr[0] != NON_TERMINAL) {
            return RULE_INVALID;
        }
        if(arr[1] == OP_NEG) {
            return RULE_NEG;
        }
        else if(arr[1] == OP_NOT) {
            return RULE_NOT;
        }
        else {
            return RULE_INVALID;
        }
        
    }
    // binary operators
    else if(size == 3) {
        if(arr[0] == OP_BRACKET_CL && arr[1] == NON_TERMINAL && arr[2] == OP_BRACKET_OP) {
            return RULE_BRA;
        }
        if(arr[0] != NON_TERMINAL || arr[2] != NON_TERMINAL) {
            return RULE_INVALID;
        }
        switch(arr[1]) {
            case OP_PLUS:
                return RULE_ADD;
            case OP_MINUS:
                return RULE_SUB;
            case OP_MUL:
                return RULE_MUL;
            case OP_DIV:
                return RULE_DIV;
            case OP_AND:
                return RULE_AND;
            case OP_OR:
                return RULE_OR;
            case OP_GEQ:
                return RULE_GEQ;
            case OP_LEQ:
                return RULE_LEQ;
            case OP_LOW:
                return RULE_LOW;
            case OP_GRE:
                return RULE_GRE;
            case OP_IS:
                return RULE_IS;
            case OP_EQ:
                return RULE_EQ;
            case OP_NEQ:
                return RULE_NEQ;
            default:
                return RULE_INVALID;
        }
    }
    // ternary operator
    else if(size == 5) {
        return (arr[0] == NON_TERMINAL && arr[3] == OP_QUE && 
                arr[2] == NON_TERMINAL && arr[1] == OP_COL &&
                arr[4] == NON_TERMINAL) ? RULE_ITE : RULE_INVALID;
    }
    return RULE_INVALID;
}


StatusEnum expandArithmetic(ExpanderPtr exp) {
    (void)exp;
    // TODO: arithmetic expansion $((expr)) not implemented yet
    return ERROR_DEFAULT;
}