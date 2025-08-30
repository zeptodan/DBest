#include<stdlib.h>
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
    OP
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
typedef struct {
    Exprnode* exprnode;
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
    Keyword type;
    int index;
} Column;
typedef struct {
    Column** cols;
    int col_count;
    char* table;
} Createnode;
typedef struct {
    Token token;
    union {
        Selectnode select;
        Insertnode insert;
        Createnode create;
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
    CREATE_PLAN
} Plantype;
typedef struct{
    Plantype type;
    char* index;
    union {
        Selectnode select;
        Insertnode insert;
        Createnode create;
    };
} Planner;
typedef struct {
    char* table_name;
    char* index;
    char** col_names;
    char** types;
    int col_count;
} Table;
typedef struct {
    int table_count;
    Table** tables;
} Catalog;
extern Catalog catalog;
ASTnode* parser(InputBuffer* buffer);
Statement* tokenizer(InputBuffer* buffer);
ASTnode* AST(Parser* parser);
void save_catalog();
int planner(ASTnode* ast);