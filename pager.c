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
FreeSpaceMap* load_fsm(char* table){
    FreeSpaceMap* fsm = malloc(sizeof(FreeSpaceMap));
    FILE* file = fopen(table,"rb");
    if (!file){
        fsm->entry_count = 0;
        return fsm;
    }
    fread(&fsm->entry_count,sizeof(int),1,file);
    fsm->entries = malloc(fsm->entry_count*sizeof(FreeSpaceEntry));
    for(int i = 0;i< fsm->entry_count;i++){
        fread(&fsm->entries[i],sizeof(FreeSpaceEntry),1,file);
    }
    fclose(file);
    return fsm;
}
int save_fsm(char* table,FreeSpaceMap* fsm){
    FILE* file = fopen(table,"wb");
    if (!file){
        return -1;
    }
    fwrite(fsm->entry_count,PAGE_SIZE,1,file);
    for(int i = 0;i< fsm->entry_count;i++){
        fwrite(&fsm->entries[i],sizeof(FreeSpaceEntry),1,file);
    }
    fclose(file);
    return 0;
}