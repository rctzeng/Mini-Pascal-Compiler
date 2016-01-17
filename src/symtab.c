#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DSRPT_MAX 100

struct SymTable SymbolTable;
int depth; int global = 0;
int par = 0;

/* main functions */
void openScope() { SymbolTable.scope_num++; depth = SymbolTable.scope_num; if(depth >= DEP_MAX) {printf("[ERROR] Scope Depth Limit Exhausted\n"); SymbolTable.error = 1; exit(0);} SymbolTable.scopes[depth] = -1;}
/* void deleteEntry(int index) {SymbolTable.entries[index].valid = 0; SymbolTable.entries[index].next = -1;} */
void closeScope() { depth = 0;}
int findNextFreeEntry() { for(int i=0; i<SYM_MAX; i++) if(SymbolTable.entries[i].valid == 0) return i; return -1;}
int findSymbolAll(char *s, int dep) { // current -> global scope
  int index = SymbolTable.scopes[dep];
  while(index != -1) { if(strcmp(s, SymbolTable.entries[index].name) == 0) return index; index = SymbolTable.entries[index].next;}
  if(dep > 0) return findSymbolAt(s, 0);
  return -1;
}
int findSymbolAt(char *s, int dep) {
  if(s == NULL) {printf("[ERROR] Referencing NULL names in SymbolgTable\n"); SymbolTable.error = 1; return -1;}
  int index = SymbolTable.scopes[dep];
  while(index != -1) { if(strcmp(s, SymbolTable.entries[index].name) == 0) return index; index = SymbolTable.entries[index].next; }
  return -1;
}
int findSymbol(char *s) {int index = SymbolTable.scopes[depth]; while(index != -1) { if(strcmp(s, SymbolTable.entries[index].name) == 0) return index; index = SymbolTable.entries[index].next; } return -1;}
struct SymTableEntry* addVariable(char *s, enum StdType type) {
  int exist = findSymbol(s);
  if(exist != -1) {printf("[ERROR] Duplicate Declaration of Variable %s\n", s);SymbolTable.error = 1;return &SymbolTable.entries[exist];}
  int index = findNextFreeEntry();
  if(index == -1) {printf("[ERROR] SymbolTable is Full\n"); SymbolTable.error = 1; exit(0);}
  strcpy(SymbolTable.entries[index].name, s); SymbolTable.entries[index].type = type; SymbolTable.entries[index].valid = 1;
  // associate var to scope
  SymbolTable.entries[index].depth = depth; SymbolTable.entries[index].defined = 0;
  int head = SymbolTable.scopes[depth];
  SymbolTable.scopes[depth] = index; SymbolTable.entries[index].next = head; SymbolTable.entries[index].typeDsrpt = NULL;
  SymbolTable.entries[index].index = -1; // used in Jasmine
  return &SymbolTable.entries[index];
}
struct nodeType* nthChild(int n, struct nodeType *node) { struct nodeType *child = node->child; for(int i=1; i<n; i++) child = child->rsibling; return child; }
void setError(struct nodeType *node) {node->nodeType = NODE_ERROR; SymbolTable.error = 1;}
struct DsrptType* copyType(struct SymTableEntry *sym) {
  struct DsrptType *tp = (struct DsrptType *)malloc(sizeof(struct DsrptType));
  tp->type = sym->type; tp->typeDsrpt = sym->typeDsrpt; return tp;
}
char* translateType(int type) {
  switch(type) {
  case TypeInt: return "Int"; case TypeReal: return "Real"; case TypeChar: return "String";
  case TypeBool: return "Bool"; case TypeArray: return "Array"; case TypeFun: return "Function";
  case TypeProc: return "Procedure"; case TypeLabel: return "Label"; case TypeType: return "Type";
  default: return "NULL";
  }
}
int getDimension(struct ArrayDsrpt * A) {
  switch(A->type) {
  case TypeInt:
  case TypeReal:
  case TypeChar: return 1;
  case TypeArray: return getDimension(A->typeDsrpt)+1;
  default: { printf("[ERROR] Not Supported Array Type \"%s\"\n", translateType(A->type)); return -1;}
  }
}
struct ArrayDsrpt* generateArrayDsrpt(struct nodeType *node) {
  struct nodeType *range = nthChild(1, node); struct nodeType *type = nthChild(2, node);
  struct nodeType *left = nthChild(1, range); struct nodeType *right = nthChild(2, range);
  struct ArrayRange *lower = malloc(sizeof(struct ArrayRange)); struct ArrayRange *upper = malloc(sizeof(struct ArrayRange));
  if(left->nodeType == NODE_INT) { lower->value = left->iValue; lower->defined = 1; } else if(left->nodeType == NODE_VAR) {
    int index = findSymbol(left->string);
    if(index == -1) { printf("[ERROR] Array range lower bound %s not declared\n", left->string); setError(node); return NULL; }
    if(SymbolTable.entries[index].type != TypeInt) { printf("[ERROR] Array range lower bound %s is not \"Integer\"\n", left->string); setError(node); return NULL; }
    strcpy(lower->name, left->string); lower->defined = 0;
  } else { printf("[ERROR] Invalid Array range type %d\n", left->nodeType); setError(node); return NULL; }
  if(right->nodeType == NODE_INT) { upper->value = right->iValue; upper->defined = 1;
  } else if(right->nodeType == NODE_VAR) {
    int index = findSymbol(right->string);
    if(index == -1) { printf("[ERROR] Array range upper bound %s not declared\n", right->string); setError(node); return NULL; }
    if(SymbolTable.entries[index].type != TypeInt) { printf("[ERROR] Array range upper bound %s is not \"Integer\"\n", right->string); setError(node); return NULL; }
    strcpy(upper->name, right->string); upper->defined = 0;
  } else { printf("[ERROR] Invalid Array range type %d\n", right->nodeType); setError(node); return NULL; }
  struct ArrayDsrpt *ret = malloc(sizeof(struct ArrayDsrpt));
  ret->lowerBound = lower; ret->upperBound = upper;
  // set type
  if(type->nodeType != NODE_TYPE_ARRAY) {
    switch(type->nodeType) {
    case NODE_TYPE_INT: {ret->type = TypeInt; ret->typeDsrpt = NULL;break;}
    case NODE_TYPE_REAL: {ret->type = TypeReal; ret->typeDsrpt = NULL;break;}
    case NODE_TYPE_CHAR: {ret->type = TypeChar; ret->typeDsrpt = NULL;break;}
    case NODE_TOKEN: {
      int index = findSymbolAt(node->string, global);
      if(index == -1) { printf("[ERROR] Undefined type \"%s\"\n", type->string); setError(node); return NULL; }
      ret->type = TypeType; ret->typeDsrpt = SymbolTable.entries[index].typeDsrpt; break;
    }
    default: {printf("[ERROR] Not Supported type %d for Array\n", type->nodeType); setError(node); return NULL; }
    }
    ret->dimension = getDimension(ret); return ret;
  }
  ret->type = TypeArray; ret->typeDsrpt = generateArrayDsrpt(type); ret->dimension = getDimension(ret); return ret;
}
enum StdType getTypeType(struct DsrptType *typeD) {
  switch(typeD->type) {
  case TypeInt:
  case TypeReal:
  case TypeChar:
  case TypeArray: return typeD->type;
  case TypeType: return getTypeType(typeD->typeDsrpt);
  }
}
enum StdType getType(struct nodeType *node) {
  switch(node->nodeType) {
  case NODE_TYPE_INT: return TypeInt;
  case NODE_TYPE_REAL: return TypeReal;
  case NODE_TYPE_CHAR: return TypeChar;
  case NODE_TYPE_ARRAY: return TypeArray;
  case NODE_TOKEN: {
    int index = findSymbolAt(node->string, global);
    if(index == -1) return TypeNull;
    if(SymbolTable.entries[index].type == TypeType) return TypeType;
  }
  default: return TypeNull;
  }
}
void* getDsrpt(struct nodeType *node) {
  switch(node->nodeType) { case NODE_TYPE_ARRAY: return generateArrayDsrpt(node);
  case NODE_TOKEN: { int index = findSymbolAt(node->string, global); return SymbolTable.entries[index].typeDsrpt;} }
}
int checkType(char *s, enum StdType type) { int index = findSymbolAll(s, depth); if(index != -1) if(SymbolTable.entries[index].type == type) return 1; return 0;}
enum StdType checkRange(struct nodeType *node) {
  if(node->child == NULL) { int ret = checkType(node->string, TypeArray); if(ret == 1) return TypeArray; else { printf("[ERROR] \"%s\" is not an Array\n", node->string); setError(node); return TypeNull; } }
  struct nodeType *name = nthChild(1, node);
  struct nodeType *dList = nthChild(2, node);
  if(dList->nodeType != NODE_LIST) { int ret = checkType(node->string, TypeArray); if(ret == 1) return TypeArray; else { printf("[ERROR] \"%s\" is not an Array\n", node->string); setError(node); return TypeNull; }}
  semanticCheck(dList);
  enum StdType t = TypeNull;
  int index = findSymbolAll(name->string, depth);
  struct ArrayDsrpt *arrayD = SymbolTable.entries[index].typeDsrpt; struct nodeType *idNode = dList->child;
  // check dimension first and reverse order
  struct nodeType **nodes = malloc(sizeof(struct nodeType *)*PAR_MAX);
  int paraN; int i = 0;
  do { nodes[i] = idNode; i++; idNode = idNode->rsibling; } while(idNode != dList->child); paraN = i;
  if(paraN != getDimension(arrayD)) { printf("[ERROR] Mismatched Array \"%s\" dimension, expected %d but %d dimensions\n", name->string, getDimension(arrayD), paraN); setError(node); return TypeNull; }
  for(i=paraN-1; i>=0; i--) {
    if(nodes[i]->valueType == TypeArray) nodes[i]->valueType = checkRange(nodes[i]);
    if(nodes[i]->valueType != TypeInt) { printf("[ERROR] Array Index must be integer, but \"%s\"\n", translateType(nodes[i]->valueType)); setError(node); return TypeNull; }
    if(nodes[i]->valueType == TypeInt && nodes[i]->defined == 1) { // check bound, debug
      if(arrayD->lowerBound->defined == 1 && (arrayD->lowerBound->value > nodes[i]->iValue)) { printf("[ERROR] Out of Lower Bound Referencing array \"%s\" at dimension %d = %d\n", node->string, paraN-1-i, nodes[i]->iValue); setError(node); return TypeNull; }
      if(arrayD->upperBound->defined == 1 && (arrayD->upperBound->value < nodes[i]->iValue)) { printf("[ERROR] Out of Upper Bound Referencing array \"%s\" at dimension %d = %d\n", node->string, paraN-1-i, nodes[i]->iValue); setError(node); return TypeNull; }
      
      // fix later
      SymbolTable.entries[index].defined = 2;
    } else { // check runtime not implemented 
      node->defined = 2; SymbolTable.entries[index].defined = 2;
    }
    t = arrayD->type; arrayD = arrayD->typeDsrpt;
  }
  while(arrayD != NULL) { t = arrayD->type; arrayD = arrayD->typeDsrpt;}
  return t;
}
int checkInitialize(struct nodeType *node) {
  if(node->defined == 0 && node->string == NULL) return 0;
    if(node->string != NULL && node->valueType != TypeChar) {
      int index = findSymbolAt(node->string, depth);
      if(index != -1) { if(SymbolTable.entries[index].defined == 0) return 0; return SymbolTable.entries[index].defined;}
      if(index == -1) { if(!(-1 != findSymbolAt(node->string, global) && depth > 0)) return 0; return 2;}
    }
    return node->defined;
}
enum StdType refArrayType(struct nodeType *node, struct ArrayDsrpt *dsrpt) {
  struct nodeType *dList = nthChild(2, node);
  if(dList == node->child) return TypeArray;
  struct nodeType *idNode = dList->child; struct nodeType **nodes = malloc(sizeof(struct nodeType *)*10);
  int paraN; int i = 0; do { nodes[i] = idNode; i++; idNode = idNode->rsibling; } while(idNode != dList->child); paraN = i;
  enum StdType t; struct ArrayDsrpt *arrayD = dsrpt; 
  for(i=paraN-1; i>=0; i--) {
    t = arrayD->type; arrayD = arrayD->typeDsrpt;
  }
  return t;
}
/* display symboltable */
char* intToStr(int n) { char *str = malloc(sizeof(char)*DSRPT_MAX); sprintf(str, "%d", n); return str;}
char* realToStr(double n) { char *str = malloc(sizeof(char)*DSRPT_MAX); sprintf(str, "%.3lf", n); return str;}
char* displayArray(struct ArrayDsrpt *dsrpt) {
  char *s = (char *)malloc(sizeof(char)*DSRPT_MAX); s[0] = '\0'; strcat(s, "[");
  if(dsrpt->lowerBound->defined == 0) strcat(s, dsrpt->lowerBound->name); else strcat(s, intToStr(dsrpt->lowerBound->value));
  strcat(s, "..");
  if(dsrpt->upperBound->defined == 0) strcat(s, dsrpt->upperBound->name); else strcat(s, intToStr(dsrpt->upperBound->value));
  strcat(s, "]");
  if(dsrpt->type != TypeArray) {strcat(s, translateType(dsrpt->type)); return s;}
  strcat(s, displayArray(dsrpt->typeDsrpt));
  return s;
}
char* displayType(void *type) {
  struct DsrptType *t = (struct DsrptType *) type;
  switch(t->type) {
  case TypeInt:
  case TypeReal:
  case TypeChar: return translateType(t->type);
  case TypeArray: return displayArray(t->typeDsrpt);
  case TypeType: return displayType(t->typeDsrpt);
  default: return "";
  }
}
char* transTypeDsrpt(enum StdType type, void *dsrpt) {
  if(dsrpt == NULL) return "";
  switch(type) {
  case TypeArray: return displayArray(dsrpt);
  case TypeType: return displayType(dsrpt);
  case TypeFun: {
    char *s = (char *)malloc(sizeof(char)*DSRPT_MAX); s[0] = '\0';
    struct FunctionDsrpt d = *(struct FunctionDsrpt *)dsrpt;
    strcat(s, translateType(d.retType->type)); strcat(s, ": ");
    for(int i=0; i<d.paramNum; i++) {if(i>0) strcat(s, ","); strcat(s, translateType(d.paramType[i]->type));}
    return s;
  }
  case TypeProc: {
    char *s = (char *)malloc(sizeof(char)*DSRPT_MAX); s[0] = '\0';
    struct ProcedureDsrpt d = *(struct ProcedureDsrpt *)dsrpt;
    for(int i=0; i<d.paramNum; i++) {if(i>0) strcat(s, ","); strcat(s, translateType(d.paramType[i]->type));}
    return s;
  }
  default: return "";
  }
}
char* displayValue(enum StdType type, void *dsrpt) {
  if(dsrpt == NULL) return "";
  switch(type) {
  case TypeInt: return intToStr(*(int *)dsrpt);
  case TypeReal: return realToStr(*(double *)dsrpt);
    // case TypeChar: return dsrpt;
  case TypeFun: return intToStr(((struct FunctionDsrpt *)dsrpt)->scope);
  case TypeProc: return intToStr(((struct ProcedureDsrpt *)dsrpt)->scope);
  default: return "";
  }
}
void printSymTable(int dep) {
  fprintf(stderr, "----------%s[scope %d]----------\n", SymbolTable.name[dep], dep); fprintf(stderr, "|%-10s|%-10s|%-18s|%-10s|%-10s|\n", "___name___", "___type___", "___descriptor___", "___value___", "___index___");
  int index = SymbolTable.scopes[dep];
  while(index != -1) {
    if(SymbolTable.entries[index].valid == 1) fprintf(stderr, "|%-10s|%-10s|%-18s|%-10s|%-10s|\n", SymbolTable.entries[index].name, translateType(SymbolTable.entries[index].type), transTypeDsrpt(SymbolTable.entries[index].type, SymbolTable.entries[index].typeDsrpt), displayValue(SymbolTable.entries[index].type, SymbolTable.entries[index].typeDsrpt), intToStr(SymbolTable.entries[index].index));
    index = SymbolTable.entries[index].next;
  }
  fprintf(stderr, "------------------------------\n");
}
/* printInt, printReal */
void loadDefaultProcedure() {
  struct SymTableEntry *entry = addVariable("printInt", TypeProc);
  struct ProcedureDsrpt *dsrpt = (struct ProcedureDsrpt *)malloc(sizeof(struct ProcedureDsrpt));
  struct DsrptType *type = (struct DsrptType *)malloc(sizeof(struct DsrptType));
  type->type = TypeInt; type->typeDsrpt = NULL; dsrpt->paramType[0] = type; dsrpt->paramNum = 1; entry->typeDsrpt = dsrpt;
  entry = addVariable("printReal", TypeProc);
  dsrpt = (struct ProcedureDsrpt *)malloc(sizeof(struct ProcedureDsrpt));
  type = (struct DsrptType *)malloc(sizeof(struct DsrptType));
  type->type = TypeReal; type->typeDsrpt = NULL; dsrpt->paramType[0] = type; dsrpt->paramNum = 1; entry->typeDsrpt = dsrpt;
  entry = addVariable("printString", TypeProc);
  dsrpt = (struct ProcedureDsrpt *)malloc(sizeof(struct ProcedureDsrpt));
  type = (struct DsrptType *)malloc(sizeof(struct DsrptType));
  type->type = TypeChar; type->typeDsrpt = NULL; dsrpt->paramType[0] = type; dsrpt->paramNum = 1; entry->typeDsrpt = dsrpt;
}
void initSymTable() {
  SymbolTable.error = 0; depth = 0; SymbolTable.scope_num = 0;
  char **names = (char **)malloc(sizeof(char *)*DEP_MAX);
  SymbolTable.name = names;
  for(int i=0; i<SYM_MAX; i++) {SymbolTable.entries[i].valid = 0; SymbolTable.entries[i].next = -1;}
  for(int i=0; i<DEP_MAX; i++) SymbolTable.scopes[i] = -1;
  loadDefaultProcedure();
}
void reverseSymbols(int scope) {
  int index = SymbolTable.scopes[scope];
  int symN; int i = 0;
  int *stack = malloc(sizeof(int)*SYM_MAX);
  while(index != -1) {
    stack[i] = index; i++;
    index = SymbolTable.entries[index].next;
  } symN = i;
  SymbolTable.scopes[scope] = stack[symN-1];
  for(i=symN-1; i>0; i--) SymbolTable.entries[stack[i]].next = stack[i-1];
  SymbolTable.entries[stack[0]].next = -1;
}

/* core semantic checking */
void semanticCheck(struct nodeType *node) {
  switch(node->nodeType) { 
  case NODE_ERROR: { break; } // error handling
  case NODE_PROGRAM: { 
    initSymTable(); 
    struct nodeType *nameNode = nthChild(1, node);
    SymbolTable.name[depth] = nameNode->string;
    break; }
  case NODE_SUBPROG: {
    openScope(); depth = SymbolTable.scope_num;
    struct nodeType *head = nthChild(1, node); semanticCheck(head);
    struct nodeType *var = nthChild(2, node); struct nodeType *compound = nthChild(3, node);
    semanticCheck(var);
    reverseSymbols(SymbolTable.scope_num);
    semanticCheck(compound);
    closeScope(); return;
  }
  case NODE_FUNCTION: { // return ID = funciton ID
    struct nodeType *ret = nthChild(1, node); struct nodeType *ret_type = nthChild(3, node);
    SymbolTable.name[SymbolTable.scope_num] = ret->string;
    struct nodeType *parList = nthChild(2, node);
    if(parList->nodeType != NODE_EMPTY) {par=1; semanticCheck(parList); par=0;}// handle no parameter case
    struct SymTableEntry *f_type = addVariable(ret->string, getType(ret_type));
    /* add function to global scope */
    depth = 0;
    struct SymTableEntry *entry = addVariable(ret->string, TypeFun);
    struct FunctionDsrpt *dsrpt = (struct FunctionDsrpt *)malloc(sizeof(struct FunctionDsrpt));
    dsrpt->retType = copyType(f_type); dsrpt->paramNum = 0; dsrpt->scope = SymbolTable.scope_num;
    if(parList->nodeType != NODE_EMPTY) {
      // reverse order
      struct nodeType **nodes = malloc(sizeof(struct nodeType *)*PAR_MAX); struct nodeType *idNode = parList->child; int i=0;
      do {nodes[i] = idNode; i++; idNode = idNode->rsibling;} while(idNode != parList->child);
      int Types = i;
      for(i=Types-1; i>=0; i--) { 
	idNode = nodes[i];struct nodeType *nameList = nthChild(1, idNode); struct nodeType *nameNode = nthChild(1, nameList);
	do {
	  dsrpt->paramType[dsrpt->paramNum] = copyType(&SymbolTable.entries[findSymbolAt(nameNode->string, SymbolTable.scope_num)]);
	  dsrpt->paramNum = dsrpt->paramNum + 1; nameNode = nameNode->rsibling;
	} while(nameNode != nameList->child);
      }
    }
    entry->defined = 2; entry->typeDsrpt = dsrpt; depth = SymbolTable.scope_num; return;
  }
  case NODE_PROCEDURE: {
    struct nodeType *ret = nthChild(1, node); struct nodeType *parList = nthChild(2, node);
    if(parList->nodeType != NODE_EMPTY) {par=1; semanticCheck(parList); par=0; }// handle no parameter case
    /* add procedure to global scope */
    depth = 0;
    struct SymTableEntry *entry = addVariable(ret->string, TypeProc); SymbolTable.name[SymbolTable.scope_num] = ret->string;
    struct ProcedureDsrpt *dsrpt = (struct ProcedureDsrpt *)malloc(sizeof(struct ProcedureDsrpt));
    dsrpt->paramNum = 0; dsrpt->scope = SymbolTable.scope_num;
    if(parList->nodeType != NODE_EMPTY) {
      // reverse order
      struct nodeType **nodes = malloc(sizeof(struct nodeType *)*PAR_MAX); struct nodeType *idNode = parList->child; int i=0;
      do {nodes[i] = idNode; i++;idNode = idNode->rsibling;} while(idNode != parList->child);
      int Types = i;
      for(i=Types-1; i>=0; i--) {
	idNode = nodes[i]; struct nodeType *nameList = nthChild(1, idNode); struct nodeType *nameNode = nthChild(1, nameList);
	do {
	  dsrpt->paramType[dsrpt->paramNum] = copyType(&SymbolTable.entries[findSymbolAt(nameNode->string, SymbolTable.scope_num)]);
	  dsrpt->paramNum = dsrpt->paramNum + 1; nameNode = nameNode->rsibling;
	} while(nameNode != nameList->child);
      }
    }
    entry->typeDsrpt = dsrpt; depth = SymbolTable.scope_num; return;
  }
  case NODE_VAR_DECL: {
    struct nodeType *typeNode = nthChild(2, node); enum StdType valueType;
    void *dsrpt = NULL;
    switch(typeNode->nodeType) {
    case NODE_TYPE_INT: valueType = TypeInt; break;  case NODE_TYPE_REAL: valueType = TypeReal; break; case NODE_TYPE_CHAR: valueType = TypeChar; break; case NODE_TYPE_ARRAY: { valueType = TypeArray; dsrpt = generateArrayDsrpt(typeNode); break; }
    case NODE_TOKEN: {
      int index = findSymbolAt(typeNode->string, global);
      if(index == -1) { printf("[ERROR] Undefined type \"%s\" at variable \"%s\" declaration\n", typeNode->string, node->string); setError(node); return; }
      valueType = getTypeType(SymbolTable.entries[index].typeDsrpt);
      dsrpt = SymbolTable.entries[index].typeDsrpt; break;
    }
    case NODE_ERROR: break;
    default: {printf("[ERROR] Type %d not implemented\n", typeNode->nodeType); setError(node); return;}
    }
    struct nodeType *idList = nthChild(1, node); struct nodeType *idNode = idList->child;
    int defined = 0;
    if(par == 1) defined = 2;
    do { struct SymTableEntry *entry = addVariable(idNode->string, valueType); entry->typeDsrpt = dsrpt; entry->defined = defined; idNode = idNode->rsibling; idNode->defined = defined; } while(idNode != idList->child); break;
  }
  case NODE_LABEL_DECL: {
    struct nodeType *labelList = nthChild(1, node); struct nodeType *idNode = labelList->child; semanticCheck(labelList);
    do {
      if(idNode->valueType != TypeLabel) {printf("[ERROR] Illegal Label Declaration\n"); setError(node); return;}
      addVariable(intToStr(idNode->iValue), TypeLabel); idNode = idNode->rsibling;
    } while(idNode != labelList->child);
    return;
  }
  case NODE_TYPE_DECL: {
    struct nodeType *name = nthChild(1, node); struct nodeType *type = nthChild(2, node);
    enum StdType t = getType(type);
    switch(t) {
    case TypeInt:
    case TypeReal:
    case TypeChar: { struct SymTableEntry *entry = addVariable(name->string, TypeType); struct DsrptType *td = malloc(sizeof(struct DsrptType)); td->type = t; td->typeDsrpt = NULL; entry->typeDsrpt = td; return; }
    case TypeArray: { struct SymTableEntry *entry = addVariable(name->string, TypeType); struct DsrptType *td = malloc(sizeof(struct DsrptType)); td->type = t; td->typeDsrpt = generateArrayDsrpt(node); entry->typeDsrpt = td; return; }
    case TypeType: { struct SymTableEntry *entry = addVariable(name->string, TypeType); struct DsrptType *td = malloc(sizeof(struct DsrptType)); int index = findSymbolAt(node->string, global); td->type = t; td->typeDsrpt = SymbolTable.entries[index].typeDsrpt;  entry->typeDsrpt = td; return; }
    } return;
  }
  case NODE_VAR_OR_PROC: {
    if(node->nodeType == NODE_ERROR) return;
    int index = findSymbolAll(node->string, depth);
    if(index == -1) { printf("[ERROR] Undeclared Variable \"%s\"\n", node->string); setError(node); return; }
    switch(SymbolTable.entries[index].type) {
    case TypeFun: {
      if(((struct FunctionDsrpt *) SymbolTable.entries[index].typeDsrpt)->paramNum > 0) {printf("[ERROR] Calling function \"%s\" with no parameter\n", node->string); setError(node); return; }
      node->valueType = ((struct FunctionDsrpt *) SymbolTable.entries[index].typeDsrpt)->retType->type; node->defined = 2; return;
    }
    case TypeInt: { if(SymbolTable.entries[index].defined == 1) node->iValue = *((int *)SymbolTable.entries[index].typeDsrpt); node->valueType = SymbolTable.entries[index].type; return; }
    case TypeReal: { if(SymbolTable.entries[index].defined == 1) node->rValue = *((double *)SymbolTable.entries[index].typeDsrpt); node->valueType = SymbolTable.entries[index].type; return; }
    case TypeChar: case TypeArray: { node->valueType = SymbolTable.entries[index].type; return; }
    case TypeProc: { return; }
    default: { fprintf(stderr, "[ERROR] Unimplemented type number = %d\n", SymbolTable.entries[index].type); node->valueType = SymbolTable.entries[index].type; return; }
    }
  }
  case NODE_SYM_REF: {
    if(node->nodeType == NODE_ERROR) return;
    int index = findSymbolAll(node->string, depth);
    if(index == -1) { printf("[ERROR] Undeclared Variable \"%s\"\n", node->string); setError(node); return; }
    node->entry = &SymbolTable.entries[index];
    switch(SymbolTable.entries[index].type) {
    case TypeInt: case TypeReal: case TypeChar: { node->valueType = SymbolTable.entries[index].type; break; } 
    case TypeArray: { node->valueType = refArrayType(node, SymbolTable.entries[index].typeDsrpt); break; }
    }
    node->defined = SymbolTable.entries[index].defined; return;
  }
  case NODE_OP: {
    switch(node->op) {
    case OP_ADD: // int, real -> same
    case OP_SUB: 
    case OP_MUL:
    case OP_DIV: {
      struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node);
      semanticCheck(Lexp); semanticCheck(Rexp); Lexp->defined = checkInitialize(Lexp); Rexp->defined = checkInitialize(Rexp);
      if(Lexp->nodeType == NODE_ERROR || Rexp->nodeType == NODE_ERROR) return; // error handling
      if(Lexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Lexp->string); setError(node); return; }
      if(Rexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Rexp->string); setError(node); return; }
      if(Lexp->valueType == TypeArray) Lexp->valueType = checkRange(Lexp);
      if(Rexp->valueType == TypeArray) Rexp->valueType = checkRange(Rexp);
      if(Lexp->valueType != Rexp->valueType) { 	printf("[ERROR] Mismatch ValueType for Expressions: left-op is \"%s\", and right-op is \"%s\"\n", translateType(Lexp->valueType), translateType(Rexp->valueType)); setError(node); return; }
      if(Lexp->defined == 0) {printf("[ERROR] left-op is not initialized\n"); setError(node); return;}
      if(Rexp->defined == 0) {printf("[ERROR] right-op is not initialized\n"); setError(node); return;}
      if(Lexp->defined == 1 && Rexp->defined == 1) {
	switch(Lexp->valueType) { 
	case TypeInt: { switch(node->op) {
	  case OP_ADD: node->iValue = Lexp->iValue + Rexp->iValue; break; case OP_SUB: node->iValue = Lexp->iValue - Rexp->iValue; break;
	  case OP_MUL: node->iValue = Lexp->iValue * Rexp->iValue; break; case OP_DIV: node->iValue = Lexp->iValue / Rexp->iValue; break;
	  } break; }
	case TypeReal: { switch(node->op) {
	  case OP_ADD: node->rValue = Lexp->rValue + Rexp->rValue; break; case OP_SUB: node->rValue = Lexp->rValue - Rexp->rValue; break;
	  case OP_MUL: node->rValue = Lexp->rValue * Rexp->rValue; break; case OP_DIV: node->rValue = Lexp->rValue / Rexp->rValue; break;
	  } break;
	}
	default: { printf("[ERROR] \"SUB/MUL/DIV\" must operate on INTEGER or REAL\n"); setError(node); return; }
	}
	node->defined = 1;
      } else node->defined = 2;
      node->valueType = Lexp->valueType; return;
    }
    case OP_GT: // int, real -> bool
    case OP_LT:
    case OP_GE: 
    case OP_LE: {
      struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node);
      semanticCheck(Lexp); semanticCheck(Rexp); Lexp->defined = checkInitialize(Lexp); Rexp->defined = checkInitialize(Rexp);
      if(Lexp->nodeType == NODE_ERROR || Rexp->nodeType == NODE_ERROR) return; // error handling
      if(Lexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Lexp->string); setError(node); return; }
      if(Rexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Rexp->string); setError(node); return; }
      if(Lexp->valueType == TypeArray) Lexp->valueType = checkRange(Lexp);
      if(Rexp->valueType == TypeArray) Rexp->valueType = checkRange(Rexp);
      if(Lexp->valueType != Rexp->valueType) { 	printf("[ERROR] Mismatch ValueType for Expressions: left-op is \"%s\", and right-op is \"%s\"\n", translateType(Lexp->valueType), translateType(Rexp->valueType)); setError(node); return; }
      if(Lexp->defined == 0) {printf("[ERROR] left-op is not initialized\n"); setError(node); return;}
      if(Rexp->defined == 0) {printf("[ERROR] right-op is not initialized\n"); setError(node); return;}
      if(Lexp->defined == 1 && Rexp->defined == 1) {
	switch(Lexp->valueType) { 
	case TypeInt: { switch(node->op) { case OP_GT: node->iValue = (Lexp->iValue > Rexp->iValue)?1:0; break; case OP_GE: node->iValue = (Lexp->iValue >= Rexp->iValue)?1:0; break; case OP_LT: node->iValue = (Lexp->iValue < Rexp->iValue)?1:0; break; case OP_LE: node->iValue = (Lexp->iValue <= Rexp->iValue)?1:0; break; } break; }
	case TypeReal: { switch(node->op) { case OP_GT: node->iValue = (Lexp->rValue > Rexp->rValue)?1:0; break; case OP_GE: node->iValue = (Lexp->rValue >= Rexp->rValue)?1:0; break; case OP_LT: node->iValue = (Lexp->rValue < Rexp->rValue)?1:0; break; case OP_LE: node->iValue = (Lexp->rValue <= Rexp->rValue)?1:0; break; } break;
	}
	default: {printf("[ERROR] \"GT/LT/GE/LE\" must operate on INTEGER or REAL\n"); setError(node); return;}
	}
	node->defined = 1;
      } else node->defined = 2;
      node->valueType = TypeBool; return;
    }
    case OP_EQ: // int, real, char, bool, array -> bool
    case OP_NE: { 
      struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node);
      semanticCheck(Lexp); semanticCheck(Rexp); Lexp->defined = checkInitialize(Lexp); Rexp->defined = checkInitialize(Rexp);
      if(Lexp->defined == 0) { printf("[ERROR] left-op is not initialized\n"); setError(node); return; }
      if(Rexp->defined == 0) { printf("[ERROR] reft-op is not initialized\n"); setError(node); return; }
      if(Lexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Lexp->string); setError(node); return; }
      if(Rexp->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", Rexp->string); setError(node); return; }
      if(Lexp->valueType == TypeArray) Lexp->valueType = checkRange(Lexp);
      if(Rexp->valueType == TypeArray) Rexp->valueType = checkRange(Rexp);
      if(Lexp->nodeType == NODE_ERROR || Rexp->nodeType == NODE_ERROR) break;
      if(Lexp->valueType != Rexp->valueType) {printf("[ERROR] Mismatch ValueType for left/right Expressions\n"); setError(node); return;}
      // evaluate value not implement
      node->valueType = TypeBool; node->defined = 1; return;
    }
      /* fix later */
    case OP_NOT: { // bool -> bool
      struct nodeType *child = nthChild(1, node); semanticCheck(child);
      if(child->valueType == TypeProc) { printf("[ERROR] Procedure \"%s\" cannot appear in expression\n", child->string); setError(node); return; }
      if(child->defined == 0) { printf("[ERROR] Not-op is not initialized\n"); setError(node); return; }
      if(child->nodeType == NODE_ERROR) { break; } else if(child->valueType != TypeBool) {printf("[ERROR] \"NOT\" operation with non-Boolean value\n");setError(node); return;}
      // evaluate value not implement
      node->valueType = TypeBool; node->defined = 1; return;
    }
    default: {printf("[ERROR] Unknown OP %d\n", node->op); setError(node); return;}
    }
    return;
  }

  case NODE_ASSIGN_STMT: {
    struct nodeType *child1 = nthChild(1, node); struct nodeType *child2 = nthChild(2, node);
    semanticCheck(child1); semanticCheck(child2);
    if(child1->nodeType == NODE_ERROR || child2->nodeType == NODE_ERROR) return; // error handling
    switch(child1->valueType) {
    case TypeFun: printf("[ERROR] function \"%s\" is not assignable\n", child1->string); setError(node); return;
    case TypeProc: printf("[ERROR] procedure \"%s\" is not assignable\n", child1->string); setError(node); return;
    }
    // check range and type of Array and find type
    if(child1->valueType == TypeArray) {enum StdType t = checkRange(child1); if(t == TypeNull) return; child1->valueType = t;}
    if(child2->valueType == TypeArray) {enum StdType t = checkRange(child2); if(t == TypeNull) return; child2->valueType = t;}
    if(child1->valueType != child2->valueType) {
      if(node->nodeType == NODE_OP) printf("[ERROR] Type Mismatch for Operator\n");
      else printf("[ERROR] Type Mismatch for Assignment, left-op: \"%s\", but right-op: \"%s\"\n", translateType(child1->valueType), translateType(child2->valueType));
      setError(node); return;
    }
    // do assignment, check initialization
    int defined = checkInitialize(child2); child2->defined = defined;
    if(defined == 0) {printf("[ERROR] Assignment to \"%s\" is not initialized\n", child1->string); setError(node); return;}
    int index = findSymbolAll(child1->string, depth);
    if(index == -1) {printf("[ERROR] Variable \"%s\" not declared\n", child1->string); setError(node); return;}
    if(child2->defined == 2) {node->defined = 2; child1->defined = 2; node->valueType = child1->valueType; SymbolTable.entries[index].defined = 2; return;}
    child1->defined = child2->defined; SymbolTable.entries[index].defined = child2->defined;
    switch(child1->valueType) {
    case TypeInt: {node->iValue = child2->iValue;
	switch(SymbolTable.entries[index].type) {
	case TypeInt: {int *v = (int *)malloc(sizeof(int)); *v = child2->iValue; SymbolTable.entries[index].typeDsrpt = v; break;}
	/* case TypeArray: { printf("Checking assignment to Int Array is not implemented\n"); return;} // not implemented */
	}
	break;
    }
    case TypeReal: {node->rValue = child2->rValue;
	switch(SymbolTable.entries[index].type) {
	case TypeReal: { double *v = (double *)malloc(sizeof(double)); *v = child2->rValue; SymbolTable.entries[index].typeDsrpt = v; break;}
	/* case TypeArray: { printf("Checking assignment to Real Array is not implemented\n"); return;} // not implemented */
	}
	break;
    }
    case TypeArray: { // check if range is the same
      printf("[ERROR] Checking assignment of Array is not implemented\n"); break;}
    }
    node->valueType = child1->valueType; node->defined = child2->defined; return;
  }
  case NODE_PROC_STMT: {
    struct nodeType *nameNode = nthChild(1, node);
    if(nameNode->nodeType == NODE_ERROR) return;
    int index = findSymbolAt(nameNode->string, global);
    if(index == -1) { printf("[ERROR] Undefined function/procedure \"%s\"\n", nameNode->string); setError(node); return; }
    struct SymTableEntry entry = SymbolTable.entries[index];
    switch(entry.type) {
    case TypeFun: {
      struct FunctionDsrpt *funDef = entry.typeDsrpt; struct nodeType *expList = nthChild(2, node); semanticCheck(expList);
      struct nodeType *idNode = expList->child; int i = 0;
      do { // check parameters
	if(i >= funDef->paramNum) {printf("[ERROR] Too much parameters for function \"%s\", expecting only %d parameters\n", nameNode->string, funDef->paramNum); setError(node); return; }
	if(idNode->string != NULL) {int index = findSymbolAll(idNode->string, depth); idNode->defined = SymbolTable.entries[index].defined;}
	if(idNode->defined == 0) { printf("[ERROR] Parameter for function \"%s\" is not initialized\n", nameNode->string); setError(node); return; }
	enum StdType fp = funDef->paramType[i]->type; enum StdType curp = idNode->valueType;
	switch(curp) {case TypeArray: { curp = checkRange(idNode); idNode->valueType = curp; break; }}
	if(fp != curp) { printf("[ERROR] Calling function \"%s\" with mismatch type, expecting \"%s\" but \"%s\"\n", nameNode->string, translateType(fp), translateType(curp)); setError(node); return; }
	idNode = idNode->rsibling; i++;
      } while(idNode != expList->child);
      if(i<funDef->paramNum) { printf("[ERROR] Too few parameters for function \"%s\", expecting %d but only found %d\n", nameNode->string, funDef->paramNum, i); setError(node); return;}
      node->valueType = funDef->retType->type; node->defined = 2; // set return value
      break;
    }
    case TypeProc: {
      struct ProcedureDsrpt *procDef = entry.typeDsrpt; struct nodeType *expList = nthChild(2, node); semanticCheck(expList);
      struct nodeType *idNode = expList->child; int i = 0;
      do { // check parameters
	if(idNode->nodeType == NODE_ERROR) return;
	if(i >= procDef->paramNum) { printf("[ERROR] Too much parameters for procedure \"%s\", expecting only %d parameters\n", nameNode->string, procDef->paramNum); setError(node); return; }
	if(idNode->string != NULL) {int index = findSymbolAll(idNode->string, depth); idNode->defined = SymbolTable.entries[index].defined;}
	if(idNode->defined == 0) { printf("[ERROR] Parameter for procedure \"%s\" is not initialized\n", nameNode->string); setError(node); return; }
	enum StdType pp = procDef->paramType[i]->type; enum StdType curp = idNode->valueType;
	if(pp != curp) { printf("[ERROR] Calling procedure \"%s\" with mismatch type, expecting \"%s\" but \"%s\"\n", nameNode->string, translateType(pp), translateType(curp)); setError(node); return; }
	idNode = idNode->rsibling; i++;
      } while(idNode != expList->child);
      if(i<procDef->paramNum) {printf("[ERROR] Too few parameters for procedure \"%s\", expecting %d but only found %d\n", nameNode->string, procDef->paramNum, i); setError(node); return;}
      node->valueType = TypeNull; // no return value
      break;
    }
    default: { printf("[ERROR] Variable \"%s\" is not callable\n", nameNode->string); setError(node); return; }
    }
    break;
  }
  case NODE_INT: { node->valueType = TypeInt; node->defined = 1; return; }
  case NODE_REAL: { node->valueType = TypeReal; node->defined = 1; return; }
  case NODE_CHAR: { node->valueType = TypeChar; node->defined = 1; return; }
  case NODE_LABEL: { node->valueType = TypeLabel; return; }
  case NODE_IF: { // check if condition
    struct nodeType *ifCond = nthChild(1, node); semanticCheck(ifCond);
    if(ifCond->valueType != TypeBool) {printf("[ERROR] Condition in IF statement must be \"Logical\" comparison\n");setError(node); return;}
    semanticCheck(nthChild(2, node)); semanticCheck(nthChild(3, node));
    return; 
  }
  case NODE_WHILE: { // check while condition
    struct nodeType *whileCond = nthChild(1, node); semanticCheck(whileCond);
    if(whileCond->valueType != TypeBool) {printf("[ERROR] Condition in WHILE statement must be \"Logical\" comparison\n");setError(node); return;}
    semanticCheck(nthChild(2, node)); return; 
  }
  case NODE_FOR: {
    struct nodeType *id = nthChild(1, node); struct nodeType *lexp = nthChild(2, node);
    struct nodeType *rexp = nthChild(3, node); struct nodeType *stmt = nthChild(4, node);
    semanticCheck(lexp); semanticCheck(rexp); semanticCheck(stmt);
    // check if ID assignable
    int index = findSymbol(id->string);
    if(index == -1) {printf("[ERROR] Undefined FOR variable \"%s\"\n", id->string); setError(node); return;}
    enum StdType type = SymbolTable.entries[index].type;
    if(type != lexp->valueType) {printf("[ERROR] Mismatch Variable with lower expression in FOR condition\n"); setError(node); return;}
    if(type != rexp->valueType) {printf("[ERROR] Mismatch Variable with upper expression in FOR condition\n"); setError(node); return;}
    return; 
  }
  case NODE_REPEAT: {
    struct nodeType *repeatCond = nthChild(2, node); semanticCheck(repeatCond);
    if(repeatCond->valueType != TypeBool) {printf("[ERROR] Condition in REPEAT statement must be \"Logical\" comparison\n");setError(node); return;} semanticCheck(nthChild(1, node)); return; 
  }
  // open new scope, use var of new scope first
  case NODE_WITH: { openScope(); semanticCheck(nthChild(1, node)); semanticCheck(nthChild(2, node)); closeScope(); return; }
  case NODE_GOTO: { // check goto label
    struct nodeType *label = nthChild(1, node);
    semanticCheck(label);
    if(label->valueType != TypeInt) {printf("[ERROR] GOTO label must be \"Integer\"\n");setError(node); return;}
    int index = findSymbol(intToStr(label->iValue));
    if(SymbolTable.entries[index].type != TypeLabel) {printf("[ERROR] Undefined GOTO label %d\n", label->iValue);setError(node); return;}
    return; 
  }
  }

  /* default action for other nodes */
  struct nodeType *child = node->child; if(child != 0) {do { semanticCheck(child); child = child->rsibling; } while(child != node->child);}
}
