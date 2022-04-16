#ifndef __LEX__
#define __LEX__

#define MAXLEN 256

// Token types
typedef enum
{
    UNKNOWN,
    END,
    ENDFILE,
    INT,
    ID,
    ADDSUB,
    MULDIV,
    ASSIGN,
    LPAREN,
    RPAREN,
    //new added
    //todo 這些operator的順序在四則運算之後: [*, /] > [+, -] > & > ^ > | 要想的地方是，如何在factor/term/expr中插入and/or/xor的邏輯

    XOR,
    INCDEC,
    AND,
    OR
} TokenSet;

// Test if a token matches the current token
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__
