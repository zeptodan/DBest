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
int insert_record(Insertnode* node,Page* page, Slot* slot){
    int offset = 0;
    for (int i =0; i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            memcpy((char*)page + offset,&node->cols[i]->int_value,sizeof(int));
            offset += sizeof(int);
        }
        else{
            memcpy((char*)page + offset,node->cols[i]->value,strlen(node->cols[i]->value)+1);
            offset+=strlen(node->cols[i]->value)+1;
        }
    }
    save_page(node->table,page->header.page_id,page);
}
int insert_at_end(Insertnode* node,char* table,int byte_size){
    FILE* file = fopen(table,"wb");
    fseek(file,0,SEEK_END);
    Page* page = malloc(sizeof(Page));
    page->header.page_id = ftell(file) / PAGE_SIZE;
    page->header.slot_count = 1;
    Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
    slot->is_deleted = 'N';
    slot->offset = 0;
    slot->length = byte_size;
    insert_record(node,page,slot);
    return 0;
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
            Slot* slot = get_slot(page,byte_size);
            if (!slot && byte_size + sizeof(Slot) > PAGE_SIZE - sizeof(Slot)*(page->header.slot_count) - page->header.free_space_offset){
                continue;
            }
            else if (!slot){
                slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(page->header.slot_count + 1));
                fsm->entries[i].free_bytes-= sizeof(Slot) + byte_size;
            }
            //insert record
            if (insert_record(node,page,slot) == -1){
                return -1;
            }
        }
    }
    insert_at_end(node,table_name,byte_size);
}