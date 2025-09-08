#include<stdio.h>
#include "main.h"
Page* load_page(char* table,int pageno){
    Page* page = malloc(sizeof(Page));
    FILE* file = fopen(table,"rb");
    if (!file){
        return NULL;
    }
    fseek(file,PAGE_SIZE*pageno,SEEK_SET);
    fread(page->bytes,PAGE_SIZE,1,file);
    fclose(file);
    return page;
}
int save_page(char* table,int pageno,Page* page){
    FILE* file = fopen(table,"wb");
    if (!file){
        return -1;
    }
    fseek(file,PAGE_SIZE*pageno,SEEK_SET);
    fwrite(page->bytes,PAGE_SIZE,1,file);
    fclose(file);
    return 0;
}