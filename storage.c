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
int create_index(Createindexnode* node){
    
}
int check_num(int num1,int num2,Operator op){
    switch(op){
        case OP_EQ:
            return (num1 == num2) ? 0: -1;
            break;
        case OP_NEQ:
            return (num1 != num2) ? 0: -1;
            break;
        case OP_GT:
            return (num1 > num2) ? 0: -1;
            break;
        case OP_GTEQ:
            return (num1 >= num2) ? 0: -1;
            break;
        case OP_LT:
            return (num1 < num2) ? 0: -1;
            break;
        case OP_LTEQ:
            return (num1 <= num2) ? 0: -1;
            break;
        default:
            return -1;
    }
}
int check_where(Page* page,int* row,Wherenode* node,int where_no){
    if (node->type == NONE){
        return 0;
    }
    if (node->type == NUMBER){
        int num = *(int*)((char*)page + row[where_no]);
        return check_num(num,atoi(node->value),node->op);
    }
    else{
        short len = *(short*)((char*)page + row[where_no]);
        char* str = malloc(len + 1);
        memcpy(str,((char*)page + row[where_no]+sizeof(short)),len);
        str[len] = '\0';
        switch(node->op){
            case OP_EQ:
                return strcmp(str,node->value)==0?0:-1;
                break;
            case OP_NEQ:
                return strcmp(str,node->value)!=0?0:-1;
                break;
            default:
                return -1;
        }
    }
}
int select_page(Page* page,Selectnode* node){
    int is_star = 0;
    if (strcmp(node->cols[0],"*")==0){
        is_star =1;
    }
    for ( int i = 0; i < page->header.slot_count;i++){
        Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(i+1));
        if(slot->is_deleted == 'Y')
            continue;
        for (int j = 0;j < catalog.table_count;j++){
            if(strcmp(catalog.tables[j]->table_name,node->table)==0){
                int* row_pointers = malloc(catalog.tables[j]->col_count * sizeof(int));
                int offset = 0;
                for (int k = 0;k < catalog.tables[j]->col_count;k++){
                    row_pointers[k] = slot->offset + offset;
                    if (catalog.tables[j]->cols[k].type == Int){
                        offset += sizeof(int);
                    }
                    else{
                        offset += sizeof(short) + *(short*)((char*)page + slot->offset + offset);
                    }
                }
                int* col_no = malloc(node->col_count * sizeof(int));
                for (int k = 0;k < node->col_count;k++){
                    for (int l = 0;l < catalog.tables[j]->col_count;l++){
                        if(strcmp(node->cols[k],catalog.tables[j]->cols[l].name)==0){
                            col_no[k] = l;
                            break;
                        }
                    }
                }
                int where_no = -1;
                if (node->where->type != NONE){
                    for (int k = 0;k < catalog.tables[j]->col_count;k++){
                        if(strcmp(node->where->column,catalog.tables[j]->cols[k].name)==0){
                            where_no = k;
                            break;
                        }
                    }
                }
                if (check_where(page,row_pointers,node->where,where_no) ==0){
                    //print
                    if (is_star == 1){
                        for (int k = 0; k < catalog.tables[j]->col_count;k++){
                            if (catalog.tables[j]->cols[k].type == Int){
                                int num = *(int*)((char*)page + row_pointers[k]);
                                int num2;
                                memcpy(&num2,(char*)page + row_pointers[k],sizeof(int));
                                printf("%i  ",num);
                            }
                            else{
                                short len = *(short*)((char*)page + row_pointers[k]);
                                char* str = malloc(len + 1);
                                memcpy(str,((char*)page + row_pointers[k]+sizeof(short)),len);
                                str[len] = '\0';
                                printf("%s  ",str);
                            }
                        }
                    }
                    else{
                        for (int k = 0; k < node->col_count;k++){
                            if (catalog.tables[j]->cols[col_no[k]].type == Int){
                                int num = *(int*)((char*)page + row_pointers[col_no[k]]);
                                printf("%i  ",num);
                            }
                            else{
                                short len = *(short*)((char*)page + row_pointers[col_no[k]]);
                                char* str = malloc(len + 1);
                                memcpy(str,((char*)page + row_pointers[col_no[k]]+sizeof(short)),len);
                                str[len] = '\0';
                                printf("%s  ",str);
                            }
                        }
                    }
                    printf("\n");
                }
                break;
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
    fclose(file);
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
    return save_page(table,page->header.page_id,page);
}
Page* insert_at_end(Insertnode* node,char* table,int byte_size){
    FILE* file = fopen(table,"wb");
    fseek(file,0,SEEK_END);
    Page* page = malloc(sizeof(Page));
    page->header.page_id = ftell(file) / PAGE_SIZE;
    fclose(file);
    page->header.slot_count = 1;
    page->header.free_space_offset = sizeof(Pageheader) + byte_size;
    Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(page->header.slot_count));
    slot->is_deleted = 'N';
    slot->offset = sizeof(Pageheader);
    slot->length = byte_size;
    insert_record(node,page,slot,table);
    return page;
}
int insert_data(Insertnode* node){
    char* table_name = malloc(strlen(node->table)+strlen(".db") + 1);
    char* table_fsm_name = malloc(strlen(node->table)+strlen(".fsm") + 1);
    strcpy(table_name,node->table);
    strcpy(table_fsm_name,node->table);
    strcat(table_name,".db");
    strcat(table_fsm_name,".fsm");
    FreeSpaceMap* fsm = load_fsm(table_fsm_name);
    int byte_size = 0;
    for (int i = 0;i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            byte_size+= sizeof(node->cols[i]->int_value);
        }
        else{
            byte_size+= strlen(node->cols[i]->value) + sizeof(short);
        }
    }
    for (int i =0;i < fsm->entry_count;i++){
        if (byte_size < fsm->entries[i].free_bytes){
            Page* page = load_page(table_name,fsm->entries[i].page_id);
            Slot* slot = get_slot(page,byte_size);
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