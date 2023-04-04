#define KEYWORDS_SIZE 17


typedef enum
{
    IDENTIFIER = 0,
    NUMBER = 1,
    STRING = 2,
    KEYWORD = 3,
    OPERATOR = 4,
    BLOCK_START = 5,
    BLOCK_END = 6,
    COMMA = 7,
    LINE_END = 8,
    HALT = 9
} TokenType;

char* tokenNames[10] =
{
    "IDENTIFIER",
    "NUMBER",
    "STRING",
    "KEYWORD",
    "OPERATOR",
    "BLOCK_START",
    "BLOCK_END",
    "COMMA",
    "LINE_END",
    "HALT"
};

char *keywords[KEYWORDS_SIZE] = {
    "with",
    "output",
    "break",
    "loop",
    "if",
    "function",
    "let",
    "be",
    "yes",
    "no",
    "display",
    "input",
    "import",
    "true",
    "false",
    "throw",
    "display"
};

typedef struct Token
{
    TokenType type;
    char *lexeme;
    int line;
} Token;

typedef struct Node
{
    struct Token *value;
    struct Node *nextNode;
} Node;

void lex();
void addToken(TokenType type, char* lexeme);
void processChar(char c);
void processString();
void processNumber();
void processWord();
void run();
void parse();
void handleLet();
void jumpToPreviousIndex();
char* evaluate(Node *c_node);
char* tostring(double);
Token getToken(Node *node);
char *operate(char operator, char* a, char* b, TokenType atype, TokenType btype);
int main(int argc, char **argv);
char* cmb_s(char* a, char* b, char* c);
void throw(char* errname, int line, char* message);