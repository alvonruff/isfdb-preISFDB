#
#     (C) COPYRIGHT 1994 Al von Ruff
#         ALL RIGHTS RESERVED
#
#

YACC	= /usr/bin/bison
LEX	= /usr/bin/flex
OBJS	= parser.tab.o \
	  lex.yy.o \
	  books.o

all:		books

books:	$(OBJS)
		$(CC) $(CFLAGS) -o $@ $(OBJS)

lex.yy.o:	parser.l
		$(LEX) parser.l
		$(CC) $(CFLAGS) -c lex.yy.c

parser.tab.o:	parser.y
		$(YACC) -d parser.y
		$(CC) $(CFLAGS) -c parser.tab.c

books.o:	books.c
		$(CC) $(CFLAGS) -c books.c

clean:
		rm -f *.o
		rm -f parser.tab.h
		rm -f parser.tab.c
		rm -f lex.yy.c
		rm -f books
