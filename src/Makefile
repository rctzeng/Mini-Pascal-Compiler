CC = cc
FLEX = flex
YACC = yacc

SCAN = scanner.l
SCAN_YY = lex.yy.c
YY_TAB = y.tab.c
YY_HEAD = y.tab.h
PARSER = parser.y
NODE = node.c
SYMTAB = symtab.c
CODEGEN = gencode
DEST_CODEGEN = ../compiled_executable/mini_pascal_compiler
BYTEFILE = foo
JASMINE = ../compiled_executable/jasmin.jar

IN = ../testcase/12_fib.p

all: $(CODEGEN)

clean_all:
	rm -rf $(DEST_CODEGEN) $(SCAN_YY) $(YY_TAB) $(YY_HEAD) $(BYTEFILE).* *.o *.output *.~
clean:
	rm -rf $(SCAN_YY) $(YY_TAB) $(YY_HEAD) $(BYTEFILE).* *.o *.output *.~
gen:
	./$(DEST_CODEGEN) $(IN) $(BYTEFILE)
	rm -rf $(CODEGEN) $(SCAN_YY) $(YY_TAB) $(YY_HEAD) *.o *.output
run:
	java -jar $(JASMINE) $(BYTEFILE).j
	java $(BYTEFILE)
$(CODEGEN):
	$(FLEX) $(SCAN)
	$(YACC) -d $(PARSER) --verbose
	$(CC) -c -o $(SCAN_YY).o $(SCAN_YY)
	$(CC) -c -o $(YY_TAB).o $(YY_TAB)
	$(CC) -c -o $(NODE).o $(NODE) -std=gnu99
	$(CC) -c -o $(SYMTAB).o $(SYMTAB) -std=gnu99
	$(CC) -c -o $(CODEGEN).o $(CODEGEN).c -std=gnu99
	$(CC) -o $(DEST_CODEGEN) $(SCAN_YY).o $(YY_TAB).o $(NODE).o $(SYMTAB).o $(CODEGEN).o -ll
