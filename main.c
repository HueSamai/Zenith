#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "main.h"
#include "errors.h"
#include "c-hashmap/map.h"
#include <math.h>

#define MAX_NAME_LENGTH 50

FILE *currentFile;
Node *startNode;
Node *currentNode;
int currentLine = 1;

hashmap *variables;

void throw(char *errname, int line, char *message)
{
    printf("%s(%d): %s", errname, line, message);
    fclose(currentFile);
    exit(1);
}

void advanceNode()
{
    currentNode = currentNode->nextNode;
}

char *cmb_s(char *a, char *b, char *c)
{
    char *f = (char *)malloc(sizeof(*a) + sizeof(*b) + sizeof(*c) + 1);
    int i = 0;
    int j = 0;

    while (a[i] != '\0')
    {
        f[j] = a[i];
        i++;
        j++;
    }

    i = 0;
    while (b[i] != '\0')
    {
        f[j] = b[i];
        i++;
        j++;
    }

    i = 0;
    while (c[i] != '\0')
    {
        f[j] = c[i];
        i++;
        j++;
    }

    f[j] = '\0';
    return f;
}

int getLengthOfInt(int n)
{
    if (n == 0)
    {
        return 1;
    }

    int length = 0;
    
    while (n > 0)
    {
        length++;
        n /= 10;
    }

    return length;
}

int ACCURACY_LIMIT = 5;
int ACCURACY_MUL = 0;
char *tostring(double d)
{
    if (ACCURACY_MUL == 0)
    {
        ACCURACY_MUL = pow(10, ACCURACY_LIMIT);
    }

    int a = round(d * ACCURACY_MUL);
    int decimalPoint = getLengthOfInt((int)d);
    int numberOfZerosInFront = 0;
    
    int length = getLengthOfInt(a);
    if (d < 1)
    {
        numberOfZerosInFront = getLengthOfInt((d + 1) * ACCURACY_MUL) - length;
    }

    int strLength = numberOfZerosInFront + length;
    if (decimalPoint == strLength) // checks if decimal point is going to be displayed
    {
        strLength--;
    }
    char *str = (char *)malloc(strLength);

    int i = strLength;
    int rem;
    while (i > -1)
    {
        if (i == decimalPoint) // checks if should display decimal pint
        {
            str[i] = '.';
            i--;
            continue;
        }
        
        // decides whether it should fill spot with zero
        if (i < numberOfZerosInFront)
        {
            str[i] = '0';
            i--;
            continue;
        }

        // gets the next digit
        rem = a % 10;
        str[i] = rem + '0';
        a /= 10;
        i--;
    }

    str[numberOfZerosInFront + length + 1] = '\0'; // adds \0 to show end of string
    return str;
}

void jumpToPreviousIndex()
{
    fseek(currentFile, -1, SEEK_CUR);
}

Token getToken(Node *n)
{
    return *n->value;
}

char *evaluate(Node *c_node)
{
    Token firstToken = getToken(c_node);
    advanceNode();
    Token currentToken = getToken(currentNode);

    char *currentValue = firstToken.lexeme;
    TokenType currentValueType = firstToken.type;

    while (currentToken.type != LINE_END)
    {
        if (currentToken.type == OPERATOR)
        {
            char op = currentToken.lexeme[0];

            if (currentNode->nextNode == NULL)
            {
                throw(ERR_INV_OP, currentNode->value->line, "Tried to perform operation on nothing.");
            }
            advanceNode();
            currentToken = getToken(currentNode);

            currentValue = operate(op, currentValue, currentToken.lexeme, currentValueType, currentToken.type);
            if (currentToken.type == STRING)
            {
                currentValueType = STRING;
            }
        }

        // get next token
        if (currentNode->nextNode == NULL)
        {
            break;
        }
        advanceNode();
        currentToken = getToken(currentNode);
    }

    return currentValue;
}

char *operate(char operator, char * a, char *b, TokenType atype, TokenType btype)
{
    if (operator== '+')
    {
        if (atype != btype)
        {
            throw(ERR_INV_OP, currentNode->value->line,
                  "Cannot perform operation '+' on different token types.");
        }

        if (atype == NUMBER)
        {
            return tostring(atof(a) + atof(b));
        }

        if (atype == STRING)
        {
            char *com = cmb_s(a, b, "");
            free(a);
            return com;
        }

        throw(ERR_INV_OP, currentNode->value->line, cmb_s("Cannot perform operation '+' on token ", tokenNames[atype], ""));
    }
    if (operator== '-')
    {
        if (atype != btype)
        {
            throw(ERR_INV_OP, currentNode->value->line,
                  "Cannot perform operation '+' on different token types.");
        }

        if (atype == NUMBER)
        {
            return tostring(atof(a) - atof(b));
        }

        if (atype == STRING)
        {
            throw(ERR_INV_OP, currentNode->value->line, "Cannot perform operation '-' on a string");
        }

        throw(ERR_INV_OP, currentNode->value->line, cmb_s("Cannot perform operation '+' on token ", tokenNames[atype], ""));
    }
    if (operator== '*')
    {
        if (atype == NUMBER)
        {
            return tostring(atof(a) * atof(b));
        }

        if (atype == STRING)
        {
            throw(ERR_INV_OP, currentNode->value->line, "Cannot perform operation '*' on a string");
        }

        throw(ERR_INV_OP, currentNode->value->line, cmb_s("Cannot perform operation '+' on token ", tokenNames[atype], ""));
    }
    if (operator== '/')
    {
        if (atype == NUMBER)
        {
            return tostring(atof(a) / atof(b));
        }

        if (atype == STRING)
        {
            throw(ERR_INV_OP, currentNode->value->line, "Cannot perform operation '*' on a string");
        }

        throw(ERR_INV_OP, currentNode->value->line, cmb_s("Cannot perform operation '+' on token ", tokenNames[atype], ""));
    }
}

void lex()
{
    while (1)
    {
        char c = fgetc(currentFile);

        if (feof(currentFile) || c == EOF)
        {
            break;
        }
        processChar(c);
    }

    addToken(HALT, "");
}

void addToken(TokenType type, char *lexeme)
{
    Token *token = (Token *)malloc(sizeof(Token));
    token->type = type;
    token->lexeme = lexeme;
    token->line = currentLine;

    if (startNode == NULL)
    {
        startNode = (Node *)malloc(sizeof(Node));
        startNode->nextNode = NULL;
        startNode->value = token;
        return;
    }

    Node *currentNode = startNode;
    while (currentNode->nextNode != NULL)
    {
        currentNode = currentNode->nextNode;
    }

    currentNode->nextNode = (Node *)malloc(sizeof(Node));
    currentNode->nextNode->value = token;
    currentNode->nextNode->nextNode = NULL;
}

void processString()
{
    char s[MAX_NAME_LENGTH]; // sets up a buffer for the string

    char c = fgetc(currentFile);
    int length = 0;
    while (c != '"')
    {
        s[length] = c;
        length++;
        c = fgetc(currentFile);
    }

    char *newS = (char *)malloc(length + 1); // creates a smaller string

    int j = 0;
    while (j < length)
    {
        newS[j] = s[j];
        j++;
    }
    newS[j] = '\0';

    addToken(STRING, newS);
}

void processNumber()
{
    char *s = (char *)malloc(MAX_NAME_LENGTH);

    jumpToPreviousIndex();
    char c = fgetc(currentFile);

    int hadPoint = 0;

    int i = 0;
    while (isdigit(c) || (c == '.' && !hadPoint))
    {
        if (c == '.')
        {
            hadPoint = 1;
        }
        s[i] = c;
        i++;
        c = fgetc(currentFile);
    }

    char *newS = (char *)malloc(i + 1); // creates a smaller string

    int j = 0;
    while (j < i)
    {
        newS[j] = s[j];
        j++;
    }
    newS[j] = '\0';

    if (!feof(currentFile))
    {
        jumpToPreviousIndex();
    }

    addToken(NUMBER, newS);
}

int isKeyword(char *s)
{
    for (int i = 0; i < KEYWORDS_SIZE; i++)
    {
        if (!strcmp(s, keywords[i]))
        {
            return 1;
        }
    }

    return 0;
}

void processWord()
{
    char *s = (char *)malloc(MAX_NAME_LENGTH);

    jumpToPreviousIndex();
    char c = fgetc(currentFile);

    int i = 0;
    while (isalpha(c) || isdigit(c))
    {
        s[i] = c;
        i++;
        c = fgetc(currentFile);
    }

    char *newS = (char *)malloc(i + 1); // creates a smaller string

    int j = 0;
    while (j < i)
    {
        newS[j] = s[j];
        j++;
    }
    newS[j] = '\0';

    if (isKeyword(s))
    {
        addToken(KEYWORD, s);
    }
    else
    {
        addToken(IDENTIFIER, s);
    }

    if (!feof(currentFile))
    {
        jumpToPreviousIndex();
    }
}

void processChar(char c)
{
    switch (c)
    {
    case '*':
        addToken(OPERATOR, "*");
        break;
    case '/':
        addToken(OPERATOR, "/");
        break;
    case '+':
        addToken(OPERATOR, "+");
        break;
    case '-':
        addToken(OPERATOR, "-");
        break;
    case '"':
        processString();
        break;
    case '{':
        addToken(BLOCK_START, "");
        break;
    case '}':
        addToken(BLOCK_END, "");
        break;
    case ',':
        addToken(COMMA, "");
        break;
    case '\n':
        currentLine++;
        addToken(LINE_END, "");
        break;
    default:
        if (isspace(c))
        {
            break;
        }
        if (isdigit(c))
        {
            processNumber();
            break;
        }
        if (isalpha(c))
        {
            processWord();
            break;
        }
        char arr[] = {c, 0};
        throw(ERR_UNRCG_CHAR, currentLine, cmb_s("Unrecognised character '", arr, "' in file."));
        break;
    }
}

void handleLet(Node *c_node)
{
    Token nextToken = *c_node->nextNode->value;
    if (nextToken.type != IDENTIFIER)
    {
        throw(ERR_UNEXP_TOKEN, nextToken.line,
              cmb_s("Expected an 'IDENTIFIER' but instead got a '",
                    tokenNames[nextToken.type], "'."));
    }
    char *varName = nextToken.lexeme;
}

void handleDisplay(Node *c_node)
{
    printf("%s\n", evaluate(c_node->nextNode));
}

void parse()
{
    currentNode = startNode;
    while (currentNode != NULL)
    {
        char *lxm = currentNode->value->lexeme;
        TokenType type = currentNode->value->type;

        switch (type)
        {
        case KEYWORD:
            if (!strcmp(lxm, "let"))
            {
                handleLet(currentNode);
                break;
            }
            if (!strcmp(lxm, "display"))
            {
                handleDisplay(currentNode);
                break;
            }
        }
        advanceNode();
    }
}

void run()
{
    lex();

    variables = hashmap_create();

    parse();
    FILE *output = fopen("tokens.txt", "wb");

    Node *node = startNode;
    while (node != NULL)
    {
        fputs(tokenNames[node->value->type], output);
        fputc(' ', output);
        fputs(node->value->lexeme, output);
        fputc('\n', output);

        node = node->nextNode;
    }
    fclose(output);
}

int main(int argc, char **argv)
{
    printf("Running Zenith...\n");
    // opens file based on argv
    currentFile = fopen(argv[1], "rb");

    // runs code inside of currentFile
    run();

    // closes the file
    fclose(currentFile);
    return 0;
}