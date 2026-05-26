#
#     (C) COPYRIGHT 1994 Al von Ruff
#         ALL RIGHTS RESERVED
#
#

all:	novels search authors exact_author exact_title

novels:		novels.o
		$(CC) -O -o novels novels.o

search:		search.o
		$(CC) -O -o search search.o

authors:	authors.o
		$(CC) -O -o authors authors.o

exact_author:	exact_author.o
		$(CC) -O -o exact_author exact_author.o

exact_title:	exact_title.o
		$(CC) -O -o exact_title exact_title.o

clean:
	rm -f novels search authors exact_author
	rm -f *.o
