#include<stdio.h>
#include"main.h"
int executor(Planner* planner){
    switch(planner->type){
        case SEQ_PLAN:
        case INDEX_PLAN:
        case CREATE_PLAN:
        create_table(&planner->create);
        break;
        case INSERT_PLAN:
    }
}