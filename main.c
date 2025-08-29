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
    {.keyword=Values,.value="values"}
};
const size_t keyword_length = sizeof(keywords) / sizeof(keywords[0]);
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
    fread(&catalog,sizeof(Catalog),1,file);
    fclose(file);
    return;
}
void save_catalog(){
    FILE* file = fopen("catalog.meta","wb");
    if (!file){
        return;
    }
    fwrite(&catalog,sizeof(Catalog),1,file);
    fclose(file);
    return;
}
int main(int argc, char *argv[]) {
    load_catalog();
    InputBuffer *input_buffer = new_input_buffer();
    while (true){
        printf("db >");
        read_input(input_buffer);
        //parser
        ASTnode* ast = parser(input_buffer);
        planner(ast);
    }
    return 0;
}