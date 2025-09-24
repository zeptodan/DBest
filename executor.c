#include<stdio.h>
#include"main.h"
int executor(Planner* planner){
    switch(planner->type){
        case SEQ_PLAN:
        select_data(&planner->select);
        break;
        case INDEX_PLAN:
        break;
        case CREATE_PLAN:
        create_table(&planner->create);
        break;
        case CREATE_INDEX_PLAN:
        create_index(&planner->createindex);
        break;
        case INSERT_PLAN:
        insert_data(&planner->insert);
        break;
    }
}