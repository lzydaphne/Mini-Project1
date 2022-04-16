#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root, int, int);
extern int has_id(BTNode *root);
// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
