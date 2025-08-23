#include<string.h>
#include<ctype.h>
#include "main.h"
void add(Statement* statement,TokenType type,Keyword keyword,char* value){
    if (statement->length==statement->capacity){
        statement->tokens=realloc(statement->tokens,2*statement->capacity*sizeof(Token));
        statement->capacity *=2;
    }
    Token token;
    if (type == TOKEN_KEYWORD){
        token = (Token){.type = type, .data.keyword = keyword};
    }
    else{
        token = (Token){.type = type, .data.value = value};
    }
    statement->tokens[statement->length] = token;
    statement->length++;
}
Statement* tokenizer(InputBuffer* buffer){
    int current = 0;
    Statement* statement = malloc(sizeof(Statement));
    statement->tokens = malloc(sizeof(Token));
    statement->length=0;
    statement->capacity=1;
    while (current < buffer->input_length){
        char character = buffer->input[current];
        if (character == '('){
            add(statement,TOKEN_LEFTPAREN,None,'(');
        }
        else if (character == ')'){
            add(statement,TOKEN_RIGHTPAREN,None,')');
        }
        else if (character == ','){
            add(statement,TOKEN_COMMA,None,',');
        }
        else if (character == ' '){
            add(statement,TOKEN_WHITESPACE,None,' ');
        }
        else if (character == '='){
            add(statement,TOKEN_Eq,None,'=');
        }
        else if (isdigit(character)){
            int index = current + 1;
            while(isdigit(buffer->input[index])){
                index++;
            }
            char* value = malloc(sizeof(char)*(index-current));
            strncpy(value,buffer->input+current,index-current);
            add(statement,TOKEN_NUMBER,None,value);
        }
        else if (isalpha(character)){
            int index = current + 1;
            while(isalpha(buffer->input[index])){
                index++;
            }
            char* value = malloc(sizeof(char)*(index-current));
            strncpy(value,buffer->input+current,index-current);
            for (int i =0; i<keyword_length;i++){
                if(value==keywords[i]){
                    
                }
            }
            add(statement,TOKEN_NUMBER,None,value);
        }
    }
    return statement;
}