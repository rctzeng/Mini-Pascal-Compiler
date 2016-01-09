%{
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include "node.h"
  #include "symtab.h"
  #include "gencode.h"

  struct nodeType* newOpNode(int op);
  extern struct nodeType* ASTRoot;

  void yyerror(const char *str) {
    extern char *yytext;
    extern int line_no, chr_no;
    fprintf(stdout, "[ERROR] %s at line %d:%d symbol '%s'\n", str, line_no, chr_no, yytext);
    exit(0);
  }
%}

%union {
  struct nodeType *node;
}

%token <node> ARRAY PCONST DO DOWNTO TO REPEAT UNTIL GOTO WITH ELSE END FOR FUNCTION IDENTIFIER IF LABEL NOT OF PROCEDURE PROGRAM PBEGIN THEN VAR WHILE ASSIGNMENT COLON COMMA DOT DOTDOT EQUAL GE GT LBRACE LE LPAREN LT MINUS NOTEQUAL PLUS RBRACE RPAREN SEMICOLON SLASH STAR TYPE notEQUAL INTEGER STRING DIGSEQ REAL NUMBER CHARACTER_STRING ID NUM END_OF_FILE

%type <node> goal prog identifier_list block compound_statement label_list statement_list statement const_list constant type_list type var_list label_declaration const_declaration type_declaration var_declaration subprogram_declarations standard_type simple_type idconst subprogram_declaration subprogram_head arguments parameter_list optional_var optional_statements else_statement expression procedure_statement optional_to variable tail term simple_expression factor relop const_asgn type_asgn var_asgn par_asgn expression_list negative

%%
goal: prog {
  fprintf(stdout, "goal: prog\n");
  ASTRoot = $1; YYACCEPT;
};
prog : PROGRAM IDENTIFIER LPAREN identifier_list RPAREN SEMICOLON block compound_statement DOT {
  $$ = newNode(NODE_PROGRAM);
  addChild($$, $2); addChild($$, $4); 
  addChild($$, $7);
  addChild($$, $8);
  deleteNode($1); deleteNode($3); deleteNode($5); deleteNode($6); deleteNode($9);
}| error END_OF_FILE {
  fprintf(stdout, "[RECOVER ERROR]\n");
  yyerrok;
}; 

identifier_list : IDENTIFIER {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| identifier_list COMMA IDENTIFIER {
  $$ = $1;
  addChild($$, $3);
  deleteNode($2);
};


label_list : DIGSEQ {
  $$ = newNode(NODE_LIST);
  $1->nodeType = NODE_LABEL;
  addChild($$, $1);
}| label_list COMMA DIGSEQ { 
  $$ = $1;
  $3->nodeType = NODE_LABEL;
  addChild($$, $3);
  deleteNode($2);
};

const_asgn : IDENTIFIER EQUAL constant SEMICOLON {
  $$ = newNode(NODE_CONST);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2); deleteNode($4);
};

const_list : const_asgn {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| const_list const_asgn {
  $$ = $1;
  addChild($$, $2);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong constant assignment\n");
  yyerrok;
};

negative: MINUS DIGSEQ {
  $$ = newNode(NODE_INT); $$->iValue = -($2->iValue);
  deleteNode($1);
} | MINUS NUMBER {
  $$ = newNode(NODE_REAL); $$->rValue = -($2->rValue);
  deleteNode($1);
};

standard_type: INTEGER {
  $$ = newNode(NODE_TYPE_INT);
} | REAL {
  $$ = newNode(NODE_TYPE_REAL);
}| STRING {
  $$ = newNode(NODE_TYPE_CHAR);
}| ID {
  $$ = newNode(NODE_TYPE_ID);
}| NUM{
  $$ = newNode(NODE_TYPE_NUM);
};
simple_type : idconst DOTDOT idconst {
  $$ = newNode(NODE_RANGE);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2);
}| IDENTIFIER {
  $$ = $1;
};
type : standard_type {
  $$ = $1;
} | ARRAY LBRACE simple_type RBRACE OF type {
  $$ = newNode(NODE_TYPE_ARRAY);
  addChild($$, $3); addChild($$, $6);
  deleteNode($1); deleteNode($2); deleteNode($4); deleteNode($5);
 } | IDENTIFIER {
   $$ = $1;
};

type_asgn : IDENTIFIER EQUAL type SEMICOLON {
  $$ = newNode(NODE_TYPE_DECL);
  addChild($$, $1);
  addChild($$, $3);
  deleteNode($2); deleteNode($4);
};
type_list : type_asgn {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| type_list type_asgn {
  $$ = $1;
  addChild($$, $2);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong type assignment\n");
  yyerrok;
};

var_asgn : identifier_list COLON type {
  $$ = newNode(NODE_VAR_DECL);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2);
};

var_list : var_asgn {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| var_list SEMICOLON var_asgn {
  $$ = $1;
  addChild($$, $3);
  deleteNode($2);
};

block : label_declaration const_declaration type_declaration var_declaration subprogram_declarations {
  $$ = newNode(NODE_BLOCK);
  addChild($$, $1); addChild($$, $2); addChild($$, $3); addChild($$, $4); addChild($$, $5);
};

label_declaration : LABEL label_list SEMICOLON {
  $$ = newNode(NODE_LABEL_DECL);
  addChild($$, $2);
  deleteNode($1); deleteNode($3);
}| {
  $$ = newNode(NODE_EMPTY);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong label declaration\n");
  yyerrok;
};
const_declaration : PCONST const_list {
  $$ = newNode(NODE_CONST);
  addChild($$, $2);
  deleteNode($1);
}| {
  $$ = newNode(NODE_EMPTY);
};
type_declaration : TYPE type_list {
  $$ = $2;
  deleteNode($1);
}| {
  $$ = newNode(NODE_EMPTY);
};
var_declaration : var_declaration VAR var_list SEMICOLON {
  $$ = $1;
  addChild($$, $3);
  deleteNode($2); deleteNode($4);
}| {
  $$ = newNode(NODE_LIST);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong variable declaration\n");
  yyerrok;
};

idconst : IDENTIFIER {
  $$ = $1;
  $$->nodeType = NODE_TYPE_ID;
}| constant {
  $$ = $1;
};
subprogram_declarations : subprogram_declarations subprogram_declaration SEMICOLON {
  $$ = $1;
  addChild($$, $2);
  deleteNode($3);
}| {
  $$ = newNode(NODE_LIST);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong subprogram declaration\n");
  yyerrok;
};
subprogram_declaration : subprogram_head var_declaration compound_statement {
  $$ = newNode(NODE_SUBPROG);
  addChild($$, $1); addChild($$, $2); addChild($$, $3);
};
subprogram_head : FUNCTION IDENTIFIER arguments COLON standard_type SEMICOLON {
  $$ = newNode(NODE_FUNCTION);
  addChild($$, $2); addChild($$, $3); addChild($$, $5);
  deleteNode($1); deleteNode($4); deleteNode($6);
}| PROCEDURE IDENTIFIER arguments SEMICOLON {
  $$ = newNode(NODE_PROCEDURE);
  addChild($$, $2); addChild($$, $3); deleteNode($4);
}| error SEMICOLON {
  fprintf(stdout, "[RECOVER ERROR] wrong function declaration\n");
  yyerrok;
};
arguments : LPAREN parameter_list RPAREN {
  $$ = $2; deleteNode($1); deleteNode($3);
}| {
  $$ = newNode(NODE_EMPTY);
}| error RPAREN {
  fprintf(stdout, "[RECOVER ERROR] wrong parameter_list\n");
  yyerrok;
};
par_asgn: optional_var identifier_list COLON type {
  $$ = newNode(NODE_VAR_DECL);
  addChild($$, $2); addChild($$, $4);
  deleteNode($3);
};
parameter_list : par_asgn {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| par_asgn SEMICOLON parameter_list {
  $$ = $3;
  addChild($$, $1);
  deleteNode($2);
};

optional_var : VAR {
}| {
}; // skip
compound_statement : PBEGIN optional_statements END {
  $$ = $2;
  deleteNode($1); deleteNode($3);
}| error END {
  fprintf(stdout, "[RECOVER ERROR] wrong compound statement\n");
  yyerrok;
};
optional_statements : statement_list {
  $$ = $1;
};
statement_list : statement {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| statement_list SEMICOLON statement {
  $$ = $1;
  addChild($$, $3);
  deleteNode($2);
};
else_statement : ELSE statement {
  $$ = newNode(NODE_ELSE);
  addChild($$, $2); deleteNode($1);
}| {
  $$ = newNode(NODE_EMPTY);
};
statement : variable ASSIGNMENT expression {
  $$ = newNode(NODE_ASSIGN_STMT);
  addChild($$, $1);
  addChild($$, $3);
  $1->nodeType = NODE_SYM_REF;
  deleteNode($2);
}| procedure_statement {
  $$ = $1;
}| compound_statement {
  $$ = $1;
}| IF expression THEN statement else_statement {
  $$ = newNode(NODE_IF);
  addChild($$, $2); addChild($$, $4); addChild($$, $5);
  deleteNode($1); deleteNode($3);
}| WHILE expression DO statement {
  $$ = newNode(NODE_WHILE);
  addChild($$, $2); addChild($$, $4);
  deleteNode($1); deleteNode($3);
}| FOR IDENTIFIER ASSIGNMENT expression optional_to expression DO statement {
  $$ = newNode(NODE_FOR);
  addChild($$, $2); addChild($$, $4); addChild($$, $6); addChild($$, $8);
  deleteNode($1); deleteNode($3); deleteNode($5); deleteNode($7);
}| REPEAT statement_list UNTIL expression {
  $$ = newNode(NODE_REPEAT);
  addChild($$, $2); addChild($$, $4);
  deleteNode($1); deleteNode($3);
}| WITH var_list DO statement {
  $$ = newNode(NODE_WITH);
  addChild($$, $2); addChild($$, $4);
  deleteNode($1); deleteNode($3);
}| GOTO DIGSEQ {
  $$ = newNode(NODE_GOTO);
  addChild($$, $2);
  deleteNode($1);
}| {
  $$ = newNode(NODE_EMPTY);
};
optional_to : TO | DOWNTO;
variable : IDENTIFIER tail {
  $$ = newNode(NODE_VAR);
  $$->string = $1->string;
  addChild($$, $1); addChild($$, $2);
 } | IDENTIFIER {
   $$ = newNode(NODE_VAR);
   $$->string = $1->string;
   addChild($$, $1);
};
tail : LBRACE expression RBRACE tail {
  $$ = $4;
  addChild($$, $2);
  deleteNode($1); deleteNode($3);
}| LBRACE expression RBRACE {
  $$ = newNode(NODE_LIST);
  addChild($$, $2);
  deleteNode($1); deleteNode($3);
};
procedure_statement : IDENTIFIER {
  $$ = $1;
  $$->nodeType = NODE_VAR_OR_PROC;
}| IDENTIFIER LPAREN expression_list RPAREN {
  $$ = newNode(NODE_PROC_STMT);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2); deleteNode($4);
};
expression_list : expression {
  $$ = newNode(NODE_LIST);
  addChild($$, $1);
}| expression_list COMMA expression {
  $$ = $1;
  addChild($$, $3);
  deleteNode($2);
};
expression : simple_expression {
  $$ = $1;
}| simple_expression relop simple_expression {
  $$ = newOpNode($2->op);
  addChild($$, $1); addChild($$, $3);
};
simple_expression : term {
  $$ = $1;
}| simple_expression PLUS term {
  $$ = newOpNode(OP_ADD);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2);
}| simple_expression MINUS term {
  $$ = newOpNode(OP_SUB);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2);
}| negative {
  $$ = $1;
};
term : factor {
  $$ = $1;
}| term STAR factor {
  $$ = newOpNode(OP_MUL);
  addChild($$, $1);
  addChild($$, $3);
  deleteNode($2); 
} | term SLASH factor {
  $$ = newOpNode(OP_DIV);
  addChild($$, $1);
  addChild($$, $3);
  deleteNode($2);
};
constant : NUMBER {
  $$ = $1;
  $$->nodeType = NODE_REAL;
}| DIGSEQ {
  $$ = $1;
  $$->nodeType = NODE_INT;
}| CHARACTER_STRING {
  $$ = $1;
  $$->nodeType = NODE_CHAR;
};
factor : IDENTIFIER {
  $$ = $1;
  $$->nodeType = NODE_VAR_OR_PROC;
 }| IDENTIFIER tail {
  $$ = newNode(NODE_SYM_REF);
  $$->string = $1->string;
  addChild($$, $1); addChild($$, $2);
 } | IDENTIFIER LPAREN expression_list RPAREN {
  $$ = newNode(NODE_PROC_STMT);
  addChild($$, $1); addChild($$, $3);
  deleteNode($2); deleteNode($4);
}| constant {
  $$ = $1;
}| LPAREN expression RPAREN {
  $$ = $2;
  deleteNode($1); deleteNode($3);
}| NOT factor {
  $$ = newNode(NODE_OP);
  $$->op = OP_NOT;
  addChild($$, $2);
  deleteNode($1);
 }| error RPAREN {
  fprintf(stdout, "[RECOVER ERROR] wrong factor\n");
  yyerrok;
};
relop : LT { $$->op = OP_LT; }
| GT { $$->op = OP_GT; }
| EQUAL { $$->op = OP_EQ; }
| LE { $$->op = OP_LE; }
| GE { $$->op = OP_GE; }
| notEQUAL { $$->op = OP_NE; }
;
%%

struct nodeType *ASTRoot;

struct nodeType* newOpNode(int op) {
    struct nodeType *node = newNode(NODE_OP);
    node->op = op;

    return node;
}

int main(int argc, char** argv) {  
  int res;
  if (argc != 3) { fprintf(stdout, "[ERROR] Please sepcify [INPUT].p and [OUTPUT].j!\n"); exit(1); }
  if (NULL == freopen(argv[1], "r", stdin)) { fprintf(stdout, "[ERROR] Cannot open file %s\n", argv[1]); exit(1); }
  yyparse();
  printf("[No Syntax Error]\n");
  printTree(ASTRoot, 0);
  semanticCheck(ASTRoot);
  if(SymbolTable.error == 0)
    printf("[No Semantic Error]\n");
  char *outFile = (char *)malloc(sizeof(char)*(strlen(argv[2])+3));
  strcpy(outFile, argv[2]); strcat(outFile, ".j");
  if (NULL == freopen(outFile, "w", stdout)) { fprintf(stderr, "[ERROR] Cannot write to file %s\n", outFile); exit(1);}
  genCode(ASTRoot, argv[2]);
  int i; for(i=0;i<=SymbolTable.scope_num; i++) printSymTable(i);
  return 0;
}
