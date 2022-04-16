#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

/*
1.用cache管理register
2.

*/

/*
* x = 100+10*y
MOV r0 100
MOV r1 10
MOV r2 [4]
MUL r1 r2
ADD r0 r1
MOV [0] r0

y = z+100*10/50*10
MOV r0 [8]
MOV r1 100
MOV r2 10
MUL r1 r2
MOV r2 50
DIV r1 r2
MOV r2 10
MUL r1 r2
ADD r0 r1
MOV [4] r0

z = 10*x/100
MOV r0 10
MOV r1 [0]
MUL r0 r1
MOV r1 100
DIV r0 r1
MOV [8] r0

xx = x
MOV r0 [0]
MOV [12] r0

yy = y
MOV r0 [4]
MOV [16] r0

zz = z
MOV r0 [8]
MOV [20] r0

x = xx ^x
MOV r0 [12]
MOV r1 [0]
XOR r0 r1
MOV [0] r0

y=yy|y
MOV r0 [16]
MOV r1 [4]
OR r0 r1
MOV [4] r0

z = zz&z
MOV r0 [20]
MOV r1 [8]
AND r0 r1
MOV [8] r0

最後收尾
MOV r0 [0]
MOV r1 [4]
MOV r2 [8]
EXIT 0


//觀察:
1. 新的變數從[12]開始宣告，[16]..[20]...[24]...
2. 做一個管理器，把0,1預留給運算 2~7暫存
- 阿因為我們最後會直接把[0] [4] [8]保留給x y z 所以前面用到r0 r1沒關係
3.

//順序:
1.變數先分配記憶體位置
2. 依照運算順序使用指令+分配register
*/

//check
int REG[8] = {0};

//check
void free_reg(int i)
{
    if (i >= 0 && i < 8)
        REG[i] = 0;
}

//check
int get_free_reg()
{
    for (int idx = 1; idx < 8; idx++)
    {
        if (REG[idx] == 0)
        {
            REG[idx] = 1;
            return idx;
        }
    }
    return -1;
}

//check
int has_id(BTNode *root)
{
    if (root == NULL)
        return 0;
    else if (root->data == ID)
        return 1;

    if (has_id(root->left) || has_id(root->right))
        return 1;
    else
        return 0;
}

int evaluateTree(BTNode *root, int to_store, int check_id)
{
    //printf("%s reg", registers[0]);
    int retval = 0, lv = 0, rv = 0, r_store, l_id, r_id;

    //check
    int print_asm = 1;

    if (root != NULL)
    {
        switch (root->data)
        {
        case ID:
            retval = getval(root->lexeme);
            //check
            print_asm = 1;
            if (print_asm)
                printf("MOV r%d [%d]\n", to_store, getAddress(root->lexeme));
            break;
        case INT:
            retval = atoi(root->lexeme);
            //check
            print_asm = 0;
            if (print_asm)
                printf("MOV r%d %d\n", to_store, retval);
            break;

        case ASSIGN:
            //check
            print_asm = 1;
            rv = evaluateTree(root->right, to_store, check_id);
            retval = setval(root->left->lexeme, rv);
            if (print_asm)
                printf("MOV [%d] r%d\n", getAddress(root->left->lexeme), to_store);
            break;
        case AND:
        case OR:
        case XOR:
        case ADDSUB:
        case MULDIV:
            //check
            l_id = has_id(root->left);
            lv = evaluateTree(root->left, to_store, l_id);
            r_store = get_free_reg();
            r_id = has_id(root->right);
            rv = evaluateTree(root->right, r_store, r_id);
            //check
            if (!l_id && print_asm)
                printf("MOV r%d, %d\n", to_store, lv);
            if (!r_id && print_asm)
                printf("MOV r%d, %d\n", r_store, rv);

            if (strcmp(root->lexeme, "+") == 0)
            {
                retval = lv + rv;
                if (print_asm)
                    printf("ADD r%d r%d\n", to_store, r_store);
            }
            else if (strcmp(root->lexeme, "-") == 0)
            {
                retval = lv - rv;
                if (print_asm)
                    printf("SUB r%d r%d\n", to_store, r_store);
            }
            else if (strcmp(root->lexeme, "*") == 0)
            {
                retval = lv * rv;
                if (print_asm)
                    printf("MUL r%d r%d\n", to_store, r_store);
            }
            else if (strcmp(root->lexeme, "/") == 0)
            { //check
                if (rv == 0)
                    if (r_id)
                        rv = 1;
                    else
                        error(DIVZERO);
                retval = lv / rv;
                if (print_asm)
                    printf("DIV r%d r%d\n", to_store, r_store);

                //記得處理: 如果"/"右邊沒有變數就不要檢查!
            }
            else if (strcmp(root->lexeme, "&") == 0)
            {
                retval = lv & rv;
                if (print_asm)
                    printf("AND r%d r%d\n", to_store, r_store);
            }
            else if (strcmp(root->lexeme, "|") == 0)
            {
                retval = lv | rv;
                //printf("%s", "OR ");
                if (print_asm)
                    printf("OR r%d r%d\n", to_store, r_store);
            }
            else if (strcmp(root->lexeme, "^") == 0)
            {
                retval = lv ^ rv;
                //printf("%s", "XOR ");
                if (print_asm)
                    printf("XOR r%d r%d\n", to_store, r_store);
            }
            free_reg(r_store);
            break;
        default:
            retval = 0;
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
