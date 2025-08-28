#include<stdio.h>
#include "main.h"
ASTnode* parser(InputBuffer* buffer) {
    Statement* statement = tokenizer(buffer);
    if (statement->length == -1){
        return NULL;
    }
    for (int i = 0;i < statement->length;i++){
        printf("type: %d ",statement->tokens[i].type);
        if (statement->tokens[i].type==TOKEN_KEYWORD){
            printf("keyword: %d\n",statement->tokens[i].data.keyword);
        }
        else{
            printf("value: %s\n",statement->tokens[i].data.value);
        }
    }
    Parser* parser = malloc(sizeof(Parser));
    parser->length = statement->length;
    parser->index = 0;
    parser->tokens = statement->tokens;
    free(statement);
    ASTnode* ast = AST(parser);
    free(parser->tokens);
    free(parser);
    if (ast == NULL){
        printf("not good");
        return 0;
    }
    printf("type: %i\nkeyword: %i\ncol count: %i\ntable: %s\ncol: %s\nwhere: %p\n",ast->token.type,ast->token.data.keyword,ast->select.col_count,ast->select.table,ast->select.cols[0],ast->select.where);
    return ast;
}