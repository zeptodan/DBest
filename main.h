#include<stdlib.h>
#define PAGE_SIZE 4096
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDEN,
    TOKEN_LEFTPAREN,
    TOKEN_RIGHTPAREN,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_Eq,
    TOKEN_Neq,
    TOKEN_Lt,
    TOKEN_Gt,
    TOKEN_LtEq,
    TOKEN_GtEq,
    TOKEN_Mul,
    TOKEN_Div,
    TOKEN_Plus,
    TOKEN_Minus,
    TOKEN_WHITESPACE,
    TOKEN_COMMA,
    TOKEN_SEMI,
    TOKEN_UNDEFINED
} TokenType;
typedef enum {
    Select,
    Create,
    Update,
    Delete,
    Insert,
    Into,
    Values,
    Set,
    Drop,
    From,
    Where,
    And,
    Or,
    PrimaryKey,
    Keyword_Table,
    Database,
    Int,
    Varchar,
    Bool,
    True,
    False,
    Index,
    On,
    None
} Keyword;
typedef struct {
    Keyword keyword;
    char* value;
} KeyToken;
extern const KeyToken keywords[];
extern const size_t keyword_length;
typedef struct {
    TokenType type;
    union {
        Keyword keyword;
        char* value;
    } data;
} Token;
typedef struct {
    Token* tokens;
    size_t length;
    size_t capacity;

} Statement;
typedef struct {
    char input[1024];
    size_t input_length;
} InputBuffer;
typedef enum {
    LITERAL,
    NUMBER,
    IDEN,
    OP,
    NONE
} Exprtype;
typedef struct {
    Exprtype type;
    union {
        char* value;
        int int_value;
        char* ident;
        struct {
            char op;
            struct Exprnode* left;
            struct Exprnode* right;
        } operator;
    };
} Exprnode;
typedef enum { OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LTEQ, OP_GTEQ } Operator;
typedef struct Wherenode{
    Exprtype type;
    char* column;
    char* value;
    struct Wherenode* left;
    struct Wherenode* right;
    Operator op;
} Wherenode;
typedef struct {
    char** cols;
    int col_count;
    char* table;
    Wherenode* where;
} Selectnode;
typedef struct {
    Exprnode** cols;
    int col_count;
    char* table;
} Insertnode;
typedef struct {
    char* name;
    char* index_name;
    Keyword type;
    int index;
} Column;
typedef struct {
    Column* cols;
    int col_count;
    char* table;
} Createnode;
typedef struct {
 char* table;
 char* name;
 char* column;
} Createindexnode;
typedef struct {
    Token token;
    union {
        Selectnode select;
        Insertnode insert;
        Createnode create;
        Createindexnode createindex;
    };
} ASTnode;
typedef struct {
    Token* tokens;
    size_t length;
    size_t index;
} Parser;
typedef enum{
    INDEX_PLAN,
    SEQ_PLAN,
    INSERT_PLAN,
    CREATE_PLAN,
    CREATE_INDEX_PLAN
} Plantype;
typedef struct{
    Plantype type;
    char* index;
    union {
        Selectnode select;
        Insertnode insert;
        Createnode create;
        Createindexnode createindex;
    };
} Planner;
typedef struct {
    char* table_name;
    Column* cols;
    int col_count;
} Table;
typedef struct {
    int table_count;
    Table** tables;
} Catalog;
typedef struct {
    int page_id;
    int slot_count;
    int free_space_offset;
} Pageheader;
typedef struct { 
    short offset;
    short length;
    char is_deleted;
} Slot;
typedef struct {
    Pageheader header;
    char bytes[PAGE_SIZE - sizeof(Pageheader)];
} Page;
typedef struct {
    int page_id;
    int free_bytes;
} FreeSpaceEntry;
typedef struct {
    int entry_count;
    FreeSpaceEntry* entries;
} FreeSpaceMap;
typedef enum {
    LEAF_NODE,
    INTERNAL_NODE
} BNodeType;
typedef struct {
    short key_len;
    short offset;
} IndexSlot;
typedef struct {
    char* key;
    int child_page_id;
} InternalEntry;
typedef struct {
    char* key;
    int table_page_id;
    int slot_id;
} LeafEntry;
typedef struct {
    BNodeType type;
    int page_id;
    int slot_count;
    int free_space_offset;
    int next_page;
} IndexHeader;
typedef struct {
    IndexHeader header;
    char bytes[PAGE_SIZE - sizeof(IndexHeader)];
} IndexPage;
extern Catalog catalog;
ASTnode* parser(InputBuffer* buffer);
Statement* tokenizer(InputBuffer* buffer);
ASTnode* AST(Parser* parser);
void save_catalog();
Planner* planner(ASTnode* ast);
int executor(Planner* planner);
int select_data(Selectnode* node);
int insert_data(Insertnode* node);
int create_table(Createnode* node);
Page* load_page(char* table,int pageno);
int save_page(char* table,int pageno,Page* page);
FreeSpaceMap* load_fsm(char* table);
int save_fsm(char* table,FreeSpaceMap* fsm);
int create_index(Createindexnode* node);
IndexPage* load_idx(char* table,int pageno);
int save_idx(char* table,IndexPage* page,int pageno);