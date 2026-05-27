# isfdb-preISFDB

This repo contains the so-called "awards database" that preceded the original ISFDB. This consisted predominately of a set of
command line tools that ran on Unix, but also includes a prototype bibliographic tool that emitted some limited HTML. 
These files were thought lost, but were found on a hard drive from an old non-functional Fujitsu Transmeta laptop. Work on these
files began sometime in 1993, with copyright headers added in 1994, and the newest information in the dbase files from early 1995.

The files are presented as-was, but with some minor changes so that they can be compiled and run on modern Linux systems:
* Addition of function declarations
* Addition of a few includes, mostly <string.h>, <unistd.h>, and <stdlib.h>
* Conversion from lex/yacc to flex/bison. These changes were restricted to Makefile changes.
* Filesystem layout was slightly altered, as it relied on directory structures on machines that no longer exist.
* 1994 was a different time, so email address and phone numbers have been redacted.

The utility of the tools has been greatly superceded by the ISFDB and sfadb, and are shown here for archeological purposes.
These files were not previously released publicly, so they have been given an MIT license.

# Contents

## awards
**GENERAL**
This was the first set of online tools created.
* award.c        - Provides a variety of search terms for the Hugos and Nebulas.
* dbsearch.c     - Provides numerical analytics to score works based on award performance.
* genlist.c      - Outputs a Hugo or Nebula listing, formatted exactly as by Evelyn C. Leeper in rec.arts.sf.written.

**NESFA**
This was the next set of tools. This combines the Hugos and Nebulas with The NESFA Core Reading List of Fantasy and Science Fiction.
* nesfa.c        - Provides a variety of search terms for Hugos/Nebula/NESFA.
* nesfasearch.c  - Provides numerical analytics to score works based on award performance.
* gennesfa.c     - Outputs a NESFAlisting, formatted exactly as by the Core Reading List.

**PERSONAL**
This was a personal set of tools. Based on the NESFA toolset, it added additional awards: Locus, PK Dick, Clarke, and World Fantasy Award.
* als.c          - Very similar to nesfa.c, but now focused on awards again.
* alsearch.c     - Similar to nesfasearch.c, but contains more analytics
* genal.c        - Similar to gennesfa.c, but awards-centric
* readlist.c     - Generates a reading list based on award scores.

**UTILITIES**
* booksort.c     - sorts book titles according to leading A, An, The rules.

## books
* books.c      - General purpose tool, able to output a wide variety of award information, but limited to novels. Supported Hugos, Nebulas, Locus Poll, Campbell, Stoker, Clarke, PK Dick, and World Fantasy Award. This code was clearly being expanded to support other works like short fiction, but there are no short fiction entries in the provided dbase file.

## SFdbase
Experimental ISFDB1 prototype code. Generates some HTML, but no links
* search.c       - Given an author substring, finds all matching canonical names. Useful for an initial search.
* novels.c       - Can search for authors, titles, or authors
* authors.c      - Outputs a non-HTML bibliography in John Wenn format if given a canonical name
* exact_author.c - An update to authors.c, with output of some rudimentary HTML, but no links. This file became the basis for the ISFDB file a_exact_author.c
* exact_title    - Given an exact title, author, year, and type, outputs a line from the database, which contains the exact title, author, year, and type. Of dubious utility, likely a dbase debugging tool.

Example output from authors.c:

    ./authors -a "Isaac Asimov"

    Asimov, Isaac     (U.S.S.R., 1/2/1920-4/6/1992)

    Series:
        Trantorian Empire
                Pebble in the Sky (1950)
                The Stars Like Dust (1951)

        Foundation
                Foundation (1951)
                Foundation and Empire (1952)
                Second Foundation (1953)
                Foundation's Edge (1982)
                Foundation and Earth (1986)
                Prelude to Foundation (1988)

        The Positronic Robot Stories
                The Caves of Steel (1954)
                The Naked Sun (1957)
                The Robots of Dawn (1983)
                Robots and Empire (1985)

    Novels:
    I, Robot (1950) [C]
    The End of Eternity (1955)
    The Gods Themselves (1972)

    Short Fiction:
    Trends (1939)
    Strange Playfellow (1940)
    Liar! (1941)
    Nightfall (1941)
    Blind Alley (1945)
    The Mule (1945)
    ... and so on ...

This was the starting point for the ISFDB, as data missing from existing John Wenn bibliographies could be added until parity was reached.

# Background
While a student at the University of Illinois in the late 80s, I had taken two semesters of writer’s workshops, and had been fully indoctrinated into the realm of academic literature, along with an accompanying low regard for genre fiction. On the other hand, I had been a member of the Science Fiction Book Club since 1970, and occasionally forgot to send in the monthly slip indicating my rejection of the automatic monthly selection, so I would sometimes receive a surprise book. In August 1991, I had once again forgotten to send in the rejection slip, and received a copy of Gardner Dozois’ *The Year's Best Science Fiction: Eighth Annual Collection*. Instead of sending it back, I decided to read these stories while on the exercise bike, and found myself surprised at the overall level of quality of the stories.

From late 1991 through 1992, I used the gopher tool to locate recent lists of Hugo and Nebula award winners, maintained by Evelyn C. Leeper. I then started reading through the most recent novel winners and nominees. Given my reading hiatus from the field, there were a lot of books on the lists, so I wanted to first focus on books that had won both the Hugo and the Nebula - so I made a spreadsheet. Over the course of 1992 and 1993 the spreadsheet grew to be quite elaborate, containing many more awards, and moved into tracking short fiction as well, and recording the location of anthologies where the stories appeared. I still occasionally find printouts from these spreadsheets in the back of anthologies in my library, when my reading focus was on short fiction in late 1993 and early 1994. 

The Venn diagram overlap between readers of Speculative Fiction and UNIX developers is quite high, so in 1994 I took the spreadsheets, converted them to .csv files, introduced column delimiters, and made a set of Unix command-line tools to perform search and formatting of SF award information, and made those tools available for use on Unix systems at work. This repo is a copy of those tools.

The bottomline motivation was to help find the top books to read. As such, there are numerous ways to slice and dice the data, with accompanying scoring algorithms. Again, it's utility has been eclipsed by more modern websites.

# Usage

## award.c

Operates strictly on Hugo and Nebula data.

    [-y year]                    (Display works from a particular year)
    [-t title]                   (Display titles containing the given substring)
    [-a author]                  (Display works by authors containing the given substring)
    [-h]                         (Display Hugos only)
    [-n]                         (Display Nebulas only)
    [-c (n|nv|nt|ss) or 
        (!n|!nv|!nt|!ss)         (Display - or not - fiction by length)
         n  = Novel
         nv = Novella
         nt = Novelette
         ss = Sorth Story
    [-w]                         (Display winners only)
    [-g] (1|2|3|4)               (Display a given award level)
         1 = Hugo Novel Nominees
         2 = Hugo Short Fiction Nominees
         3 = Nebula Novel Nominees
         4 = Nebula Short Fiction Nominees

## dbsearch.c

Provides numerical analytics to score works based on award performance. The concept of "heat" was the first appearance of scoring authors based on how recent their award record occured.

    [-s (n|w|h)           (nominations|winners|heat)]
    [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)
    [-h]                  (Hugos only)
    [-n]                  (Nebulas only)
    [-a]                  (Scan for author errors. <sensitivity> (0.0 - 2.0))
    [-t]                  (Scan for title errors. <sensitivity> (0.0 - 2.0))
    [-m]                  (Merge duplicate records)
    [-b]                  (Display year data)
    [-d]                  (Perform peak)
    [-y]                  (Display titles under max age)

## genlist.c

Outputs a Hugo or Nebula listing, formatted exactly as by Evelyn C. Leeper in rec.arts.sf.written

    [-c] (l | s)          (Display long/short fiction)
    [-h]                  (Display Hugos only)
    [-n]                  (Display Nebulas only)
    [-b]                  (Display works which won both awards)

## nesfa.c

These tools were structured around The NESFA Core Reading List of Fantasy and Science Fiction. The dbase file also contained Hugo/Nebula info. This particular tool only used the award side of things, and was never meant to be user facing, as it simply dumps database records that matches the search criteria.

    [-b]                  (Display works which won both awards)
    [-y year]             (Display works from a given year)
    [-t title]            (Display works with the title substring)
    [-a author]           (Display works with the author substring)
    [-h]                  (Hugos only)
    [-n]                  (Nebulas only) 
    [-c (n|nv|nt|ss) or 
        (!n|!nv|!nt|!ss)  (Search by work length)
    [-w]                  (Display winners only)

## nesfasearch.c

This tool pulls from the awards and NESFA reading list, and outputs results in the NESFA Reading List format, including the NESFA encodings for the source location (See: https://www.nesfa.org/awards/yearly-hugo-lists/the-nesfa-core-reading-list-of-fantasy-and-science-fiction/)

    [-s (n|w|h)           (nominations|winners|heat)]
    [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)
    [-h]                  (Hugos only)
    [-n]                  (Nebulas only)
    [-a]                  (Scan for author errors. <sensitivity> (0.0 - 2.0))
    [-t]                  (Scan for title errors. <sensitivity> (0.0 - 2.0))
    [-b]                  (Perform title sort)

## gennesfa.c

    [-c (l|s) ]                       (long/short fiction)
    [-w (a|b|c|d|e|f|g|h|z|n|N|H)]    (Search by NESFA encodings)
    [-h]                              (Hugos only)
    [-n]                              (Nebulas only)

## als.c

Very similar to nesfa.c, but adds supports for Locus

    [-n]                              (Nebulas only)
    [-y year]                         (Display works from a given year)
    [-t title]                        (Display works with the title substring)
    [-a author]                       (Display works with the author substring)
    [-h]                              (Hugos only)
    [-n]                              (Nebulas only)
    [-c (n|nv|nt|ss) or (!n|!nv|!nt|!ss) ]
    [-w]                              (Winners only)

## alsearch.c

Updated version of nesfasearch.c, but containing more analytics.    

    [-s (n|w|h) (nominations|winners|heat)]\n");
    [-a] <sensitivity> (0.0 - 2.0)\n");
    [-b] (perform title sort)\n");
    [-t] <sensitivity> (0.0 - 2.0)\n");
    [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)\n");
    [-h] (Hugos only)\n");
    [-n] (Nebulas only)\n");

## genal.c

Updated version of gennesfa.c

## readlist.c

Generates a reading list based on award scores. Includes Hugos, Nebulas, Locus, Campbell, Stoker, Clarke,

## booksort.c

Utility that alphabetizes book titles, using standard rules of ignoring leading a, A, an, An, the, The

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
