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
char* get_value(Page* page,int table_no, int col_no,int i){
    Slot* slot = (Slot*)((char*)page + PAGE_SIZE - sizeof(Slot)*(i+1));
    int offset = 0;
    for (int j = 0; j < col_no;j++){
        if (catalog.tables[table_no]->cols[j].type == Int){
            offset += sizeof(int);
        }
        else{
            int val = *(int*)((char*)page + offset +slot->offset);
            offset += sizeof(int) + val;
        }
    }
    char* val;
    if (catalog.tables[table_no]->cols[col_no].type == Int){
        val = malloc(sizeof(int));
        memcpy(val,(char*)page + offset +slot->offset,sizeof(int));
    }
    else{
        int len = *(int*)((char*)page + offset +slot->offset);
        val= malloc(len+1);
        memcpy(val,(char*)page + offset +slot->offset + sizeof(int),len);
        val[len] = '\0';
    }
    return val;
}
print_index(Selectnode* node,char* table_name,int pageno,Keyword type){
    IndexPage* page = load_idx(table_name,pageno);
    for (int i = 0; i < page->header.slot_count;i++){
        
    }
}
int traverse_index(Selectnode* node,char* table_name,int pageno,Keyword type){
    IndexPage* page = load_idx(table_name,pageno);
    if (page->header.type == LEAF_NODE){
        free(page);
        print_index(node,table_name,pageno,type);
        return;
    }
    for(int i = 0;i < page->header.slot_count;i++){
        IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - i) * sizeof(IndexSlot);
        if (type == Int){
            int val;
            memcpy(&val,(char*) page + slot->offset,sizeof(int));
            if (atoi(node->where->value) <= val){
                int indexpageno;
                if (i==0){
                    indexpageno = page->header.next_page;
                }
                else{
                    slot -=1;
                    memcpy(&indexpageno,(char*) page + slot->offset  + sizeof(int),sizeof(int));
                }
                traverse_index(node,table_name,indexpageno,type);
                break;
            }
        }
        else{
            char* val = malloc(slot->key_len + 1);
            memcpy(&val,(char*) page + slot->offset,slot->key_len);
            val[slot->key_len] = '\0';
            if (strcmp(node->where->value,val) <=0){
                int indexpageno;
                if (i==0){
                    indexpageno = page->header.next_page;
                }
                else{
                    slot -=1;
                    memcpy(&indexpageno,(char*) page + slot->offset  + slot->key_len,sizeof(int));
                }
                traverse_index(node,table_name,indexpageno,type);
                break;
            }
        }
    }
    free(page);
}
int select_data_from_index(Selectnode* node){
    int table_index;
    int col_index;
    for (int i = 0;i < catalog.table_count;i++){
        if (strcmp(node->table,catalog.tables[i]->table_name)==0){
            table_index = i;
            break;
        }
    }
    for (int i = 0;i < catalog.tables[table_index]->col_count;i++){
        if (strcmp(node->where->column,catalog.tables[table_index]->cols[i].name)==0){
            col_index = i;
            break;
        }
    }
    char* table_name = malloc(strlen(catalog.tables[table_index]->cols[col_index].index_name)+strlen(".idx") +1);
    strcpy(table_name,catalog.tables[table_index]->cols[col_index].index_name);
    strcat(table_name,".idx");
    traverse_index(node,table_name,0,catalog.tables[table_index]->cols[col_index].type);
}
char* traverse_index(char* table_name,int pageno,char* value,Keyword type,int heap_page,int slotno){
    IndexPage* page = load_idx(table_name,pageno);
    int free_bytes = PAGE_SIZE - page->header.free_space_offset - page->header.slot_count * sizeof(IndexSlot);
    if (page->header.type == LEAF_NODE){
        if(free_bytes < strlen(value) + 2*sizeof(int) + sizeof(IndexSlot)){

        }
        int index;
        for(int i =0; i < page->header.slot_count;i++){
            IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - i) * sizeof(IndexSlot);
            if (type == Int){
                int val;
                memcpy(&val,(char*) page + slot->offset,sizeof(int));
                if (atoi(value) <= val){
                    index = i;
                    break;
                }
                else{
                    memcpy(slot -1,slot,sizeof(IndexSlot));
                }
            }
            else{
                char* val = malloc(slot->key_len+1);
                memcpy(&val,(char*) page + slot->offset,sizeof(slot->key_len));
                val[slot->key_len] = '\0';
                if (strcmp(value,val) <= 0){
                    index = i;
                    break;
                }
                else{
                    memcpy(slot -1,slot,sizeof(IndexSlot));
                }
            }
            if(i == page->header.slot_count - 1){
                index = page->header.slot_count;
            }
        }
        IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - index + 1) * sizeof(IndexSlot);
        if (type == Int){
            slot->offset = page->header.free_space_offset;
            int int_val = atoi(value);
            memcpy((char*) page + slot->offset,&int_val,sizeof(int));
            memcpy((char*) page + slot->offset + sizeof(int),&heap_page,sizeof(int));
            memcpy((char*) page + slot->offset + 2*sizeof(int),&slotno,sizeof(int));
            page->header.free_space_offset += 3*sizeof(int);
            page->header.slot_count++;
        }
        else{
            slot->offset = page->header.free_space_offset;
            slot->key_len = strlen(value);
            memcpy(slot->offset,value,slot->key_len);
            memcpy((char*) page + slot->offset + slot->key_len,&heap_page,sizeof(int));
            memcpy((char*) page + slot->offset + slot->key_len + sizeof(int),&slotno,sizeof(int));
            page->header.free_space_offset += slot->key_len + 2*sizeof(int);
            page->header.slot_count++;
        }
    }
    else{
        if(free_bytes < strlen(value) + 2*sizeof(int) + sizeof(IndexSlot)){

        }
        int index;
        for(int i =0; i < page->header.slot_count;i++){
            IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - i) * sizeof(IndexSlot);
            if (type == Int){
                int val;
                memcpy(&val,(char*) page + slot->offset,sizeof(int));
                if (atoi(value) <= val){
                    index = i;
                    break;
                }
            }
            else{
                char* val = malloc(slot->key_len+1);
                memcpy(&val,(char*) page + slot->offset,sizeof(slot->key_len));
                val[slot->key_len] = '\0';
                if (strcmp(value,val) <= 0){
                    index = i;
                    break;
                }
            }
            if(i == page->header.slot_count - 1){
                index = page->header.slot_count;
            }
        }
        char* value_to_insert;
        if (index == 0){
            value_to_insert = traverse_index(table_name,page->header.next_page,value,type,heap_page,slotno);
        }
        else{
            IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - index + 1) * sizeof(IndexSlot);
            int child_page;
            if (type == Int){
                memcpy(&child_page,(char*) page + slot->offset + sizeof(int),sizeof(int));
            }
            else{
                memcpy(&child_page,(char*) page + slot->offset + slot->key_len,sizeof(int));
            }
            value_to_insert = traverse_index(table_name,child_page,value,type,heap_page,slotno);
        }
        if(value_to_insert != NULL){
            if(free_bytes < strlen(value) + sizeof(int) + sizeof(IndexSlot)){

            }
            for (int i =0; i < index;i++){
                IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - i) * sizeof(IndexSlot);
                memcpy(slot -1,slot,sizeof(IndexSlot));
            }
            IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - index + 1) * sizeof(IndexSlot);
            FILE* file = fopen(table_name,"rb");
            fseek(file,0,SEEK_END);
            int total_pages = ftell(file) / PAGE_SIZE;
            total_pages--;
            if (type == Int){
                slot->offset = page->header.free_space_offset;
                int int_val = atoi(value_to_insert);
                memcpy((char*) page + slot->offset,&int_val,sizeof(int));
                memcpy((char*) page + slot->offset + sizeof(int),&total_pages,sizeof(int));
                page->header.free_space_offset += 2*sizeof(int);
                page->header.slot_count++;
            }
            else{
                slot->offset = page->header.free_space_offset;
                slot->key_len = strlen(value_to_insert);
                memcpy(slot->offset,value,slot->key_len);
                memcpy((char*) page + slot->offset + slot->key_len,&total_pages,sizeof(int));
                page->header.free_space_offset += slot->key_len + sizeof(int);
                page->header.slot_count++;
            }
        }
    }
}
int insert_into_index(char* table_name,char* value,Keyword type,int insert_page,int insert_slot){
    FILE* file = fopen(table_name,"rb");
    fseek(file,0,SEEK_END);
    int total_pages = ftell(file) / PAGE_SIZE;
    fclose(file);
    if (total_pages == 0){
        IndexPage* page = malloc(sizeof(IndexPage));
        page->header.free_space_offset = sizeof(IndexHeader);
        page->header.page_id = 0;
        page->header.next_page = -1;
        page->header.slot_count = 0;
        page->header.type = LEAF_NODE;
        save_idx(table_name,page,0);
        free(page);
    }
    traverse_index(table_name,0,value,type,insert_page,insert_slot);
    IndexPage* page = load_idx(table_name,0);
    if (PAGE_SIZE - page->header.free_space_offset - sizeof(IndexSlot) * page->header.slot_count)
    for(int i =0; i < page->header.slot_count;i++){
        IndexSlot* slot = (char*) page + PAGE_SIZE - (page->header.slot_count - i) * sizeof(IndexSlot);
        if (type == Int){

        }
        else{
            
        }
    }
}
int create_index(Createindexnode* node){
    int table_index;
    int col_index;
    for (int i = 0;i < catalog.table_count;i++){
        if (strcmp(catalog.tables[i],node->table)==0){
            table_index = i;
            break;
        }
    }
    for (int i = 0;i < catalog.tables[table_index]->col_count;i++){
        if (strcmp(catalog.tables[table_index]->cols->name,node->table)==0){
            col_index = i;
            break;
        }
    }
    if (catalog.tables[table_index]->cols[col_index].index==1){
        printf("index on this column already exists\n");
        return NULL;
    }
    catalog.tables[table_index]->cols[col_index].index=1;
    save_catalog();
    char* table_idx_name = malloc(strlen(node->table)+strlen(".idx") + 1);
    strcpy(table_idx_name,node->table);
    strcat(table_idx_name,".idx");
    char* table_name = malloc(strlen(node->table)+strlen(".db") + 1);
    strcpy(table_name,node->table);
    strcat(table_name,".db");
    FILE* file = fopen(table_name,"rb");
    fseek(file,0,SEEK_END);
    int total_pages = ftell(file) / PAGE_SIZE;
    fclose(file);
    for (int i = 0; i < total_pages;i++){
        Page* page = load_page(table_name,i);
        for (int j =0;j < page->header.slot_count;j++){
            char* value = get_value(page,table_index,col_index,j);
            insert_into_index(table_idx_name,value,catalog.tables[table_index]->cols[col_index].type,i,j);
        }
    }
    return 0;
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
int select_data_from_index(){

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
    int table_idx;
    for (int i = 0; i < catalog.table_count;i++){
        if(strcmp(node->table,catalog.tables[i]->table_name)==0){
            table_idx = i;
            break;
        }
    }
    for (int i =0; i < node->col_count;i++){
        if (node->cols[i]->type == NUMBER){
            memcpy((char*)page + slot->offset + offset,&node->cols[i]->int_value,sizeof(int));
            offset += sizeof(int);
            if (catalog.tables[table_idx]->cols[i].index==1){
                char* table_idx_name = malloc(strlen(catalog.tables[table_idx]->cols[i].index_name)+strlen(".idx") + 1);
                strcpy(table_idx_name,catalog.tables[table_idx]->cols[i].index_name);
                strcat(table_idx_name,".idx");
                insert_into_index(table_idx_name,&node->cols[i]->int_value,catalog.tables[table_idx]->cols[i].type,page->header.page_id,page->header.slot_count);
            }
        }
        else{
            short len = strlen(node->cols[i]->value);
            memcpy((char*)page + slot->offset + offset,&len,sizeof(short));
            offset += sizeof(short);
            memcpy((char*)page + slot->offset + offset,node->cols[i]->value,len);
            offset+=len;
            if (catalog.tables[table_idx]->cols[i].index==1){
                char* table_idx_name = malloc(strlen(catalog.tables[table_idx]->cols[i].index_name)+strlen(".idx") + 1);
                strcpy(table_idx_name,catalog.tables[table_idx]->cols[i].index_name);
                strcat(table_idx_name,".idx");
                insert_into_index(table_idx_name,node->cols[i]->value,catalog.tables[table_idx]->cols[i].type,page->header.page_id,page->header.slot_count);
            }
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