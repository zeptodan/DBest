#include<stdio.h>
#include"main.h"
Planner* planner(ASTnode* ast){
    Planner* planner = malloc(sizeof(Planner));
    switch (ast->token.data.keyword){
        case Select:
            planner->type = SEQ_PLAN;
            planner->select = ast->select;
        case Insert:
            planner->type = INSERT_PLAN;
            planner->insert = ast->insert;
        case Create:
            planner->type = CREATE_PLAN;
            planner->create = ast->create;
    }
    return planner;
}