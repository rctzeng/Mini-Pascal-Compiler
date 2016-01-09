#ifndef __SYMTAB_H___
#define __SYMTAB_H___

#define SYM_MAX 100
#define DEP_MAX 10
#define PAR_MAX 10

enum StdType {
  TypeInt, TypeReal, TypeChar, TypeBool, TypeArray, TypeFun, TypeProc, TypeNull, TypeLabel, TypeType
};

struct SymTableEntry {
  char name[100];
  enum StdType type;
  int depth; // debug
  int valid;
  int defined;
  void *typeDsrpt;
  int next; // connect all same scope entries
  int index; // in Jasmine Field List
};

struct DsrptType {
  enum StdType type;
  void *typeDsrpt;
};

struct ArrayRange {
  char name[100];
  int value;
  int defined;
};

struct FunctionDsrpt {
  struct DsrptType *retType;
  struct DsrptType *paramType[PAR_MAX];
  int paramNum;
  int scope;
};

struct ProcedureDsrpt {
  struct DsrptType *paramType[PAR_MAX];
  int paramNum;
  int scope;
};

struct ArrayDsrpt {
  struct ArrayRange *lowerBound;
  struct ArrayRange *upperBound;
  int dimension;
  enum StdType type;
  void *typeDsrpt;
};

struct SymTable {
  struct SymTableEntry entries[SYM_MAX];
  int scopes[DEP_MAX];
  char **name;
  int error;
  int scope_num;
};

extern struct SymTable SymbolTable;

#include "node.h"
void semanticCheck(struct nodeType* node);
struct nodeType* nthChild(int n, struct nodeType *node);
int findSymbolAt(char *s, int dep);
int findSymbolAll(char *s, int dep);
char* displayType(void *type);
void printSymTable(int dep);
#endif

