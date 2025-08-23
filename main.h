#include<stdlib.h>
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_COL,
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
    Table,
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
const char *const keywords[];
const size_t keyword_length;
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
int parser(InputBuffer* buffer);
Statement* tokenizer(InputBuffer* buffer);