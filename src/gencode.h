#ifndef __CODEGEN_H___
#define __CODEGEN_H___

#define MAX 100
#define MAX_SIG 100
#define MAX_LABEL 10
#define MAX_DIM 10

#include "node.h"
void genCode(struct nodeType* node, char *fname);
#endif
