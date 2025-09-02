#include<stdio.h>
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
    
}