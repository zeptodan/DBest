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
int select_page(Page* page,Selectnode* node){
    for ( int i = 0; i < page->header.slot_count;i++){
        Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(i+1));
        if(slot->is_deleted == 'Y')
            continue;
        for (int j = 0;j < catalog.table_count;j++){
            if(strcmp(catalog.tables[j]->table_name,node->table)==0){
                for (int k = 0;k < catalog.tables[j]->col_count;k++){
                    //catalog.tables[j]->cols.
                }
            }
        }
    }
}
int select_data(Selectnode* node){
    char* table_name = malloc(strlen(node->table)+strlen(".db") + 1);
    strcpy(table_name,node->table);
    strcat(table_name,".db");
    FILE* file = fopen(table_name,"rb");
    fseek(file,0,SEEK_END);
    int total_pages = ftell(file) / PAGE_SIZE;
    for (int i = 0;i < total_pages;i++){
        Page* page = load_page(table_name,i);
        select_page(page,node);
    }
}
Slot* get_slot(Page* page,int byte_size){
    for (int i = 0;i< page->header.slot_count;i++){
        Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(i + 1));
        if (slot->is_deleted == 'N' || slot->length < byte_size){
            continue;
        }
        else{
            return slot;
        }
    }
    return NULL;
}
int insert_record(Insertnode* node,Page* page, Slot* slot,char* table){
    int offset = 0;
    for (int i =0; i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            memcpy((char*)page + slot->offset + offset,&node->cols[i]->int_value,sizeof(int));
            offset += sizeof(int);
        }
        else{
            short len = strlen(node->cols[i]->value);
            memcpy((char*)page + slot->offset + offset,&len,sizeof(short));
            offset += sizeof(short);
            memcpy((char*)page + slot->offset + offset,node->cols[i]->value,len);
            offset+=len;
        }
    }
    printf("offset: %i\nend: %i\npage id: %i\nsize of slot %i\n",page->header.free_space_offset,PAGE_SIZE - sizeof(Slot)*(page->header.slot_count + 1),page->header.page_id,sizeof(Slot));
    return save_page(table,page->header.page_id,page);
}
Page* insert_at_end(Insertnode* node,char* table,int byte_size){
    FILE* file = fopen(table,"wb");
    fseek(file,0,SEEK_END);
    Page* page = malloc(sizeof(Page));
    page->header.page_id = ftell(file) / PAGE_SIZE;
    fclose(file);
    page->header.slot_count = 1;
    page->header.free_space_offset = sizeof(Pageheader);
    Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
    slot->is_deleted = 'N';
    slot->offset = sizeof(Pageheader);
    slot->length = byte_size;
    printf("page offset in function: %i\n",page->header.free_space_offset);
    insert_record(node,page,slot,table);
    printf("page offset in function after: %i\n",page->header.free_space_offset);
    return page;
}
int insert_data(Insertnode* node){
    char* table_name = malloc(strlen(node->table)+strlen(".db") + 1);
    char* table_fsm_name = malloc(strlen(node->table)+strlen(".fsm") + 1);
    strcpy(table_name,node->table);
    strcpy(table_fsm_name,node->table);
    strcat(table_name,".db");
    strcat(table_fsm_name,".fsm");
    printf("table name: %s\n",table_name);
    printf("table fsm name: %s\n",table_fsm_name);
    FreeSpaceMap* fsm = load_fsm(table_fsm_name);
    printf("fsm count: %i\n",fsm->entry_count);
    int byte_size = 0;
    for (int i = 0;i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            byte_size+= sizeof(node->cols[i]->int_value);
        }
        else{
            byte_size+= strlen(node->cols[i]->value) + sizeof(short);
        }
    }
    printf("byte size: %i\n",byte_size);
    for (int i =0;i < fsm->entry_count;i++){
        if (byte_size < fsm->entries[i].free_bytes){
            printf("fsm entry count %i\n fsm id %i\n free bytes%i\n",fsm->entry_count,fsm->entries[0].page_id,fsm->entries[0].free_bytes);
            Page* page = load_page(table_name,fsm->entries[i].page_id);
            printf("offset: %i\nend: %i\n",page->header.free_space_offset,PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
            Slot* slot = get_slot(page,byte_size);
            printf("offset: %i\nend: %i\n",page->header.free_space_offset,PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
            if (!slot && byte_size + sizeof(Slot) > PAGE_SIZE - sizeof(Slot)*(page->header.slot_count) - page->header.free_space_offset){
                free(page);
                continue;
            }
            else if (!slot){
                slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(page->header.slot_count + 1));
                fsm->entries[i].free_bytes-= sizeof(Slot);
                slot->is_deleted = 'N';
                slot->length = byte_size;
                slot->offset = page->header.free_space_offset;
                page->header.free_space_offset += byte_size;
                page->header.slot_count++;
                printf("offset: %i\nend: %i\n",page->header.free_space_offset,PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
            }
            else{
                slot->is_deleted = 'N';
                slot->length = byte_size;
            }
            fsm->entries[i].free_bytes-= byte_size;
            //insert record
            if (insert_record(node,page,slot,table_name) == -1){
                free(table_name);
                free(table_fsm_name);
                free(fsm->entries);
                free(fsm);
                return -1;
            }
            else{
                save_fsm(table_fsm_name,fsm);
                free(table_name);
                free(table_fsm_name);
                free(fsm->entries);
                free(fsm);
                return 0;
            }
        }
    }
    Page* page = insert_at_end(node,table_name,byte_size);
    printf("page id: %i\n",page->header.page_id);
    fsm->entries = realloc(fsm->entries,fsm->entry_count+1);
    fsm->entries[fsm->entry_count].page_id = page->header.page_id;
    free(page);
    fsm->entries[fsm->entry_count].free_bytes = PAGE_SIZE - sizeof(Slot) - sizeof(Pageheader) - byte_size;
    fsm->entry_count++;
    save_fsm(table_fsm_name,fsm);
    free(table_name);
    free(table_fsm_name);
    free(fsm->entries);
    free(fsm);
    return 0;
}