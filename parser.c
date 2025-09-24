#include<stdio.h>
#include "main.h"
ASTnode* parser(InputBuffer* buffer) {
    Statement* statement = tokenizer(buffer);
    if (statement->length == -1){
        return NULL;
    }
    // for (int i = 0;i < statement->length;i++){
    //     printf("type: %d ",statement->tokens[i].type);
    //     if (statement->tokens[i].type==TOKEN_KEYWORD){
    //         printf("keyword: %d\n",statement->tokens[i].data.keyword);
    //     }
    //     else{
    //         printf("value: %s\n",statement->tokens[i].data.value);
    //     }
    // }
    Parser* parser = malloc(sizeof(Parser));
    parser->length = statement->length;
    parser->index = 0;
    parser->tokens = statement->tokens;
    free(statement);
    ASTnode* ast = AST(parser);
    free(parser->tokens);
    free(parser);
    // if (ast->token.data.keyword == Select){
    //     printf("col count: %i\ntable: %s\ncol: %s\nwhere: %p\n",ast->select.col_count,ast->select.table,ast->select.cols[0],ast->select.where);

    // }
    // else if (ast->token.data.keyword == Create)
    // {
    //     printf("col count: %i\ntable: %s\ncol name: %scol type: %i\n",ast->create.col_count,ast->create.table,ast->create.cols[0].name,ast->create.cols[0].type);
    // }
    // else if (ast->token.data.keyword == Insert)
    // {
    //     printf("col count: %i\ntable: %s\ncol: %i\n",ast->insert.col_count,ast->insert.table,ast->insert.cols[1]->type);
    // }
    return ast;
}