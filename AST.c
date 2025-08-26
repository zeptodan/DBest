#include<stdio.h>
#include<string.h>
#include "main.h"
Token* peek(Parser* parser){
    return parser->index < parser->length? &parser->tokens[parser->index]: NULL;
}
Token* advance(Parser* parser){
    return parser->index < parser->length? &parser->tokens[parser->index++]: NULL;
}
Exprnode* parse_where(Parser* parser){
    Exprnode* expr;
    return expr;
}
ASTnode* parse_select(Parser* parser){
    ASTnode* ast = malloc(sizeof(ASTnode));
    ast->token = parser->tokens[0];
    Token* token = advance(parser);
    ast->select.col_count =0;
    ast->select.cols=NULL;
    if (token->type != IDEN){
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
            if (token->type!=IDEN){
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
    if (token == NULL || token->type != IDEN){
        return NULL;
    }
    ast->select.table = token->data.value;
    token = advance(parser);
    if (token->type == TOKEN_SEMI){
        return ast;
    }
    else if (token->type != TOKEN_KEYWORD || token->data.keyword != Where){
        return NULL;
    }
    ast->select.where = malloc(sizeof(Wherenode));
    ast->select.where->exprnode = malloc(sizeof(Exprnode));
    ast->select.where->exprnode = parse_where(parser);
    return ast;
}
ASTnode* parse_create(Parser* parser){
    ASTnode* ast;
    return ast;
}
ASTnode* parse_insert(Parser* parser){
    ASTnode* ast;
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
            //ast = parse_select(parser);
            break;
        case Create:
            ast = parse_create(parser);
            break;
    }
    return ast;
}