#include<stdio.h>
#include "string.h"
#include"main.h"
Planner* planner(ASTnode* ast){
    Planner* planner = malloc(sizeof(Planner));
    switch (ast->token.data.keyword){
        case Select:
            for (int i =0;i < catalog.table_count;i++){
                if (strcmp(catalog.tables[i]->table_name,ast->select.table) ==0){
                    int exists =0;
                    if (strcmp(ast->select.cols[0],"*")!=0){
                        for (int j =0;j< ast->select.col_count ;i++){
                            for (int k = 0;k < catalog.tables[i]->col_count;k++){
                                if(strcmp(catalog.tables[i]->cols[k].name,ast->select.cols[j])==0){
                                    exists=1;
                                    break;
                                }
                            }
                            if (exists==0){
                                return NULL;
                            }
                        }
                    }
                    exists= 0;
                    if (ast->select.where->type != NONE){
                        for (int j =0; j < catalog.tables[i]->col_count;j++){
                            if (strcmp(catalog.tables[i]->cols[j].name,ast->select.where->column)==0){
                                if (catalog.tables[i]->cols[j].type == Int && ast->select.where->type != NUMBER)
                                    return NULL;
                                else if (catalog.tables[i]->cols[j].type == Varchar && ast->select.where->type != LITERAL)
                                    return NULL;
                                exists = 1;
                                break;
                            }
                        }
                        if (exists == 0){
                            return NULL;
                        }
                    }
                    break;
                }
                if (i == catalog.table_count -1){
                    return NULL;
                }
            }
            planner->type = SEQ_PLAN;
            planner->select = ast->select;
            break;
        case Insert:
            for (int i =0;i < catalog.table_count;i++){
                if (strcmp(catalog.tables[i]->table_name,ast->insert.table)==0){
                    if (ast->insert.col_count != catalog.tables[i]->col_count){
                        printf("expected %i values",catalog.tables[i]->col_count);
                        return NULL;
                    }
                    for (int j = 0;j < ast->insert.col_count;j++){
                        if (ast->insert.cols[j]->type == NUMBER && catalog.tables[i]->cols[j].type != Int){
                            printf("type is %i\n",catalog.tables[i]->cols->type);
                            printf("argument number %i has incorrect type. correct type is not int\n",j);
                            return NULL;
                        }
                        else if (ast->insert.cols[j]->type == LITERAL && catalog.tables[i]->cols[j].type != Varchar){
                            printf("argument number %i has incorrect type. correct type is not varchar\n",j);
                            return NULL;
                        }
                        else if (ast->insert.cols[j]->type != LITERAL && ast->insert.cols[j]->type != NUMBER){
                            printf("argument number %i has incorrect type\n",j);
                            return NULL;
                        }
                    }
                    break;
                }
                if (i == catalog.table_count -1){
                    printf("no such table\n");
                    return NULL;
                }
            }
            planner->type = INSERT_PLAN;
            planner->insert = ast->insert;
            break;
        case Create:
            planner->type = CREATE_PLAN;
            planner->create = ast->create;
            break;
    }
    return planner;
}