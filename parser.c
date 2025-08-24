#include<stdio.h>
#include "main.h"
int parser(InputBuffer* buffer) {
    Statement* statement = tokenizer(buffer);
    for (int i = 0;i < statement->length;i++){
        printf("type: %d ",statement->tokens[i].type);
        if (statement->tokens[i].type==TOKEN_KEYWORD){
            printf("keyword: %d\n",statement->tokens[i].data.keyword);
        }
        else{
            printf("value: %s\n",statement->tokens[i].data.value);
        }
    }
    return 0;
}