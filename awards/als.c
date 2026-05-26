#ident "%W%	%G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */


#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DBASE "dbases/als.dbase"
#define OPTARGS "hnwy:t:a:c:"

#define SHORT_STORY	0x0001
#define NOVELETTE	0x0002
#define NOVELLA		0x0004
#define NOVEL		0x0010
#define NONFICTION	0x0100
#define COLLECTION	0x0200
#define ANTHOLOGY	0x0400
#define SERIES		0x0800
#define SHORT_FICTION	(SHORT_STORY|NOVELETTE|NOVELLA)
#define ALL		(SHORT_STORY|NOVELETTE|NOVELLA|NOVEL)

#define WEBDAM		0x0001	/* [a] */
#define ASFANTH		0x0002	/* [b] */
#define SFHALL		0x0004	/* [c] */
#define HEALY		0x0008	/* [d] */
#define GROFF		0x0010	/* [e] */
#define ASIMOV		0x0020	/* [f] */
#define HARTWELL	0x0040	/* [g] */
#define HUGO		0x0080  /* [h] ([H] = winner) */
#define NESFA		0x0100  /* [i] */
#define NEBULA		0x0200  /* [n] ([N] = winner) */
#define HUGOWIN		0x0400  /* [H] = winner */
#define NEBULAWIN	0x0800  /* [N] = winner */
#define ASSFS		0x1000  /* [j] */
#define LOCUS		0x2000  /* [l] */
#define UNKNOWN		0x8000

#define MAX_TITLE_WIDTH 32

extern char * optarg;

static void
usage()
{
	(void)printf("award [-y year]\n");
	(void)printf("      [-t title]\n");
	(void)printf("      [-a author]\n");
	(void)printf("      [-h] (Hugos only)\n");
	(void)printf("      [-n] (Nebulas only)\n");
	(void)printf("      [-c (n|nv|nt|ss) or (!n|!nv|!nt|!ss) ]\n");
	(void)printf("      [-w] (winners)\n");
	exit(1);
}

static char	rec_title[256];
static char	rec_author[256];
static int	rec_year;
static int	rec_award;
static int	rec_winner;
static int	rec_type;

static void
print_record(rec)
	struct record *rec;
{
	int length;

	length = 0;
	if (rec_award == 0) {
		printf("  ");
	} else {

		printf("["); 
		if (rec_award & NEBULA) {
			printf("n");
			length++;
		}
        	if (rec_award & NEBULAWIN) {
			printf("N");
			length++;
		}
        	if (rec_award & WEBDAM) {
			printf("a");
			length++;
		}
        	if (rec_award & ASFANTH) {
			printf("b");
			length++;
		}
        	if (rec_award & SFHALL) {
			printf("c");
			length++;
		}
        	if (rec_award & HEALY) {
			printf("d");
			length++;
		}
        	if (rec_award & GROFF) {
			printf("e");
			length++;
		}
        	if (rec_award & ASSFS) {
			printf("j");
			length++;
		}
        	if (rec_award & LOCUS) {
			printf("l");
			length++;
		}
        	if (rec_award & ASIMOV) {
			printf("f");
			length++;
		}
        	if (rec_award & HARTWELL) {
			printf("g");
			length++;
		}
        	if (rec_award & HUGO) {
			printf("h");
			length++;
		}
        	if (rec_award & HUGOWIN) {
			printf("H");
			length++;
		}
        	if (rec_award & NESFA) {
			printf("z");
			length++;
		}
		printf("]"); 
	}

	for(;length<10; length++)
		printf(" ");

	if (rec_type == NOVEL) {
		printf("n  ");
	} else if (rec_type == NOVELLA) {
		printf("nv ");
	} else if (rec_type == NOVELETTE) {
		printf("nt ");
	} else if (rec_type == SHORT_STORY) {
		printf("ss ");
	} else if (rec_type == SHORT_FICTION) {
		printf("sf ");
	} else if (rec_type == NONFICTION) {
		printf("nf ");
	} else if (rec_type == COLLECTION) {
		printf("c  ");
	} else if (rec_type == ANTHOLOGY) {
		printf("a  ");
	} else if (rec_type == SERIES) {
		printf("se ");
	} else if (rec_type == UNKNOWN) {
		printf("u  ");
	}

	printf("%4d ", rec_year);
	printf("%s  ", rec_title);
	length = strlen(rec_title);
	if (length <= MAX_TITLE_WIDTH) {
		for(length = MAX_TITLE_WIDTH-length; length; length--)
			 printf(" ");
		printf("%s\n", rec_author);
	} else {
		printf(" --\n");
		for(length = MAX_TITLE_WIDTH+22; length; length--)
			 printf(" ");
		printf("%s\n", rec_author);
	}
}


static int
parse_record(fp)
	FILE *fp;
{
	int in;
	char *tmp;
	char buffer[32];

	/*
	 * Parse Title
	 */
	tmp = rec_title;
	/*CONSTCOND*/
	while(1) {
		in = getc(fp);
		if (in == -1) {
			return (0);
		} else if (in == '|') {
			break;
		} else {
			*tmp = (char)in;
			tmp++;
		}
	}
	*tmp = 0;

	/*
	 * Parse Author
	 */
	tmp = rec_author;
	/*CONSTCOND*/
	while(1) {
		in = getc(fp);
		if (in == -1) {
			return (0);
		} else if (in == '|') {
			break;
		} else {
			*tmp = (char)in;
			tmp++;
		}
	}
	*tmp = 0;

	/*
	 * Parse Story Type
	 */
	tmp = buffer;
	/*CONSTCOND*/
	while(1) {
		in = getc(fp);
		if (in == -1) {
			return (0);
		} else if (in == '|') {
			break;
		} else {
			*tmp = (char)in;
			tmp++;
		}
	}
	*tmp = 0;
	if (strcmp(buffer, "n") == 0) {
		rec_type = NOVEL;
	} else if (strcmp(buffer, "nv") == 0) {
		rec_type = NOVELLA;
	} else if (strcmp(buffer, "nt") == 0) {
		rec_type = NOVELETTE;
	} else if (strcmp(buffer, "ss") == 0) {
		rec_type = SHORT_STORY;
	} else if (strcmp(buffer, "nf") == 0) {
		rec_type = NONFICTION;
	} else if (strcmp(buffer, "c") == 0) {
		rec_type = COLLECTION;
	} else if (strcmp(buffer, "a") == 0) {
		rec_type = ANTHOLOGY;
	} else if (strcmp(buffer, "se") == 0) {
		rec_type = SERIES;
	} else if (strcmp(buffer, "sf") == 0) {
		rec_type = SHORT_FICTION;
	} else if (strcmp(buffer, "u") == 0) {
		rec_type = UNKNOWN;
	} else {
		(void)printf("Bogus Story Type = (%s)\n", buffer);
		printf("Last good author = %s\n", rec_author);
		printf("Last good title = %s\n", rec_title);
		return(0);
	}

	/*
	 * Parse Year
	 */
	tmp = buffer;
	/*CONSTCOND*/
	while(1) {
		in = getc(fp);
		if (in == -1) {
			return (0);
		} else if (in == '|') {
			break;
		} else {
			*tmp = (char)in;
			tmp++;
		}
	}
	*tmp = 0;
	(void)sscanf(buffer,"%d", &rec_year);

	/*
	 * Parse Award
	 */
	tmp = buffer;
	/*CONSTCOND*/
	while(1) {
		in = getc(fp);
		if (in == -1) {
			return (0);
		} else if (in == '\n') {
			break;
		} else {
			*tmp = (char)in;
			tmp++;
		}
	}
	*tmp = 0;

	rec_award = 0;
	rec_winner = 0;
	if (strchr(buffer, 'n'))
		rec_award |= NEBULA;
	if (strchr(buffer, 'N')) {
		rec_award |= NEBULAWIN;
		rec_winner = 1;
	}
	if (strchr(buffer, 'a'))
		rec_award |= WEBDAM;
	if (strchr(buffer, 'b'))
		rec_award |= ASFANTH;
	if (strchr(buffer, 'c'))
		rec_award |= SFHALL;
	if (strchr(buffer, 'd'))
		rec_award |= HEALY;
	if (strchr(buffer, 'e'))
		rec_award |= GROFF;
	if (strchr(buffer, 'f'))
		rec_award |= ASIMOV;
	if (strchr(buffer, 'g'))
		rec_award |= HARTWELL;
	if (strchr(buffer, 'h'))
		rec_award |= HUGO;
	if (strchr(buffer, 'H')) {
		rec_award |= HUGOWIN;
		rec_winner = 1;
	}
	if (strchr(buffer, 'j'))
		rec_award |= ASSFS;
	if (strchr(buffer, 'l'))
		rec_award |= LOCUS;
	if (strchr(buffer, 'z'))
		rec_award |= NESFA;

	return(1);
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int option;
	char title[256] = {0};
	char author[256] = {0};
	int year         = 0;
	int hugos_only   = 0;
	int nebulas_only = 0;
	int winners_only = 0;
	int categories   = 0;
	FILE *fp;

	if (argc < 2) {
		usage();
	}

	while ((option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {

		case '?':	usage();
				break;

		case 'a': 	(void)strcpy(author, optarg);
				break;

		case 'c':	if (strcmp(optarg, "n") == 0) {
					categories |= NOVEL;
				} else if (strcmp(optarg, "nv") == 0) {
					categories |= NOVELLA;
				} else if (strcmp(optarg, "nt") == 0) {
					categories |= NOVELETTE;
				} else if (strcmp(optarg, "ss") == 0) {
					categories |= SHORT_STORY;
				} else if (strcmp(optarg, "!n") == 0) {
					if (categories == 0)
						categories = ~NOVEL;
					else
						categories &= ~NOVEL;
				} else if (strcmp(optarg, "!nv") == 0) {
					if (categories == 0)
						categories = ~NOVELLA;
					else
						categories &= ~NOVELLA;
				} else if (strcmp(optarg, "!nt") == 0) {
					if (categories == 0)
						categories = ~NOVELETTE;
					else
						categories &= ~NOVELETTE;
				} else if (strcmp(optarg, "!ss") == 0) {
					if (categories == 0)
						categories = ~SHORT_STORY;
					else
						categories &= ~SHORT_STORY;
				}
				break;

		case 'h': 	if (nebulas_only) {
					(void)printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					hugos_only = 1;
				}
				break;

		case 'n': 	if (hugos_only) {
					(void)printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					nebulas_only = 1;
				}
				break;

		case 't': 	(void)strcpy(title, optarg);
				break;

		case 'w':	winners_only = 1;
				printf("Wins only\n");
				break;

		case 'y': 	(void)sscanf(optarg,"%d", &year);
				break;

		}
	}

	if (categories == 0)
		categories = ALL;

	fp = fopen(DBASE, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}

	while(parse_record(fp)) {

		if (year && (year != rec_year))
			continue;

		if (winners_only) {
			if (rec_winner) {
				if (hugos_only && ((rec_award & HUGOWIN) == 0) )
					continue;
				if (nebulas_only && ((rec_award & NEBULAWIN) == 0) )
					continue;
			} else {
				continue;
			}
		}

		if (hugos_only && ((rec_award & (HUGO|HUGOWIN)) == 0) )
			continue;

		if (nebulas_only && ((rec_award & (NEBULA|NEBULAWIN)) == 0) )
			continue;

		if ((categories != ALL) && ((rec_type & categories) == 0) )
			continue;

		if (title[0] && !(strstr(rec_title, title))) 
			continue;
			
		if (author[0] && !(strstr(rec_author, author))) 
			continue;
			
		print_record();
	}
	exit(0);
	return(0);
}
