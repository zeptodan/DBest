#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "main.h"
const KeyToken keywords[] = {
    {.keyword=Select,.value="select"},
    {.keyword=Insert,.value="insert"},
    {.keyword=From,.value="from"},
    {.keyword=Create,.value="create"},
    {.keyword=Keyword_Table,.value="table"},
    {.keyword=Into,.value="into"},
    {.keyword=Where,.value="where"},
    {.keyword=Values,.value="values"},
    {.keyword=Int,.value="int"},
    {.keyword=Varchar,.value="varchar"},
    {.keyword=Index,.value="index"},
    {.keyword=On,.value="on"}
};
const size_t keyword_length = sizeof(keywords) / sizeof(keywords[0]);
Catalog catalog;
InputBuffer *new_input_buffer() {
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->input_length = 0;
    return input_buffer;
}
void read_input(InputBuffer *input_buffer) {
    fgets(input_buffer->input, sizeof(input_buffer->input), stdin);
    input_buffer->input_length = strlen(input_buffer->input)-1;
    input_buffer->input[input_buffer->input_length] = '\0'; // Null-terminate the string
}
void free_input_buffer(InputBuffer *input_buffer) {
    free(input_buffer);
}
void load_catalog(){
    FILE* file = fopen("catalog.meta","rb");
    if (!file){
        return;
    }
    fread(&catalog.table_count,sizeof(int),1,file);
    catalog.tables = malloc(catalog.table_count* sizeof(Table*));
    for (int i =0;i < catalog.table_count;i++){
        catalog.tables[i] = malloc(sizeof(Table));
        fread(&catalog.tables[i]->col_count,sizeof(int),1,file);
        size_t len;
        fread(&len,sizeof(size_t),1,file);
        catalog.tables[i]->table_name = malloc(len*sizeof(char));
        fread(catalog.tables[i]->table_name,sizeof(char),len,file);
        catalog.tables[i]->cols = malloc(catalog.tables[i]->col_count * sizeof(Column));
        for (int j = 0;j< catalog.tables[i]->col_count;j++){
            fread(&catalog.tables[i]->cols[j].index,sizeof(int),1,file);
            fread(&catalog.tables[i]->cols[j].type,sizeof(Keyword),1,file);
            size_t len2;
            fread(&len2,sizeof(size_t),1,file);
            catalog.tables[i]->cols[j].name = malloc(len2 * sizeof(char));
            fread(catalog.tables[i]->cols[j].name,sizeof(char),len2,file);
            fread(&len2,sizeof(size_t),1,file);
            catalog.tables[i]->cols[j].index_name = malloc(len2 * sizeof(char));
            fread(catalog.tables[i]->cols[j].index_name,sizeof(char),len2,file);
        }
    }
    fclose(file);
    //printf("tables: %i\nname: %s\ncols: %i\n%s\n%s\n",catalog.table_count,catalog.tables[0]->table_name,catalog.tables[0]->col_count,catalog.tables[0]->cols[0].name,catalog.tables[0]->cols[1].name);
    return;
}
void save_catalog(){
    FILE* file = fopen("catalog.meta","wb");
    if (!file){
        return;
    }
    fwrite(&catalog.table_count,sizeof(int),1,file);
    for (int i =0;i < catalog.table_count;i++){
        fwrite(&catalog.tables[i]->col_count,sizeof(int),1,file);
        size_t len = strlen(catalog.tables[i]->table_name)+1;
        fwrite(&len,sizeof(size_t),1,file);
        fwrite(catalog.tables[i]->table_name,sizeof(char),len,file);
        for (int j = 0;j< catalog.tables[i]->col_count;j++){
            fwrite(&catalog.tables[i]->cols[j].index,sizeof(int),1,file);
            fwrite(&catalog.tables[i]->cols[j].type,sizeof(Keyword),1,file);
            size_t len2  = strlen(catalog.tables[i]->cols[j].name)+1;
            fwrite(&len2,sizeof(size_t),1,file);
            fwrite(catalog.tables[i]->cols[j].name,sizeof(char),len2,file);
            len2  = strlen(catalog.tables[i]->cols[j].index_name)+1;
            fwrite(&len2,sizeof(size_t),1,file);
            fwrite(catalog.tables[i]->cols[j].index_name,sizeof(char),len2,file);
        }
    }
    fclose(file);
    return;
}
void display_catalog(){
    for(int i = 0;i < catalog.table_count;i++){
        printf("\ntable name: %s\ncol count: %i\n",catalog.tables[i]->table_name,catalog.tables[i]->col_count);
        for (int j =0;j < catalog.tables[i]->col_count;j++){
            printf(" %s type: %i\n",catalog.tables[i]->cols[j].name,catalog.tables[i]->cols[j].type);
        }
    }
}
int main(int argc, char *argv[]) {
    load_catalog();
    display_catalog();
    InputBuffer *input_buffer = new_input_buffer();
    while (true){
        printf("db >");
        read_input(input_buffer);
        //parser
        ASTnode* ast = parser(input_buffer);
        if (ast == NULL){
            printf("error in parser\n");
            continue;
        }
        Planner* plan = planner(ast);
        if (plan == NULL){
            printf("error in planner\n");
            continue;
        }
        executor(plan);
    }
    return 0;
}