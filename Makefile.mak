YACC=bison
LEX=flex

all: 120

.c.o:
	gcc -c -g $<

120: 120++.o parser.tab.o lexer.o rules.o
	gcc -g -o 120 120++.o parser.tab.o lexer.o rules.o

120++.o: 120++.c parser.tab.h rules.c
	gcc -c -g 120++.c 

rules.o: rules.c
	gcc -c -g rules.c 
    
parser.tab.c parser.tab.h: parser.y
	$(YACC) -dtv parser.y

lexer.c: lexer.l
	$(LEX) -t lexer.l >lexer.c

lexer.o: parser.tab.h

clean:
	rm -f 120++ *.o
	rm -f lexer.c parser.tab.c parser.tab.h
