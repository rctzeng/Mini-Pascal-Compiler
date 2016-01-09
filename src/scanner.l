%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
#include "node.h"

struct nodeType* newTokenNode(int tokenType);

int line_no = 1;
int chr_no = 0;

#define YY_USER_ACTION chr_no+=yyleng;

%}

A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]

DIGIT [0-9]

%%
{A}{R}{R}{A}{Y}              {yylval.node = newTokenNode(ARRAY); return(ARRAY);}
{C}{O}{N}{S}{T}              {yylval.node = newTokenNode(PCONST); return(PCONST);}
{D}{O}                       {yylval.node = newTokenNode(DO); return(DO);}
{D}{O}{W}{N}{T}{O}           {yylval.node = newTokenNode(DOWNTO); return(DOWNTO);}
{E}{L}{S}{E}                 {yylval.node = newTokenNode(ELSE); return(ELSE);}
{E}{N}{D}                    {yylval.node = newTokenNode(END); return(END);}
{F}{O}{R}                    {yylval.node = newTokenNode(FOR); return(FOR);}
{F}{U}{N}{C}{T}{I}{O}{N}     {yylval.node = newTokenNode(FUNCTION); return(FUNCTION);}
{G}{O}{T}{O}                 {yylval.node = newTokenNode(GOTO); return(GOTO);}
{I}{F}                       {yylval.node = newTokenNode(IF); return(IF);}
{I}{D}                       {yylval.node = newTokenNode(ID); return(ID);}
{I}{N}{T}{E}{G}{E}{R}        {yylval.node = newTokenNode(INTEGER); return(INTEGER);}
{L}{A}{B}{E}{L}              {yylval.node = newTokenNode(LABEL); return(LABEL);}
{N}{O}{T}                    {yylval.node = newTokenNode(NOT); return(NOT);}
{N}{U}{M}                    {yylval.node = newTokenNode(NUM); return(NUM);}
{O}{F}                       {yylval.node = newTokenNode(OF); return(OF);}
{B}{E}{G}{I}{N}              {yylval.node = newTokenNode(PBEGIN); return(PBEGIN);}
{P}{R}{O}{C}{E}{D}{U}{R}{E}  {yylval.node = newTokenNode(PROCEDURE); return(PROCEDURE);}
{P}{R}{O}{G}{R}{A}{M}        {yylval.node = newTokenNode(PROGRAM); return(PROGRAM);}
{R}{E}{A}{L}                 {yylval.node = newTokenNode(REAL); return(REAL);}
{R}{E}{P}{E}{A}{T}           {yylval.node = newTokenNode(REPEAT); return(REPEAT);}
{S}{T}{R}{I}{N}{G}           {yylval.node = newTokenNode(STRING); return(STRING);}
{T}{H}{E}{N}                 {yylval.node = newTokenNode(THEN); return(THEN);}
{T}{O}                       {yylval.node = newTokenNode(TO); return(TO);}
{T}{Y}{P}{E}                 {yylval.node = newTokenNode(TYPE); return(TYPE);}
{U}{N}{T}{I}{L}              {yylval.node = newTokenNode(UNTIL); return(UNTIL);}
{V}{A}{R}                    {yylval.node = newTokenNode(VAR); return(VAR);}
{W}{H}{I}{L}{E}              {yylval.node = newTokenNode(WHILE); return(WHILE);}
{W}{I}{T}{H}                 {yylval.node = newTokenNode(WITH); return(WITH);}
[a-zA-Z]([a-zA-Z0-9])*       {yylval.node = newTokenNode(IDENTIFIER); yylval.node->string = (char*) malloc(yyleng+1); strcpy(yylval.node->string, yytext); return(IDENTIFIER);}
{DIGIT}*[a-zA-Z]+            fprintf(stdout, "[ERROR] %s Bad IDENTIFIER name on %d:%d\n", yytext, line_no, chr_no-yyleng+1);
[a-zA-Z]+([@$?_])+  fprintf(stdout, "[ERROR] %s Bad IDENTIFIER name\n", yytext);
\n                           {line_no++; chr_no = 0;}
":"                          {yylval.node = newTokenNode(COLON); return(COLON);}
":="                         {yylval.node = newTokenNode(ASSIGNMENT); return(ASSIGNMENT);}
","                          {yylval.node = newTokenNode(COMMA); return(COMMA);}
{DIGIT}+                     {yylval.node = newTokenNode(DIGSEQ); yylval.node->valueValid = VALUE_I_VALID; yylval.node->iValue = atoi(yytext); return(DIGSEQ);}
"."                          {yylval.node = newTokenNode(DOT); return(DOT);}
".."                         {yylval.node = newTokenNode(DOTDOT); return(DOTDOT);}
"="                          {yylval.node = newTokenNode(EQUAL); yylval.node->op = OP_EQ; return(EQUAL);}
"!="                         {yylval.node = newTokenNode(notEQUAL); yylval.node->op = OP_NE; return(notEQUAL);}
">="                         {yylval.node = newTokenNode(GE); yylval.node->op = OP_GE; return(GE);}
">"                          {yylval.node = newTokenNode(GT); yylval.node->op = OP_GT; return(GT);}
"<="                         {yylval.node = newTokenNode(LE); yylval.node->op = OP_LE; return(LE);}
"["                          {yylval.node = newTokenNode(LBRACE); return(LBRACE);}
"("                          {yylval.node = newTokenNode(LPAREN); return(LPAREN);}
"<"                          {yylval.node = newTokenNode(LT); yylval.node->op = OP_LT; return(LT);}
"-"                          {yylval.node = newTokenNode(MINUS); yylval.node->op = OP_SUB; return(MINUS);}
"+"                          {yylval.node = newTokenNode(PLUS); yylval.node->op = OP_ADD; return(PLUS);}

({DIGIT}+|{DIGIT}+"."{DIGIT}+)E(''|[+-])(''|{DIGIT}+|{DIGIT}+"."{DIGIT}+)     return(NUMBER);
{DIGIT}+"."{DIGIT}+          {yylval.node = newTokenNode(NUMBER); yylval.node->valueValid = VALUE_R_VALID; yylval.node->rValue=atof(yytext); return(NUMBER);}
"]"                          {yylval.node = newTokenNode(RBRACE); return(RBRACE);}
")"                          {yylval.node = newTokenNode(RPAREN); return(RPAREN);}
";"                          {yylval.node = newTokenNode(SEMICOLON); return(SEMICOLON);}
"/"                          {yylval.node = newTokenNode(SLASH); yylval.node->op = OP_DIV; return(SLASH);}
"*"                          {yylval.node = newTokenNode(STAR); yylval.node->op = OP_MUL; return(STAR);}

"//".*\n                     {fprintf (stdout, "%s", yytext); chr_no = 0; line_no++;}
\'([^\'\n]|\\\'|'')*\'       {yylval.node = newTokenNode(CHARACTER_STRING); yylval.node->string = yytext; return(CHARACTER_STRING);}
\"([^\"\n]|\\\"|'')*\"       {yylval.node = newTokenNode(CHARACTER_STRING); yylval.node->string = yytext; return(CHARACTER_STRING);}

\'([^\']|\\\'|\n)*\' {
fprintf(stdout, "line %d:%d Cross-Line String %s\n", line_no, chr_no-yyleng+1, yytext);
register int c;
for (c = 0; c < strlen(yytext); c++)
       if (c == '\n') line_no++;
}
\"([^\"]|\\\"|\n)*\" {
fprintf(stdout, "line %d:%d Cross-Line String %s\n", line_no, chr_no, yytext);
register int c;
for (c = 0; c < strlen(yytext); c++)
       if (c == '\n') line_no++;
}
\"([^\"\n])*$ fprintf(stdout, "[ERROR] Runaway String %s detected on %d:%d\n", yytext, line_no, chr_no-yyleng+1);
\'([^\'\n])*$ fprintf(stdout, "[ERROR] Runaway String %s detected on %d:%d\n", yytext, line_no, chr_no-yyleng+1);
"(*"   |
"{"    { register int c;
     while ((c = input()))
     {
      if (c == '}')
       break;
      else if (c == '*')
      {
       if ((c = input()) == ')')
        break;
       else
        unput (c);
      }
      else if (c == '\n')
       line_no++;
      else if (c == 0)
        fprintf (stdout, "[ERROR] Unexpected EOF inside comment on line %d\n",line_no);
     }
    }

[ \t\f]    ;

\r         ;
.    {fprintf(stdout,"[ERROR] Lexical analyzer error on line %d : %s\n",line_no,yytext);}
<<EOF>>    return END_OF_FILE;

%%/
/*
int main(int argc, char** argv) {
  int i, tok, prev_c, fst_chr;
  int prev_l = 1;
  if (argc != 2) {
    fprintf(stdout, "[ERROR] Input File Nout Found!\n");
    exit(1);
  }
  yyin = fopen(argv[1], "r");
  for (i = 0; 1==1; i ++) {
    tok = yylex();
    fst_chr = chr_no-yyleng+1;
    if (tok == IDENTIFIER) fprintf(stdout, "line %d:%d IDENTIFIER %.10s\n", line_no, fst_chr, yytext);
    else if (tok == NUMBER || tok == DIGSEQ) fprintf(stdout, "line %d:%d NUMBER %s\n", line_no, fst_chr, yytext);
    else if (tok == CHARACTER_STRING) fprintf(stdout, "line %d:%d CHARACTER_STRING %s\n", line_no, fst_chr, yytext);
    else fprintf(stdout, "line %d:%d %s\n", line_no, fst_chr, yytext);
  }
  return(0);
} */

yywrap() {
  fclose(yyin);
  exit(1);
}

struct nodeType* newTokenNode(int tokenType) {
  struct nodeType *node = newNode(NODE_TOKEN);
  node->tokenType = tokenType;
  return node;
}
