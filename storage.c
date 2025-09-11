#include<stdio.h>
#include<string.h>
#include"main.h"
int create_table(Createnode* node){
    catalog.table_count++;
    Table* table = malloc(sizeof(Table));
    table->table_name = node->table;
    table->col_count = node->col_count;
    table->cols = node->cols;
    catalog.tables = realloc(catalog.tables,catalog.table_count * sizeof(Table));
    catalog.tables[catalog.table_count-1] = table;
    save_catalog();
}
int select_data(Selectnode* node){

}
int insert_data(Insertnode* node){
    char* table_name = malloc(strlen(node->table)+strlen(".db") + 1);
    char* table_fsm_name = malloc(strlen(node->table)+strlen(".fsm") + 1);
    strcpy(table_name,node->table);
    strcpy(table_fsm_name,node->table);
    strcat(table_name,".db");
    strcat(table_fsm_name,".fsm");
    FreeSpaceMap* fsm = load_fsm(table_name);
    int byte_size = 0;
    for (int i = 0;i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            byte_size+= sizeof(node->cols[i]->int_value);
        }
        else{
            byte_size+= strlen(node->cols[i]->value) +1;
        }
    }
    for (int i =0;i < fsm->entry_count;i++){
        if (byte_size < fsm->entries[i].free_bytes){
            Page* page = load_page(table_name,fsm->entries[i].page_id);
            if (save_page(table_name,fsm->entries[i].page_id,page) == -1){
                return -1;
            }
        }
    }
}