#include<stdio.h>
#include<string.h>
#include "main.h"
Token* peek(Parser* parser){
    return parser->index < parser->length? &parser->tokens[parser->index]: NULL;
}
Token* advance(Parser* parser){
    return parser->index < parser->length? &parser->tokens[parser->index++]: NULL;
}
Wherenode* parse_where(Parser* parser){
    Wherenode* node = malloc(sizeof(Wherenode));
    Token* token = advance(parser);
    if (token->type != TOKEN_IDEN){
        return NULL;
    }
    node->column = token->data.value;
    token = advance(parser);
    switch(token->type){
        case TOKEN_Eq:
            node->op = OP_EQ;
            break;
        case TOKEN_Neq:
            node->op = OP_NEQ;
            break;
        case TOKEN_Gt:
            node->op = OP_GT;
            break;
        case TOKEN_GtEq:
            node->op = OP_GTEQ;
            break;
        case TOKEN_Lt:
            node->op = OP_LT;
            break;
        case TOKEN_LtEq:
            node->op = OP_LTEQ;
            break;
        default:
            return NULL;
    }
    token = advance(parser);
    if (token->type != TOKEN_STRING && token->type != TOKEN_NUMBER){
        return NULL;
    }
    node->value = token->data.value;
    if (token->type != TOKEN_STRING)
        node->type = NUMBER;
    else
        node->type= LITERAL;
    node->left = NULL;
    node->right = NULL;
    return node;
}
ASTnode* parse_select(Parser* parser){
    ASTnode* ast = malloc(sizeof(ASTnode));
    ast->token = parser->tokens[0];
    Token* token = advance(parser);
    ast->select.col_count =0;
    ast->select.cols=NULL;
    if (token->type != TOKEN_IDEN){
        return NULL;
    }
    if (strcmp(token->data.value,"*")==0){
        ast->select.cols = malloc(sizeof(char*));
        ast->select.cols[0] = "*";
        ast->select.col_count =1;
        token = advance(parser);
    }
    else {
        while(1){
            if (token->type!=TOKEN_IDEN){
                return NULL;
            }
            ast->select.col_count++;
            ast->select.cols=realloc(ast->select.cols,ast->select.col_count* sizeof(char*));
            ast->select.cols[ast->select.col_count-1] = token->data.value;
            token = advance(parser);
            if (token->type!=TOKEN_COMMA){
                break;
            }
            else{
                token = advance(parser);
            }
        }
    }
    if (token == NULL || token->type != TOKEN_KEYWORD || token->data.keyword != From){
        return NULL;
    }
    token = advance(parser);
    if (token == NULL || token->type != TOKEN_IDEN){
        return NULL;
    }
    ast->select.table = token->data.value;
    token = advance(parser);
    if (token->type == TOKEN_SEMI){
        ast->select.where = malloc(sizeof(Wherenode));
        ast->select.where->type = NONE;
        return ast;
    }
    else if (token->type != TOKEN_KEYWORD || token->data.keyword != Where){
        return NULL;
    }
    ast->select.where = parse_where(parser);
    if (ast->select.where== NULL){
        return NULL;
    }
    return ast;
}
ASTnode* parse_create(Parser* parser){
    ASTnode* ast = malloc(sizeof(ASTnode));
    ast->token = parser->tokens[0];
    Token* token = advance(parser);
    ast->create.col_count =0;
    ast->create.cols=NULL;
    if (token->type != TOKEN_KEYWORD || (token->data.keyword != Keyword_Table && token->data.keyword != Index)){
        return NULL;
    }
    if (token->data.keyword == Index){
        ast->token.data.keyword = Index;
        token = advance(parser);
        if (token->type != TOKEN_IDEN){
            return NULL;
        }
        ast->createindex.name = token->data.value;
        token = advance(parser);
        if (token->type != TOKEN_KEYWORD || token->data.keyword != On){
            return NULL;
        }
        token = advance(parser);
        if (token->type != TOKEN_IDEN){
            return NULL;
        }
        ast->createindex.table = token->data.value;
        token = advance(parser);
        if (token->type != TOKEN_LEFTPAREN){
            return NULL;
        }
        token = advance(parser);
        if (token->type != TOKEN_IDEN){
            return NULL;
        }
        ast->createindex.column = token->data.value;
        token = advance(parser);
        if (token != TOKEN_RIGHTPAREN){
            return NULL;
        }
        token = advance(parser);
        if (token != TOKEN_SEMI){
            return NULL;
        }
        return ast;
    }
    token = advance(parser);
    if (token == NULL || token->type != TOKEN_IDEN){
        return NULL;
    }
    ast->create.table = token->data.value;
    token = advance(parser);
    if (token->type != TOKEN_LEFTPAREN){
        return NULL;
    }
    while (token->type != TOKEN_RIGHTPAREN){
        token = advance(parser);
        Column* col = malloc(sizeof(Column));
        if (token->type != TOKEN_IDEN){
            return NULL;
        }
        col->name = token->data.value;
        token = advance(parser);
        if (token->type != TOKEN_KEYWORD || (token->data.keyword != Int && token->data.keyword != Varchar)){
            return NULL;
        }
        col->type = token->data.keyword;
        token = advance(parser);
        if (token->type == TOKEN_COMMA || token->type == TOKEN_RIGHTPAREN){
            col->index = 0;
        }
        else if (token->type != TOKEN_KEYWORD || token->data.keyword != PrimaryKey){
            return NULL;
        }
        col->index = 1;
        ast->create.col_count++;
        ast->create.cols = realloc(ast->create.cols,ast->create.col_count * sizeof(Column));
        ast->create.cols[ast->create.col_count-1] = *col;
        free(col);
    }
    return ast;
}
ASTnode* parse_insert(Parser* parser){
    ASTnode* ast = malloc(sizeof(ASTnode));
    ast->token = parser->tokens[0];
    Token* token = advance(parser);
    ast->insert.col_count=0;
    ast->insert.cols=NULL;
    if (token->type != TOKEN_KEYWORD || token->data.keyword != Into){
        return NULL;
    }
    token = advance(parser);
    if (token->type != TOKEN_IDEN){
        return NULL;
    }
    ast->insert.table = token->data.value;
    token = advance(parser);
    if (token->type != TOKEN_KEYWORD || token->data.keyword != Values){
        return NULL;
    }
    token = advance(parser);
    if (token->type != TOKEN_LEFTPAREN){
        return NULL;
    }
    while (token->type != TOKEN_RIGHTPAREN){
        token = advance(parser);
        ast->insert.col_count++;
        Exprnode* node = malloc(sizeof(Exprnode));
        if (token->type == TOKEN_STRING){
            node->type = LITERAL;
            node->value = token->data.value;
        }
        else if (token->type == TOKEN_NUMBER){
            node->type = NUMBER;
            node->int_value = atoi(token->data.value);
        }
        else {
            return NULL;
        }
        token = advance(parser);
        if (token->type != TOKEN_COMMA && token->type != TOKEN_RIGHTPAREN){
            return NULL;
        }
        ast->insert.cols = realloc(ast->insert.cols,ast->insert.col_count * sizeof(Exprnode*));
        ast->insert.cols[ast->insert.col_count-1] = node;
    }
    return ast;
}
ASTnode* AST(Parser* parser){
    if (parser->tokens[0].type!=TOKEN_KEYWORD){
        return NULL;
    }
    ASTnode* ast;
    Token* token = advance(parser);
    switch(token->data.keyword){
        case Select:
            ast = parse_select(parser);
            break;
        case Insert:
            ast = parse_insert(parser);
            break;
        case Create:
            ast = parse_create(parser);
            break;
    }
    return ast;
}