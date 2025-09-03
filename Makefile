# Makefile

parser: lex.yy.c parser.tab.c
	$(CC) -o parser lex.yy.c parser.tab.c

lex.yy.c: lex.l
	flex lex.l

parser.tab.c: parser.y
	bison -d -v parser.y

clean:
	rm -f parser lex.yy.c parser.tab.c parser.tab.h
