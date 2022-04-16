#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define TBLSIZE 64
#define MAXLEN 256
// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum)                                                 \
  {                                                                     \
    if (PRINTERR)                                                       \
      fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum);                                                      \
  }

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

// Error types
typedef enum
{
  UNDEFINED,
  MISPAREN,
  NOTNUMID,
  NOTFOUND,
  RUNOUT,
  NOTLVAL,
  DIVZERO,
  SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct
{
  int val;
  char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node
{
  TokenSet data;
  int val;
  char lexeme[MAXLEN];
  struct _Node *left;
  struct _Node *right;
} BTNode;

// Test if a token matches the current token
int match(TokenSet token);

// Get the next token
void advance(void);

// Get the lexeme of the current token
char *getLexeme(void);
int evaluateTree(BTNode *root, int);

// Print the syntax tree in prefix
void printPrefix(BTNode *root);

// The symbol table
Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
void initTable(void);

// Get the value of a variable
int getval(char *str);

// Set the value of a variable
int setval(char *str, int val);

// Make a new node according to token type and lexeme
BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
void freeTree(BTNode *root);

int getAddress(char *);
BTNode *factor(void);
BTNode *unary_expr();
BTNode *muldiv_expr();
BTNode *muldiv_expr_tail(BTNode *left);
BTNode *addsub_expr();
BTNode *addsub_expr_tail(BTNode *left);
BTNode *and_expr();
BTNode *and_expr_tail(BTNode *left);
BTNode *xor_expr_tail(BTNode *left);
BTNode *xor_expr();
BTNode *or_expr_tail(BTNode *left);
BTNode *or_expr(void);
BTNode *assign_expr();
void statement(void);

// Print error message and exit the program
void err(ErrorType errorNum);

TokenSet getToken(void);
TokenSet curToken = UNKNOWN;
char lexeme[MAXLEN];

int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void)
{
  strcpy(table[0].name, "x");
  table[0].val = 0;
  strcpy(table[1].name, "y");
  table[1].val = 0;
  strcpy(table[2].name, "z");
  table[2].val = 0;
  sbcount = 3;
}
//check : 得到記憶體位置
int getAddress(char *str)
{
  for (int i = 0; i < sbcount; i++)
  {
    if (strcmp(str, table[i].name) == 0)
      return i * 4;
  }
  error(UNDEFINED);
  return 0;
}
//check : 檢查變數是否存在
int getval(char *str)
{
  for (int i = 0; i < sbcount; i++)
    if (strcmp(str, table[i].name) == 0)
    {
      //advance();
      return table[i].val;
    }
  //error(NOTFOUND);
  error(UNDEFINED);
  return 0;
}

int setval(char *str, int val)
{
  int i = 0;

  for (i = 0; i < sbcount; i++)
  {
    if (strcmp(str, table[i].name) == 0)
    {
      table[i].val = val;
      return val;
    }
  }

  if (sbcount >= TBLSIZE)
    error(RUNOUT);

  strcpy(table[sbcount].name, str);
  // printf("setval str: %s\n", table[sbcount].name);
  table[sbcount].val = val;
  sbcount++;
  return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe)
{
  BTNode *node = (BTNode *)malloc(sizeof(BTNode));
  strcpy(node->lexeme, lexe);
  node->data = tok;
  node->val = 0;
  node->left = NULL;
  node->right = NULL;
  return node;
}

void freeTree(BTNode *root)
{
  if (root != NULL)
  {
    freeTree(root->left);
    freeTree(root->right);
    free(root);
  }
}

// factor := INT | ADDSUB INT |
//		   	 ID  | ADDSUB ID  |
//		   	 ID ASSIGN expr |
//		   	 LPAREN expr RPAREN |
//		   	 ADDSUB LPAREN expr RPAREN
BTNode *factor(void)
{
  BTNode *retp = NULL, *left = NULL, *adsub = NULL;
  int exist_val = 0;

  if (match(INT))
  {
    retp = makeNode(INT, getLexeme());
    advance();
  }
  else if (match(ID))
  {
    retp = makeNode(ID, getLexeme());
    advance();
    //check
    /*for (int i = 0; i < sbcount; i++) //檢查該變數是否存在
    {
      if (strcmp(retp->lexeme, table[i].name) == 0)
      {
        exist_val = 1;
        advance();
        break;
      }
    }
    if (!exist_val)
    {
      error(NOTFOUND);
    }*/
  }
  else if (match(LPAREN))
  {
    advance();
    retp = assign_expr();
    if (match(RPAREN))
      advance();
    else
      error(MISPAREN);
  }
  else if (match(INCDEC))
  { //!自己新增的，會到codeGen中去修改該變數的value
    //check
    adsub = makeNode(ADDSUB, getLexeme());
    adsub->lexeme[1] = '\0'; //因為我們會吃到++或--
    advance();
    if (match(ID))
    {
      retp = makeNode(ASSIGN, "=");
      retp->left = makeNode(ID, getLexeme());
      retp->right = adsub;
      retp->right->left = makeNode(ID, getLexeme());
      retp->right->right = makeNode(INT, "1");
      advance();
    }
    else
      //error(NOTLVAL);
      error(SYNTAXERR);
  }
  else
  {
    error(NOTNUMID);
  }
  return retp;
}

//unary_expr       := ADDSUB unary_expr | factor
//check
BTNode *unary_expr()
{
  BTNode *node = NULL;
  if (match(ADDSUB))
  {
    node = makeNode(ADDSUB, getLexeme());
    node->left = makeNode(INT, "0");
    advance();
    node->right = unary_expr();
  }
  else
    return node = factor();

  return node;
}

//muldiv_expr      := unary_expr muldiv_expr_tail
//check
BTNode *muldiv_expr()
{
  BTNode *node = unary_expr();
  return muldiv_expr_tail(node);
}
//check
//muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL
BTNode *muldiv_expr_tail(BTNode *left)
{
  BTNode *node = NULL;
  if (match(MULDIV))
  {
    node = makeNode(MULDIV, getLexeme());
    advance();
    node->left = left;
    node->right = unary_expr();
    return muldiv_expr_tail(node);
  }
  else
    return left;
}
//check
//addsub_expr      := muldiv_expr addsub_expr_tail
BTNode *addsub_expr()
{
  BTNode *node = muldiv_expr();
  return addsub_expr_tail(node);
}
//check
//addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL
BTNode *addsub_expr_tail(BTNode *left)
{
  BTNode *node = NULL;
  if (match(ADDSUB))
  {
    node = makeNode(ADDSUB, getLexeme());
    advance();
    node->left = left;
    node->right = muldiv_expr();
    return addsub_expr_tail(node);
  }
  else
    return left;
}

//check
//and_expr         := addsub_expr and_expr_tail
BTNode *and_expr()
{
  BTNode *node = addsub_expr();
  return and_expr_tail(node);
}
//check
//and_expr_tail    := AND addsub_expr and_expr_tail | NiL
BTNode *and_expr_tail(BTNode *left)
{
  BTNode *node = NULL;
  if (match(AND))
  {
    node = makeNode(AND, getLexeme());
    advance();
    node->left = left;
    node->right = addsub_expr();
    return and_expr_tail(node);
  }
  else
    return left;
}
//check
//xor_expr_tail    := XOR and_expr xor_expr_tail | NiL
BTNode *xor_expr_tail(BTNode *left)
{
  BTNode *node = NULL;
  if (match(XOR))
  {
    node = makeNode(XOR, getLexeme());
    advance();
    node->left = left;
    node->right = and_expr();
    return xor_expr_tail(node);
  }
  else
    return left;
}
//check
//xor_expr         := and_expr xor_expr_tail
BTNode *xor_expr()
{
  BTNode *node = and_expr();
  return xor_expr_tail(node);
}
//check
//or_expr_tail := OR xor_expr or_expr_tail | NiL
BTNode *or_expr_tail(BTNode *left)
{
  BTNode *node = NULL;
  if (match(OR))
  {
    node = makeNode(OR, getLexeme());
    advance();
    node->left = left;
    node->right = xor_expr();
    return or_expr_tail(node);
  }
  else
    return left;
}
//check
//or_expr  := xor_expr or_expr_tail
BTNode *or_expr(void)
{
  BTNode *node = xor_expr();
  return or_expr_tail(node);
}

//check
void roll_back(char lexeme[])
{
  int len = strlen(lexeme);
  for (int i = len - 1; i > -1; i--)
    ungetc(lexeme[i], stdin);
}

/*
check
If a new variable first appear in the right hand side of an assign (=) , it is invalid and the output should be EXIT 1
- 目前是exit 0
*/
BTNode *assign_expr()
{
  BTNode *node = NULL, *left = NULL;
  if (match(ID))
  {
    left = makeNode(ID, getLexeme());
    advance();
    if (match(ASSIGN))
    {
      node = makeNode(ASSIGN, getLexeme());
      advance();
      // node = assign_expr();
      node->left = left;
      node->right = assign_expr();
      return node;
    }
    else
    {
      roll_back(getLexeme());
      roll_back(left->lexeme);
      advance();
      return or_expr();
    }
  }
  else
    return or_expr();
}

/*
MOV r0 [0]
MOV r1 [4]
MOV r2 [8]
EXIT 0
*/
// statement := ENDFILE | END | expr END
void statement(void)
{
  BTNode *retp = NULL;
  //check
  int res;
  if (match(ENDFILE))
  {
    printf("MOV r0 [0]\n");
    printf("MOV r1 [4]\n");
    printf("MOV r2 [8]\n");
    printf("EXIT 0\n");
    exit(0);
  }
  else if (match(END))
  {
    //printf(">> ");
    advance();
  }
  else
  {
    //check
    retp = assign_expr(); //!
    if (match(END))
    {

      res = evaluateTree(retp, 0); //
      printf("%d\n", res);
      printf("Prefix traversal: ");
      //printPrefix(retp);
      //printf("\n");
      freeTree(retp);
      //printf(">> ");
      advance();
    }
    //!改!
    else
    {
      if (match(ASSIGN))
      {
        error(NOTLVAL);
      }
      else
      {
        error(SYNTAXERR);
      }
    }
  }
}

void err(ErrorType errorNum)
{
  printf("EXIT 1\n");
  if (PRINTERR)
  { //check
    fprintf(stderr, "error: ");
    switch (errorNum)
    {
    case MISPAREN:
      fprintf(stderr, "mismatched parenthesis\n");
      break;
    case NOTNUMID:
      fprintf(stderr, "number or identifier expected\n");
      break;
    case NOTFOUND:
      fprintf(stderr, "variable not defined\n");
      break;
    case RUNOUT:
      fprintf(stderr, "out of memory\n");
      break;
    case NOTLVAL:
      fprintf(stderr, "lvalue required as an operand\n");
      break;
    case DIVZERO:
      fprintf(stderr, "divide by constant zero\n");
      break;
    case SYNTAXERR:
      fprintf(stderr, "syntax error\n");
      break;
    default:
      fprintf(stderr, "undefined error\n");
      break;
    }
  }
  exit(0);
}

TokenSet getToken(void)
{
  int i = 0;
  char c = '\0';

  while ((c = fgetc(stdin)) == ' ' || c == '\t')
    ;

  if (isdigit(c))
  {
    lexeme[0] = c;
    c = fgetc(stdin);
    i = 1;
    if (isalpha(c) || c == '_') //處理ERROR
      return UNKNOWN;
    while (isdigit(c) && i < MAXLEN)
    {
      lexeme[i] = c;
      ++i;
      c = fgetc(stdin);
    }
    ungetc(c, stdin);
    lexeme[i] = '\0';
    return INT;
  }
  else if (c == '+' || c == '-')
  {
    lexeme[0] = c;
    char d = fgetc(stdin);
    if (d == c)
    {
      lexeme[1] = d;
      lexeme[2] = '\0';
      return INCDEC;
    }
    else
    {
      ungetc(d, stdin);
      lexeme[1] = '\0';
      return ADDSUB;
    }
  }
  else if (c == '*' || c == '/')
  {
    lexeme[0] = c;
    lexeme[1] = '\0';
    return MULDIV;
  }
  else if (c == '\n')
  {
    lexeme[0] = c;
    lexeme[1] = '\0';
    return END;
  }
  else if (c == '=')
  {
    strcpy(lexeme, "=");
    return ASSIGN;
  }
  else if (c == '(')
  {
    strcpy(lexeme, "(");
    return LPAREN;
  }
  else if (c == ')')
  {
    strcpy(lexeme, ")");
    return RPAREN;
  }
  else if (isalpha(c) || c == '_')
  {
    //!覺得是要在這邊把更長的變數名吃進去!
    lexeme[0] = c;
    c = fgetc(stdin);
    i = 1;
    while ((isalpha(c) || c == '_' || isdigit(c)) && i < MAXLEN)
    {
      lexeme[i] = c;
      ++i;
      c = fgetc(stdin);
    }
    ungetc(c, stdin); //吃掉不想要的 就吐回去
    lexeme[i] = '\0';
    return ID;
  }
  else if (c == EOF)
  {
    return ENDFILE;
  }
  else if (c == '^')
  {
    lexeme[0] = c;
    lexeme[1] = '\0';
    return XOR;
  }
  else if (c == '&')
  {
    lexeme[0] = c;
    lexeme[1] = '\0';
    return AND;
  }
  else if (c == '|')
  {
    lexeme[0] = c;
    lexeme[1] = '\0';
    return OR;
  }

  else
  {
    return UNKNOWN;
  }
}

void advance(void)
{
  curToken = getToken();
}

int match(TokenSet token)
{
  if (curToken == UNKNOWN)
    advance();
  return token == curToken;
}

char *getLexeme(void)
{
  //printf("cur: %s \n", lexeme);
  return lexeme;
}

int REG[8] = {0};
//check
void free_reg(int i)
{
  if (i >= 0 || i < 8)
    REG[i] = 0;
}
//check
int get_free_reg()
{
  for (int i = 1; i < 8; i++)
  {
    if (REG[i] == 0)
    {
      REG[i] = 1;
      return i;
    }
  }
  return -1;
}

//check

int evaluateTree(BTNode *root, int to_store)
{
  //printf("%s reg", registers[0]);
  int retval = 0, lv = 0, rv = 0, r_store, l_id, r_id;
  //int print_asm = 1;

  if (root != NULL)
  {
    switch (root->data)
    {
    case ID:
      retval = getval(root->lexeme);
      //print_asm = 1;

      printf("MOV r%d [%d]\n", to_store, getAddress(root->lexeme));
      break;
    case INT:
      retval = atoi(root->lexeme);
      //print_asm = 0;

      printf("MOV r%d %d\n", to_store, retval);
      break;

    case ASSIGN:
      //print_asm = 1;

      rv = evaluateTree(root->right, to_store); //回傳右節點的數值
      retval = setval(root->left->lexeme, rv);

      printf("MOV [%d] r%d\n", getAddress(root->left->lexeme), to_store);

      break;
    case AND:
    case OR:
    case XOR:
    case ADDSUB:
    case MULDIV:

      lv = evaluateTree(root->left, to_store);
      r_store = get_free_reg();

      rv = evaluateTree(root->right, r_store);

      if (strcmp(root->lexeme, "+") == 0)
      {
        retval = lv + rv;

        printf("ADD r%d r%d\n", to_store, r_store);
      }
      else if (strcmp(root->lexeme, "-") == 0)
      {
        retval = lv - rv;

        printf("SUB r%d r%d\n", to_store, r_store);
      }
      else if (strcmp(root->lexeme, "*") == 0)
      {
        retval = lv * rv;

        printf("MUL r%d r%d\n", to_store, r_store);
      }
      else if (strcmp(root->lexeme, "/") == 0)
      {
        if (rv == 0)
          if (r_id)
            rv = 1;
          else
            error(DIVZERO);
        retval = lv / rv;

        printf("DIV r%d r%d\n", to_store, r_store);

        //記得處理: 如果"/"右邊沒有變數就不要檢查!
      }
      else if (strcmp(root->lexeme, "&") == 0)
      {
        retval = lv & rv;

        printf("AND r%d r%d\n", to_store, r_store);
      }
      else if (strcmp(root->lexeme, "|") == 0)
      {
        retval = lv | rv;
        //printf("%s", "OR ");

        printf("OR r%d r%d\n", to_store, r_store);
      }
      else if (strcmp(root->lexeme, "^") == 0)
      {
        retval = lv ^ rv;
        //printf("%s", "XOR ");

        printf("XOR r%d r%d\n", to_store, r_store);
      }
      free_reg(r_store);
      break;
      //default:
      //retval = 0;
    }
  }

  return retval;
}

void printPrefix(BTNode *root)
{
  if (root != NULL)
  {
    printf("%s ", root->lexeme);
    printPrefix(root->left);
    printPrefix(root->right);
  }
}

int main()
{
  //freopen("input.txt", "w", stdout);
  initTable();
  //printf(">> ");
  while (1)
  {
    statement();
  }
  return 0;
}

//todo
//1.第八筆錯誤
//2.熟悉codeGen