TARGETS		= award booksort genlist dbsearch nesfa gennesfa nesfasearch als genal readlist

all:		$(TARGETS)

award:		award.c
		gcc -O -o award award.c

booksort:	booksort.c
		gcc -O -o booksort booksort.c

genlist:	genlist.c
		gcc -O -o genlist genlist.c

dbsearch:	dbsearch.c
		gcc -O -o dbsearch dbsearch.c

nesfa:		nesfa.c
		gcc -O -o nesfa nesfa.c

als:		als.c
		gcc -O -o als als.c

nesfasearch:	nesfasearch.c
		gcc -O -o nesfasearch nesfasearch.c

gennesfa:	gennesfa.c
		gcc -O -o gennesfa gennesfa.c

genal:		genal.c
		gcc -O -o genal genal.c

readlist:	readlist.c
		gcc -DSVR4 -O -o readlist readlist.c

clean:
		rm -f $(TARGETS)
