#include "symtab.h"
#include "gencode.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int depth;
int label_num;

void print(char *instr) { fprintf(stdout, "\t%s\n", instr); }
void genHeader(char *fname) { fprintf(stdout, ".class public %s\n.super java/lang/Object\n", fname); }
int getArraySize(struct ArrayDsrpt *dsrpt) {
  int lower, upper;
  if(dsrpt->lowerBound->defined == 0) {
    fprintf(stderr, "Runtime retrieve array bound is not implemented\n"); exit(1);
  } else { lower = dsrpt->lowerBound->value; }
  if(dsrpt->upperBound->defined == 0) {
    fprintf(stderr, "Runtime retrieve array bound is not implemented\n"); exit(1);
  } else { upper = dsrpt->upperBound->value; }
  switch(dsrpt->type) {
  case TypeInt: { return (upper-lower+1); }
  case TypeReal: { return (upper-lower+1)*2; }
  case TypeArray: { return (upper-lower+1)*getArraySize(dsrpt->typeDsrpt); }
  default: { fprintf(stderr, "[ERROR] Array Type of %d is undefined\n", dsrpt->type); exit(1); }
  }
}
int genVar(int depth) {
  int num = 0;
  int index = SymbolTable.scopes[depth];
  while(index != -1) { // assign Jasmine local index to global variables
    switch(SymbolTable.entries[index].type) {
    case TypeInt: { SymbolTable.entries[index].index = num++; break;}
    case TypeReal: { SymbolTable.entries[index].index = num; num += 2; break; }
    case TypeArray: { SymbolTable.entries[index].index = num; int size = getArraySize(SymbolTable.entries[index].typeDsrpt); num += size; break; }
    }
    index = SymbolTable.entries[index].next;
  }
  return num+2;
}
char* genArraySig(struct ArrayDsrpt *dsrpt) {
  switch(dsrpt->type) {
  case TypeInt: { return "[I"; }
  case TypeReal: { return "[D"; }
  case TypeChar: { return "[Ljava/lang/String;"; }
  case TypeArray: { char *sig = (char *)malloc(sizeof(char)*MAX_SIG); strcpy(sig, "["); strcat(sig, genArraySig(dsrpt->typeDsrpt)); return sig; }
  default: { fprintf(stderr, "[ERROR] Array Type of %d is undefined\n", dsrpt->type); exit(1); }
  }
}
void genGlobalVar() {
  int index = SymbolTable.scopes[0];
  while(index != -1) { // assign Jasmine fieled index to global variables
    switch(SymbolTable.entries[index].type) {
    case TypeInt: { SymbolTable.entries[index].index = -1; fprintf(stdout, ".field public static %s I\n", SymbolTable.entries[index].name); break;}
    case TypeReal: { SymbolTable.entries[index].index = -1; fprintf(stdout, ".field public static %s D\n", SymbolTable.entries[index].name); break; }
    case TypeChar: { SymbolTable.entries[index].index = -1; fprintf(stdout, ".field public static %s Ljava/lang/String;\n", SymbolTable.entries[index].name); break; }
    case TypeArray: { SymbolTable.entries[index].index = -1; fprintf(stdout, ".field public static %s %s\n", SymbolTable.entries[index].name, genArraySig(SymbolTable.entries[index].typeDsrpt)); break; }
    }
    index = SymbolTable.entries[index].next;
  }
}
char* genField(char *name) {
  char *rst = (char *)malloc(sizeof(char)*MAX_SIG);
  strcpy(rst, name);
  int index = findSymbolAt(name, 0);
  switch(SymbolTable.entries[index].type) {
  case TypeInt: { strcat(rst, " I"); return rst; }
  case TypeReal: { strcat(rst, " D"); return rst; }
  case TypeChar: { strcat(rst, " Ljava/lang/String;"); return rst; }
  default: { strcat(rst, " "); strcat(rst, genArraySig(SymbolTable.entries[index].typeDsrpt)); return rst; }
  }
}
void genArrayRange(struct ArrayDsrpt *dsrpt) {
  int lower, upper;
  lower = dsrpt->lowerBound->value; upper = dsrpt->upperBound->value;
  fprintf(stdout, "\tbipush %d\n", (upper-lower+1));
  if(dsrpt->type == TypeArray)
    genArrayRange(dsrpt->typeDsrpt);
}
void allocArray(int index, char *fname) {
  struct ArrayDsrpt *dsrpt = SymbolTable.entries[index].typeDsrpt;
  if(dsrpt->dimension > 1) {
    genArrayRange(dsrpt);
    fprintf(stdout, "\tmultianewarray %s %d\n", genArraySig(dsrpt), dsrpt->dimension);
  } else {
    genArrayRange(dsrpt);
    switch(dsrpt->type) {
    case TypeInt: fprintf(stdout, "\tnewarray int\n"); break;
    case TypeReal: fprintf(stdout, "\tnewarray double\n"); break;
    case TypeChar: fprintf(stdout, "\tanewarray java/lang/String\n"); break;
    default: fprintf(stderr, "Array Type of %d not implemented\n", dsrpt->type); exit(1);
    }
  }
  if(SymbolTable.entries[index].index == -1) fprintf(stdout, "\tputstatic %s.%s\n", fname, genField(SymbolTable.entries[index].name));
  else fprintf(stdout, "\tastore %d\n", SymbolTable.entries[index].index);
}
void genAllocArray(int depth, char *fname) {
  int index = SymbolTable.scopes[depth];
  while(index != -1) {
    if(SymbolTable.entries[index].type == TypeArray) allocArray(index, fname);
    index = SymbolTable.entries[index].next;
  }
}
void genBodyHead(char *fname, int varnum) { fprintf(stdout, ".method public static main([Ljava/lang/String;)V\n\t.limit stack %d\n\t.limit locals %d\n", varnum*2, varnum); }
void genBodyEnd() { fprintf(stdout, "\treturn\n.end method");}
char* sigType(struct DsrptType *dtype) {
  enum StdType type = dtype->type;
  switch(type) { case TypeInt: return "I"; case TypeReal: return "D"; case TypeChar: return "Ljava/lang/String;"; case TypeArray: return genArraySig(dtype->typeDsrpt);
  default: { fprintf(stderr, "[ERROR] %s not implemented yet\n", displayType(dtype)); exit(1); }
  }
}
char* genSignature(char *funproc_name) {
  char *signature = (char *)malloc(sizeof(char)*MAX_SIG);
  strcpy(signature, funproc_name); strcat(signature, "(");
  int index = findSymbolAt(funproc_name, 0);
  struct SymTableEntry entry = SymbolTable.entries[index];
  switch(entry.type) {
  case TypeFun: {
    struct FunctionDsrpt *funDef = entry.typeDsrpt;
    int i; for(i=0;i<funDef->paramNum;i++) strcat(signature, sigType(funDef->paramType[i]));
    strcat(signature, ")"); strcat(signature, sigType(funDef->retType));
    return signature;
  }
  case TypeProc: {
    struct ProcedureDsrpt *procDef = entry.typeDsrpt;
    int i; for(i=0;i<procDef->paramNum;i++) strcat(signature, sigType(procDef->paramType[i]));
    strcat(signature, ")V");
    return signature;
  }
  default: { fprintf(stderr, "[ERROR] %s must be a function or procedure\n", funproc_name); exit(1); }
  }
}
void genMethodDeclar() {
  fprintf(stdout, ".method public <init>()V\n\taload_0\n\tinvokenonvirtual java/lang/Object/<init>()V\n\treturn\n.end method\n");
  /* printInt, printReal, printString */
  fprintf(stdout, ".method public static printInt(I)V\n\t.limit locals 1\n\t.limit stack 2\n\tgetstatic java/lang/System/out Ljava/io/PrintStream;\n\tiload_0\n\tinvokevirtual java/io/PrintStream/println(I)V\n\treturn\n.end method\n.method public static printReal(D)V\n\t.limit locals 2\n\t.limit stack 3\n\tgetstatic java/lang/System/out Ljava/io/PrintStream;\n\tdload_0\n\tinvokevirtual java/io/PrintStream/println(D)V\n\treturn\n.end method\n.method public static printString(Ljava/lang/String;)V\n\t.limit locals 1\n\t.limit stack 2\n\tgetstatic java/lang/System/out Ljava/io/PrintStream;\n\taload_0\n\tinvokevirtual java/io/PrintStream/println(Ljava/lang/String;)V\n\treturn\n.end method\n");
}
int checkReference(struct nodeType *node) {
  int index = findSymbolAll(node->string, depth); if(SymbolTable.entries[index].type == TypeArray) return 1; return 0;
}
int genSimpleLHS(struct nodeType *node) {
  int index = findSymbolAll(node->string, depth); if(index == -1) fprintf(stderr, "[ERROR] %s not found at scope %d\n", node->string, depth); return SymbolTable.entries[index].index;
}
void genArrayLHS(struct nodeType *node, char *fname) {
  struct nodeType *nameNode = nthChild(1, node); struct nodeType *dList = nthChild(2, node);
  int index = genSimpleLHS(nameNode);
  if(index == -1) {
    int idx = findSymbolAt(nameNode->string, 0);
    fprintf(stdout, "\tgetstatic %s.%s\n", fname, genField(nameNode->string));
  } else fprintf(stdout, "\taload %d\n", index);
  struct nodeType *idNode = dList->child;
  struct nodeType **nodes = malloc(sizeof(struct nodeType *)*MAX_DIM);
  int paraN; int i = 0; do { nodes[i] = idNode; i++; idNode = idNode->rsibling; } while(idNode != dList->child); paraN = i;
  index = findSymbolAll(nameNode->string, depth);
  struct ArrayDsrpt *arrayD = SymbolTable.entries[index].typeDsrpt;
  for(i=paraN-1; i>=0; i--) {
    genCode(nodes[i], fname);
    fprintf(stdout, "\tldc %d\n\tisub\n", arrayD->lowerBound->value);
    if(i>0) fprintf(stdout, "\taaload\n");
    arrayD = arrayD->typeDsrpt;
  }
}
void genArrayRef(struct nodeType *node, char *fname) {
  struct nodeType *nameNode = nthChild(1, node); struct nodeType *dList = nthChild(2, node);
  int index = genSimpleLHS(nameNode);
  if(index == -1) fprintf(stdout, "\tgetstatic %s.%s\n", fname, genField(nameNode->string));
  else fprintf(stdout, "\taload %d\n", index);
  index = findSymbolAll(nameNode->string, depth);
  struct nodeType *idNode = dList->child; struct nodeType **nodes = malloc(sizeof(struct nodeType *)*MAX_DIM);
  int paraN; int i = 0; do { nodes[i] = idNode; i++; idNode = idNode->rsibling; } while(idNode != dList->child); paraN = i;
  struct ArrayDsrpt *arrayD = SymbolTable.entries[index].typeDsrpt;
  for(i=paraN-1; i>=0; i--) {
    genCode(nodes[i], fname);
    fprintf(stdout, "\tldc %d\n\tisub\n", arrayD->lowerBound->value);
    if(i>0) fprintf(stdout, "\taaload\n");
    else {
      switch(arrayD->type) {
      case TypeInt: print("iaload"); return;
      case TypeReal: print("daload"); return;
      case TypeChar: print("aaload"); return;
      default: fprintf(stderr, "Array Type of %d not implemented\n", arrayD->type); exit(1);
      }
    }
    arrayD = arrayD->typeDsrpt;
  }
}
char* genLabel() {
  char *label = (char *)malloc(sizeof(char)*MAX_LABEL);
  char *num = malloc(sizeof(char)*(MAX_LABEL-1)); sprintf(num, "%d", label_num);
  strcpy(label, "L"); strcat(label, num); label_num++;
  return label;
}
void negateOperator(struct nodeType *node) {
  switch(node->op) {
  case OP_GT: node->op = OP_LE; break; case OP_LT: node->op = OP_GE; break; case OP_GE: node->op = OP_LT; break; case OP_LE: node->op = OP_GT; break; case OP_EQ: node->op = OP_NE; break; case OP_NE: node->op = OP_EQ; break; case OP_NOT: node->op = OP_NO; break; case OP_NO: node->op = OP_NOT; break;
  }
}
void genPredicate(struct nodeType *node, char *nlabel, char *fname) {
  switch(node->op) {
  case OP_GT:
  case OP_LT:
  case OP_GE:
  case OP_LE: {
    struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node); genCode(Lexp, fname); genCode(Rexp, fname);
    switch(Lexp->valueType) {
    case TypeInt: {
      switch(node->op) {
      case OP_GT: { fprintf(stdout, "\tif_icmple %s\n", nlabel); return; }
      case OP_LT: { fprintf(stdout, "\tif_icmpge %s\n", nlabel); return; }
      case OP_GE: { fprintf(stdout, "\tif_icmplt %s\n", nlabel); return; }
      case OP_LE: { fprintf(stdout, "\tif_icmpgt %s\n", nlabel); return; }
      }
      break; }
    case TypeReal: {
      switch(node->op) {
      case OP_GT: { fprintf(stdout, "\tdcmpl\n\tifle %s\n", nlabel); return; }
      case OP_LT: { fprintf(stdout, "\tdcmpg\n\tifge %s\n", nlabel); return; }
      case OP_GE: { fprintf(stdout, "\tdcmpl\n\tiflt %s\n", nlabel); return; }
      case OP_LE: { fprintf(stdout, "\tdcmpg\n\tifgt %s\n", nlabel); return; }
      }
      break; }
    default: { fprintf(stderr, "[ERROR] comparision for type = %d is undefined\n", Lexp->valueType); return; }
    } return; }
  case OP_EQ:
  case OP_NE: {
    struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node); genCode(Lexp, fname); genCode(Rexp, fname);
    switch(Lexp->valueType) {
    case TypeInt: {
      switch(node->op) {
      case OP_EQ: { fprintf(stdout, "\tif_icmpne %s\n", nlabel); return; }
      case OP_NE: { fprintf(stdout, "\tif_icmpeq %s\n", nlabel); return; }
      } break;
    }
    case TypeReal: {
      switch(node->op) {
      case OP_EQ: { fprintf(stdout, "\tdcmpl\n\tifne %s\n", nlabel); return; }
      case OP_NE: { fprintf(stdout, "\tdcmpl\n\tifeq %s\n", nlabel); return; }
      } break;
    }
    default: { fprintf(stderr, "[ERROR] comparision for type = %d is undefined\n", Lexp->valueType); return; }
    }
    return; }
  case OP_NOT: { struct nodeType *exp = nthChild(1, node); negateOperator(exp); genPredicate(exp, nlabel, fname); return; }
  case OP_NO: { struct nodeType *exp = nthChild(1, node); genPredicate(exp, nlabel, fname); return; }
  }
}
int getProgScope(char *funproc_name) {
  int index = findSymbolAt(funproc_name, 0);
  switch(SymbolTable.entries[index].type) {
  case TypeFun: { struct FunctionDsrpt *fun = (struct FunctionDsrpt *)SymbolTable.entries[index].typeDsrpt; return fun->scope; }
  case TypeProc: { struct ProcedureDsrpt *proc = (struct ProcedureDsrpt *)SymbolTable.entries[index].typeDsrpt; return proc->scope; }
  }
}
void genPostLude(char *funproc_name) {
  int index = findSymbolAt(funproc_name, 0);
  switch(SymbolTable.entries[index].type) {
  case TypeFun: {
    struct FunctionDsrpt *fun = (struct FunctionDsrpt *)SymbolTable.entries[index].typeDsrpt;
    switch(fun->retType->type) {
    case TypeInt: { int index = findSymbolAt(funproc_name, depth); fprintf(stdout, "\tiload %d\n\tireturn\n", SymbolTable.entries[index].index); return; }
    case TypeReal: { int index = findSymbolAt(funproc_name, depth); fprintf(stdout, "\tdload %d\n\tdreturn\n", SymbolTable.entries[index].index); return; }
    case TypeChar: { int index = findSymbolAt(funproc_name, depth); fprintf(stdout, "\taload %d\n\tareturn\n", SymbolTable.entries[index].index); return; }
    default: fprintf(stderr, "Return type = %d is not implemented\n", fun->retType->type); exit(1);
    }
  }
  case TypeProc: fprintf(stdout, "\treturn\n"); return;
  }
}

void genCode(struct nodeType *node, char *fname) {
  switch(node->nodeType) {
  case NODE_PROGRAM: { depth = label_num = 0; 
      genHeader(fname); int varnum = 10; genGlobalVar(); genMethodDeclar(); genCode(nthChild(5, nthChild(3, node)), fname);
      genBodyHead(fname, varnum); genAllocArray(0, fname); genCode(nthChild(4, node), fname); genBodyEnd();
      return; 
  }
  case NODE_SUBPROG: {
    struct nodeType *head = nthChild(1, node); struct nodeType *stmt = nthChild(3, node);
    struct nodeType *nameNode = nthChild(1, head);
    fprintf(stdout, ".method public static %s\n", genSignature(nameNode->string));
    depth = getProgScope(nameNode->string); int varnum = genVar(depth); fprintf(stdout, "\t.limit stack %d\n\t.limit locals %d\n", varnum*2, varnum);
    genAllocArray(depth, fname); genCode(stmt, fname); genPostLude(nameNode->string); depth = 0;
    fprintf(stdout, ".end method\n");
    return;
  }
  case NODE_VAR_OR_PROC: {
    int index = findSymbolAll(node->string, depth);
    struct SymTableEntry entry = SymbolTable.entries[index];
    switch(entry.type) {
    case TypeInt: { if(entry.index == -1) fprintf(stdout, "\tgetstatic %s.%s\n", fname, genField(node->string)); else fprintf(stdout, "\tiload %d\n", entry.index); return; }
    case TypeReal: { if(entry.index == -1) fprintf(stdout, "\tgetstatic %s.%s\n", fname, genField(node->string)); else fprintf(stdout, "\tdload %d\n", entry.index); return; }
    case TypeChar: case TypeArray: { if(entry.index == -1) fprintf(stdout, "\tgetstatic %s.%s\n", fname, genField(node->string)); else fprintf(stdout, "\taload %d\n", entry.index); return; }
    case TypeFun:
    case TypeProc: { fprintf(stdout, "\tinvokestatic %s.%s\n", fname, genSignature(node->string)); return; }
    default: { fprintf(stderr, "Determing Var_or_Proc for Type %d unimplemented\n", entry.type); }
    }
    return; 
  }
  case NODE_SYM_REF: {
    struct nodeType *nameNode = nthChild(1, node);
    if(checkReference(nameNode) == 1) genArrayRef(node, fname);      
    else {
      int index = findSymbolAll(node->string, depth);
      struct SymTableEntry entry = SymbolTable.entries[index];
      switch(entry.type) {
      case TypeInt: { fprintf(stdout, "\tiload %d\n", entry.index); return; }
      case TypeReal: { fprintf(stdout, "\tdload %d\n", entry.index); return; }
      case TypeChar: { fprintf(stdout, "\taload %d\n", entry.index); return; }
      default: { fprintf(stderr, "Symbol reference for Type %d unimplemented\n", entry.type); }
      }
    }
    return; 
  }
  case NODE_OP: { 
    switch(node->op){
    case OP_ADD:
    case OP_SUB:
    case OP_MUL:
    case OP_DIV: {
      struct nodeType *Lexp = nthChild(1, node); struct nodeType *Rexp = nthChild(2, node); genCode(Lexp, fname); genCode(Rexp, fname);
      switch(Lexp->valueType) {
      case TypeInt:{ switch(node->op) { case OP_ADD: print("iadd"); break; case OP_SUB: print("isub"); break; case OP_MUL: print("imul"); break; case OP_DIV: print("idiv"); break; } break;}
      case TypeReal:{ switch(node->op) { case OP_ADD: print("dadd"); break; case OP_SUB: print("dsub"); break; case OP_MUL: print("dmul"); break; case OP_DIV: print("ddiv"); break; } break;}
      default: { fprintf(stderr, "Operation for Type %d unimplemented\n", Lexp->valueType); }
      } return; }
    case OP_GT: case OP_LT: case OP_GE: case OP_LE: case OP_EQ: case OP_NE: case OP_NOT: {fprintf(stderr, "[ERROR] Compare Statements Not in Predicate\n"); break; }
    default: { return; }
    }
    break;
  }
  case NODE_ASSIGN_STMT: {
    struct nodeType *LHS = nthChild(1, node); struct nodeType *RHS = nthChild(2, node);
    switch(LHS->valueType) {
    case TypeInt: {
      int check = checkReference(LHS);
      switch(check) {
      case 0: { 
	int index = genSimpleLHS(LHS); genCode(RHS, fname); 
	if(index == -1) fprintf(stdout, "\tputstatic %s.%s\n", fname, genField(LHS->string)); 
	else fprintf(stdout, "\tistore %d\n", index); return;}
      case 1: { 
	genArrayLHS(LHS, fname); genCode(RHS, fname); 
	fprintf(stdout, "\tiastore\n"); return;}
      }}
    case TypeReal: {
      int check = checkReference(LHS);
      switch(check) {
      case 0: {
      int index = genSimpleLHS(LHS); genCode(RHS, fname);
      if(index == -1) fprintf(stdout, "\tputstatic %s.%s\n", fname, genField(LHS->string)); else fprintf(stdout, "\tdstore %d\n", index); return; }
      case 1: {
	genArrayLHS(LHS, fname); genCode(RHS, fname); fprintf(stdout, "\tdastore\n"); return; }
      }}
    case TypeChar: {
      int check = checkReference(LHS);
      switch(check) {
      case 0: { int index = genSimpleLHS(LHS); genCode(RHS, fname);
      if(index == -1) fprintf(stdout, "\tputstatic %s.%s\n", fname, genField(LHS->string)); else fprintf(stdout, "\tastore %d\n", index); return; }
      case 1: { genArrayLHS(LHS, fname); genCode(RHS, fname); fprintf(stdout, "\taastore\n"); return; }
      }}
    default: { fprintf(stderr, "Assignment for Type %d unimplemented\n", LHS->valueType); }
    }
    return; 
  }
  case NODE_PROC_STMT: {
    struct nodeType *nameNode = nthChild(1, node); struct nodeType *expList = nthChild(2, node);
    genCode(expList, fname);
    fprintf(stdout, "\tinvokestatic %s.%s\n", fname, genSignature(nameNode->string));
    return;
  }
  case NODE_IF: {
    struct nodeType *predicate = nthChild(1, node); struct nodeType *ifstmt = nthChild(2, node); struct nodeType *elsestmt = nthChild(3, node);
    char *nlabel = genLabel(); char *endlabel = genLabel();
    genPredicate(predicate, nlabel, fname);
    genCode(ifstmt, fname); fprintf(stdout, "\tgoto %s\n", endlabel);
    fprintf(stdout, "%s:\n", nlabel);
    genCode(elsestmt, fname);
    fprintf(stdout, "%s:\n", endlabel);    
    return; 
  }
  case NODE_WHILE: {
    struct nodeType *predicate = nthChild(1, node); struct nodeType *stmt = nthChild(2, node);
    char *looplabel = genLabel(); char *endlabel = genLabel();
    fprintf(stdout, "%s:\n", looplabel);
    genPredicate(predicate, endlabel, fname);
    genCode(stmt, fname);
    fprintf(stdout, "\tgoto %s\n", looplabel);
    fprintf(stdout, "%s:\n", endlabel);
    return; 
  }
  case NODE_FOR: { fprintf(stderr, "FOR unimplemented\n"); break; }
  case NODE_REPEAT: { fprintf(stderr, "REPEAT unimplemented\n"); break; }
  case NODE_WITH: { fprintf(stderr, "WITH unimplemented\n"); break; }
  case NODE_GOTO: { fprintf(stderr, "GOTO unimplemented\n"); break; }
  case NODE_INT: { fprintf(stdout, "\tldc %d\n", node->iValue); return; }
  case NODE_REAL: { fprintf(stdout, "\tldc2_w %lfd\n", node->rValue); return; }
  case NODE_CHAR: { fprintf(stdout, "\tldc %s\n", node->string); return; }
  case NODE_LABEL: { node->valueType = TypeLabel; return; }
  }
  /* default action for other nodes */
  struct nodeType *child = node->child; if(child != 0) { do { genCode(child, fname); child = child->rsibling; } while(child != node->child); }
}
