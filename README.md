# isfdb-preISFDB

This repo contains the so-called "awards database" that preceded the original ISFDB. This consisted predominately of a set of
command line tools that ran on Unix, but also includes a prototype bibliographic tool that emitted some limited HTML. 
These files were thought lost, but were found on a hard drive from an old non-functional Fujitsu Transmeta laptop.

The files are presented as-was, but with some minor changes so that they can be compiled and run on modern Linux systems:
* Addition of function declarations
* Addition of a few includes, mostly <string.h>, <unistd.h>, and <stdlib.h>
* Conversion from lex/yacc to flex/bison. These changes were restricted to Makefile changes.
* Filesystem layout was slightly altered, as it relied on directory structures on machines that no longer exist.
* 1994 was a different time, so email address and phone numbers have been redacted.

The utility of the tools has been greatly superceded by the ISFDB and sfadb, and are shown here for archeological purposes.
These files were not previously released publicly, so they have been given an MIT license.

# Contents

## books
* books.c      - General purpose tool, able to output a wide variety of award information, but limited to novels.

## awards
**GENERAL**
* award.c        - Strictly Hugos and Nebulas
* dbsearch.c     - Provides numerical analytics to score works based on award performance
* genlist.c      - Outputs a Hugo or Nebula listing, formatted exactly as by Evelyn C. Leeper in rec.arts.sf.written

**NESFA**
Structured around The NESFA Core Reading List of Fantasy and Science Fiction
* nesfa.c        -
* nesfasearch.c
* gennesfa.c

**PERSONAL**
* als.c          - Very similar to nesfa.c; supports Locus
* alsearch.c     - Similar to nesfasearch.c, but contains more analytics
* genal.c        - Related to gennesfa.c
* readlist.c     - Generates a reading list based on award scores. Includes Hugos, Nebulas, Locus, Campbell, Stoker, Clarke,

**UTILITIES**
* booksort.c     - sorts book titles

## SFdbase
Experimental ISFDB1 prototype code. Generates some HTML, but no links
* search.c       - Given an author substring, finds all matching canonical names
* novels.c       - Can search for authors, titles, or authors
* authors.c      - Outputs a non-HTML bibliography if given a canonical name
* exact_author.c - Nearly the same as authors.c, but output rudimentary HTML, but no links
* exact_title    - Given an exact title, author, year, and type, outputs a line from the database, which contains the exact title, author, year, and type. Of dubious utility, likely a dbase debugging tool.

# Background
While a student at the University of Illinois in the late 80s, I had taken two semesters of writer’s workshops, and had been fully indoctrinated into the realm of academic literature, along with an accompanying low regard for genre fiction. On the other hand, I had been a member of the Science Fiction Book Club since 1970, and occasionally forgot to send in the monthly slip indicating my rejection of the automatic monthly selection, so I would sometimes receive a surprise book. In August 1991, I had once again forgotten to send in the rejection slip, and received a copy of Gardner Dozois’ *The Year's Best Science Fiction: Eighth Annual Collection*. Instead of sending it back, I decided to read these stories while on the exercise bike, and found myself surprised at the overall level of quality of the stories.

From late 1991 through 1992, I used the gopher tool to locate recent lists of Hugo and Nebula award winners, maintained by Evelyn C. Leeper. I then started reading through the most recent novel winners and nominees. Given my reading hiatus from the field, there were a lot of books on the lists, so I wanted to first focus on books that had won both the Hugo and the Nebula - so I made a spreadsheet. Over the course of 1992 and 1993 the spreadsheet grew to be quite elaborate, containing many more awards, and moved into tracking short fiction as well, and recording the location of anthologies where the stories appeared. I still occasionally find printouts from these spreadsheets in the back of anthologies in my library, when my reading focus was on short fiction in late 1993 and early 1994. 

The Venn diagram overlap between readers of Speculative Fiction and UNIX developers is quite high, so in 1994 I took the spreadsheets, converted them to .csv files, introduced column delimiters, and made a set of Unix command-line tools to perform search and formatting of SF award information, and made those tools available for use on Unix systems at work. This repo is a copy of those tools.

The bottomline motivation was to help find the top books to read. As such, there are numerous ways to slice and dice the data, with accompanying scoring algorithms. Again, it's utility has been eclipsed by more modern websites.

# Usage

## books.c

Novels only. Broadens the awards to include Clarke, Campbell, Locus, and Stoker

    [-a author]       (only authors with this substring)\n");
    [-A]              (list everything by author)\n");
    [-b]              (display buy list)\n");
    [-C]              (Critical Mode)\n");
    [-g ca|cl|hu|ne|lf|lh|ls|st]\n");
                  (generate an award listing\n");
    [-l]              (dump the entire database sorted by score)\n");
    [-o]              (display own list)\n");
    [-p]              (print out database (debug))\n");
    [-P]              (Popular Mode)\n");
    [-r]              (display read list)\n");
    [-s]              (top Scoring authors)\n");
    [-t title]        (only titles with this substring)\n");
    [-w]              (winners only)\n");
    [-T threshold]    (use only scores > threshold)\n");
    [-y year]         (specific year only)\n");

    Output decoder ring:
        ca = Campbell
        cl = Clarke
        hu = Hugo
        ne = Nebula
        lf = Locus Fantsy
        lh = Locus Horror
        ls = Locus Science Fiction
        st = Stoker

## award.c

Strictly Hugos and Nebulas.

    [-y year]
    [-t title]
    [-a author]
    [-h]             (Hugos only)
    [-n]             (Nebulas only)
    [-c (n|nv|nt|ss) or 
        (!n|!nv|!nt|!ss)
    [-w]             (winners only)
    [-g] (1|2|3|4) 
         1 = Hugo Novel Nominees
         2 = Hugo Short Fiction Nominees
         3 = Nebula Novel Nominees
         4 = Nebula Short Fiction Nominees

## dbsearch.c

Provides numerical analytics to score works based on award performance

    [-s (n|w|h) (nominations|winners|heat)]\n");
    [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)\n");
    [-h] (Hugos only)\n");
    [-n] (Nebulas only)\n");
    [-a] Scan for author errors. <sensitivity> (0.0 - 2.0)\n");
    [-t] Scan for title errors. <sensitivity> (0.0 - 2.0)\n");
    [-m] Merge duplicate records\n");
    [-b] Display year data\n");
    [-d] (perform peak)\n");
    [-y] Display titles under max age\n");

## genlist.c

Outputs a Hugo or Nebula listing, formatted exactly as by Evelyn C. Leeper in rec.arts.sf.written

        printf("genlist  [-c] (l | s) (long/short fiction)\n");
        printf("         [-h] (Hugos only)\n");
        printf("         [-n] (Nebulas only)\n");
        printf("         [-b] (won Both)\n");

## nesfa.c

Structured around The NESFA Core Reading List of Fantasy and Science Fiction

## nesfasearch.c
## gennesfa.c

## als.c
Very similar to nesfa.c; supports Locus

        (void)printf("award [-y year]\n");
        (void)printf("      [-t title]\n");
        (void)printf("      [-a author]\n");
        (void)printf("      [-h] (Hugos only)\n");
        (void)printf("      [-n] (Nebulas only)\n");
        (void)printf("      [-c (n|nv|nt|ss) or (!n|!nv|!nt|!ss) ]\n");
        (void)printf("      [-w] (winners)\n");

        a = WEBDAN
        b = ASFANTH
        c = SFHALL
        d = HEALY
        e = GROFF
        f = ASIMOV
        g = HARTWELL
        h = HUGO
        H = HUGO WIN
        i = NESFA
        n = NEBULA
        N = NEBULA WIN
        j = ASSFS
        l = LOCUS

## alsearch.c
Similar to nesfasearch.c, but contains more analytics       

        printf("dbsearch [-s (n|w|h) (nominations|winners|heat)]\n");
        printf("         [-a] <sensitivity> (0.0 - 2.0)\n");
        printf("         [-b] (perform title sort)\n");
        printf("         [-t] <sensitivity> (0.0 - 2.0)\n");
        printf("         [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)\n");
        printf("         [-h] (Hugos only)\n");
        printf("         [-n] (Nebulas only)\n");

## genal.c
Related to gennesfa.c

## readlist.c

Generates a reading list based on award scores. Includes Hugos, Nebulas, Locus, Campbell, Stoker, Clarke,

## booksort.c

Utility that alphabetizes book titles, using standard rules of ignoring leading a, A, an, An, the, The
