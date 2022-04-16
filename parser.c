#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.h"

int sbcount = 0;
Symbol table[TBLSIZE];

//table用來儲存變數!
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

//check
//為了得到空的記憶體地址
//sbcount:變數總數量
int getAddress(char *str)
{
    for (int i = 0; i < sbcount; i++)
    {
        if (strcmp(str, table[i].name) == 0)
            return i * 4;
    }
    return -1;
}

//得到該變數的值
int getval(char *str)
{
    //check 檢驗變數是否存在
    //int exist = 0;
    for (int i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
        {
            //exist = 1;
            //advance();
            return table[i].val;
        }

    //if (!exist)
    error(NOTFOUND);

    //if (sbcount >= TBLSIZE)
    //  error(RUNOUT);
    //strcpy(table[sbcount].name, str);
    //printf("getval str: %s\n", table[sbcount].name);
    //table[sbcount].val = 0;
    //sbcount++;
}
//設定該變數的值
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
//factor           := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN

BTNode *factor(void)
{ //check
    BTNode *retp = NULL, *left = NULL, *adsub = NULL;
    int exist_val = 0; //檢查某變數是否存在

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        advance();
    }
    else if (match(ID))
    {
        retp = makeNode(ID, getLexeme());
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
            error(NOTFOUND);
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
    else if (match(INCDEC)) //check
    {                       //!自己新增的，會到codeGen中去修改該變數的value
        adsub = makeNode(ADDSUB, getLexeme());
        adsub->lexeme[1] = '\0'; //因為我們會吃到++或--
        advance();
        if (match(ID))
        {
            /* BTNode* temp = NULL;
            temp = makeNode(ID, getLexeme());
            for (int i = 0; i < sbcount; i++)//檢查該變數是否存在
            {
                if (strcmp(temp->lexeme , table[i].name) == 0)
                {
                    exist_val = 1;
                    advance();
                    break;
                }
            }
            if (!exist_val)
                error(NOTFOUND);*/
        retp = makeNode(ASSIGN, "=");
        retp->left = makeNode(ID, getLexeme());
        retp->right = adsub;
        retp->right->left = makeNode(ID, getLexeme());
        retp->right->right = makeNode(INT, "1");
        advance();
    }
    else
        error(NOTLVAL);
}
else
{
    error(NOTNUMID);
}
return retp;
}

//unary_expr       := ADDSUB unary_expr | factor
BTNode *unary_expr()
{ //check
    BTNode *node = NULL;
    if (match(ADDSUB))
    {
        node = makeNode(ADDSUB, getLexeme());
        advance();
        node->left = makeNode(INT, "0");
        node->right = unary_expr();
    }
    else
        return node = factor();

    return node;
}
//check
//muldiv_expr      := unary_expr muldiv_expr_tail
BTNode *muldiv_expr()
{
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}
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

//addsub_expr      := muldiv_expr addsub_expr_tail
BTNode *addsub_expr()
{
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}
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

//and_expr         := addsub_expr and_expr_tail
BTNode *and_expr()
{
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}
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
//xor_expr         := and_expr xor_expr_tail
BTNode *xor_expr()
{
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

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
TODO:
If a new variable first appear in the right hand side of an assign (=) , it is invalid and the output should be EXIT 1
- 目前是exit 0
*/
//check
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

    if (match(ENDFILE))
    { //check
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
            printf("%d\n", evaluateTree(retp, 0, has_id(retp)));
            //evaluateTree(retp, 0,has_id(retp));//最後答案存在0
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
        }
        else
        {
            error(SYNTAXERR);
        }
    }
}

void err(ErrorType errorNum)
{
    if (PRINTERR)
    {
        //check
        printf("EXIT 1\n");
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
