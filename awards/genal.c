#ident "%W%     %G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define DBASE "dbases/als.dbase"
#define OPTARGS "hn:wc:"
#define CURRENT_YEAR 1994

#define SHORT_STORY     0x0001
#define NOVELETTE       0x0002
#define NOVELLA         0x0004
#define NOVEL           0x0008
#define NONFICTION      0x0010
#define COLLECTION      0x0020
#define ANTHOLOGY       0x0040
#define SERIES          0x0080
#define SHORT_FICTION   0x0100
#define ALL             (SHORT_STORY|NOVELETTE|NOVELLA|NOVEL)

#define LOCUS_POLL	0x0001	/* [l] */
#define PKDICK		0x0002	/* [p] */
#define CLARKE		0x0004	/* [c] */
#define WFAN		0x0008	/* [w] */
#define HUGO            0x0010  /* [h] */
#define NEBULA          0x0020  /* [n] */

#define LOCUSWIN	0x0100	/* [L] */
#define PKDWIN		0x0200	/* [P] */
#define CLARKEWIN	0x0400	/* [P] */
#define WFANWIN		0x0800	/* [C] */
#define HUGOWIN         0x1000  /* [H] */
#define NEBULAWIN       0x2000  /* [N] */
#define UNKNOWN         0x8000


#define LONG_FICTION	1
#define SEARCHALL	3

/*
 * Original Format was 30
 */
#define MAX_TITLE_WIDTH 32

extern char * optarg;

typedef struct record {
	char	rec_author[256];
	char	rec_title[256];
	int	rec_year;
	int	rec_award;
	int	rec_winner;
	int	rec_type;
	int	rec_noms;
	int	rec_wins;
	struct record *rec_next;
	struct record *rec_list;
} record_t;


struct record *head;
struct record *end;

static void
usage()
{
	printf("dbsearch [-c (l|s) ] (long/short fiction)\n");
	printf("         [-w (l|h|n|N|H)] (types)\n");
	exit(1);
}


static void
add_to_list(rec)
	struct record *rec;
{
	if (head == NULL) {
		head = end = rec;
		rec->rec_next = NULL;
	} else {
		rec->rec_next = NULL;
		end->rec_next = rec;
		end = rec;
	}

}


static void
print_record(rec)
	struct record *rec;
{
	int length;

#ifdef REMOVE
	if ( something && !rec->rec_winner)
		return;
#endif

	length = 0;
	if (rec->rec_award == 0) {
	printf("         "); 
	} else {
		printf("       ["); 
		if (rec->rec_award & NEBULA) {
			printf("n");
			length++;
		}
        	if (rec->rec_award & NEBULAWIN) {
			printf("N");
			length++;
		}
        	if (rec->rec_award & LOCUS_POLL) {
			printf("l");
			length++;
		}
        	if (rec->rec_award & CLARKE) {
			printf("c");
			length++;
		}
        	if (rec->rec_award & WFAN) {
			printf("w");
			length++;
		}
        	if (rec->rec_award & PKDICK) {
			printf("p");
			length++;
		}
        	if (rec->rec_award & HUGO) {
			printf("h");
			length++;
		}
        	if (rec->rec_award & HUGOWIN) {
			printf("H");
			length++;
		}
        	if (rec->rec_award & LOCUSWIN) {
			printf("L");
			length++;
		}
        	if (rec->rec_award & PKDWIN) {
			printf("L");
			length++;
		}
        	if (rec->rec_award & CLARKEWIN) {
			printf("C");
			length++;
		}
        	if (rec->rec_award & WFANWIN) {
			printf("W");
			length++;
		}
		printf("]"); 
	}

	for(;length<5; length++)
		printf(" ");

	if (rec->rec_type == NOVEL) {
		printf("n  ");
	} else if (rec->rec_type == NOVELLA) {
		printf("nv ");
	} else if (rec->rec_type == NOVELETTE) {
		printf("nt ");
	} else if (rec->rec_type == SHORT_STORY) {
		printf("ss ");
	} else if (rec->rec_type == SHORT_FICTION) {
		printf("sf ");
	} else if (rec->rec_type == NONFICTION) {
		printf("nf ");
	} else if (rec->rec_type == COLLECTION) {
		printf("c  ");
	} else if (rec->rec_type == ANTHOLOGY) {
		printf("a  ");
	} else if (rec->rec_type == SERIES) {
		printf("se ");
	} else if (rec->rec_type == UNKNOWN) {
		printf("u  ");
	}

	printf("%s  ", rec->rec_title);
	length = strlen(rec->rec_title);
	if (length <= MAX_TITLE_WIDTH) {
		for(length = MAX_TITLE_WIDTH-length; length; length--)
			 printf(" ");
	} else {
		printf(" --\n");
		for(length = MAX_TITLE_WIDTH+19; length; length--)
			 printf(" ");
	}

	if (rec->rec_winner) {
		printf("* %s\n", rec->rec_author);
	} else {
		printf("  %s\n", rec->rec_author);
	}
}

static void
print_first_record(rec)
	struct record *rec;
{
	int length;

	printf("\n");
	print_record(rec);
#ifdef REMOVE
	if (rec->rec_winner)
		printf("* ");
	else
		printf("  ");

	printf("%s  ", rec->rec_title);
	length = strlen(rec->rec_title);
	if (length < MAX_TITLE_WIDTH) {
		for(length = MAX_TITLE_WIDTH-length; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	} else {
		printf(" --\n");
		for(length = MAX_TITLE_WIDTH+10; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	}
#endif
}



struct record *
parse_record(fp)
	FILE *fp;
{
	int in;
	char *tmp;
	char buffer[32];
	struct record *rec;

	rec = (struct record *)malloc(sizeof(struct record));
	if (rec == NULL) {
		printf("Can't malloc enough memory\n");
		exit(1);
	}

	/*
	 * Parse Title
	 */
	tmp = rec->rec_title;
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
	tmp = rec->rec_author;
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
		rec->rec_type = NOVEL;
	} else if (strcmp(buffer, "nv") == 0) {
		rec->rec_type = NOVELLA;
	} else if (strcmp(buffer, "nt") == 0) {
		rec->rec_type = NOVELETTE;
	} else if (strcmp(buffer, "ss") == 0) {
		rec->rec_type = SHORT_STORY;
	} else if (strcmp(buffer, "sf") == 0) {
		rec->rec_type = SHORT_FICTION;
	} else if (strcmp(buffer, "nf") == 0) {
		rec->rec_type = NONFICTION;
	} else if (strcmp(buffer, "c") == 0) {
		rec->rec_type = COLLECTION;
	} else if (strcmp(buffer, "a") == 0) {
		rec->rec_type = ANTHOLOGY;
	} else if (strcmp(buffer, "se") == 0) {
		rec->rec_type = SERIES;
	} else if (strcmp(buffer, "u") == 0) {
		rec->rec_type = UNKNOWN;
	} else {
		printf("Bogus Story Type = (%s)\n", buffer);
		return(0);
	}

	/*
	 * Parse Year
	 */
	tmp = buffer;
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
	sscanf(buffer,"%d", &(rec->rec_year));

	/*
	 * Parse Type
	 */
	tmp = buffer;
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

        rec->rec_award = 0;
        rec->rec_winner = 0;

        if (strchr(buffer, 'n'))
                rec->rec_award |= NEBULA;
        if (strchr(buffer, 'N')) {
                rec->rec_award |= NEBULAWIN;
                rec->rec_winner = 1;
	}

        if (strchr(buffer, 'h'))
                rec->rec_award |= HUGO;
        if (strchr(buffer, 'H')) {
                rec->rec_award |= HUGOWIN;
                rec->rec_winner = 1;
	}

        if (strchr(buffer, 'c'))
                rec->rec_award |= CLARKE;
        if (strchr(buffer, 'C')) {
                rec->rec_award |= CLARKEWIN;
                rec->rec_winner = 1;
	}
        if (strchr(buffer, 'w'))
                rec->rec_award |= WFAN;
        if (strchr(buffer, 'W')) {
                rec->rec_award |= WFANWIN;
                rec->rec_winner = 1;
	}

        if (strchr(buffer, 'p'))
                rec->rec_award |= PKDICK;
        if (strchr(buffer, 'P')) {
                rec->rec_award |= PKDWIN;
                rec->rec_winner = 1;
	}
        if (strchr(buffer, 'l'))
                rec->rec_award |= LOCUS_POLL;
        if (strchr(buffer, 'L')) {
                rec->rec_award |= LOCUSWIN;
                rec->rec_winner = 1;
	}

	rec->rec_next = NULL;
	rec->rec_list = NULL;
	rec->rec_noms = 1;
	if (rec->rec_winner)
		rec->rec_wins = 1;
	else
		rec->rec_wins = 0;
	return(rec);
}


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int option;
	char title[256] = {0};
	char author[256] = {0};
	int year         = 1901;
	int hugos_only   = 0;
	int nebulas_only = 0;
	int winners_only = 0;
	int categories   = 0;
	int search_style = SEARCHALL;
	FILE *fp;
	struct record *rec;

	while ((option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {

		case '?':	usage();
				break;

		case 'c':	if (strcmp(optarg, "l") == 0) {
					search_style = LONG_FICTION;
				} else if (strcmp(optarg, "s") == 0) {
					search_style = SHORT_FICTION;
				}
				break;

		case 'w':	printf("optarg=%s\n", optarg);
				if (strcmp(optarg, "l") == 0) {
					search_style = LOCUS_POLL;
				} else if (strcmp(optarg, "c") == 0) {
					search_style = CLARKE;
				} else if (strcmp(optarg, "w") == 0) {
					search_style = WFAN;
				} else if (strcmp(optarg, "p") == 0) {
					search_style = PKDICK;
				} else if (strcmp(optarg, "h") == 0) {
					search_style = HUGO;
				} else if (strcmp(optarg, "n") == 0) {
					search_style = NEBULA;
				} else if (strcmp(optarg, "N") == 0) {
					search_style = NEBULAWIN;
				} else if (strcmp(optarg, "H") == 0) {
					search_style = HUGOWIN;
				} else if (strcmp(optarg, "L") == 0) {
					search_style = LOCUSWIN;
				} else if (strcmp(optarg, "P") == 0) {
					search_style = PKDWIN;
				} else if (strcmp(optarg, "C") == 0) {
					search_style = CLARKEWIN;
				} else if (strcmp(optarg, "W") == 0) {
					search_style = WFANWIN;
				}
				break;
		}
	}


	printf("This listing was machine generated from a common database. Please report\n");
	printf("any inaccuracies or errors to avonruff@REDACTED\n\n");

	fp = fopen(DBASE, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}


	printf("search_style = %d\n", search_style);
	while((rec = parse_record(fp))) {
		if (search_style == LONG_FICTION) {
			if (rec->rec_type != NOVEL)
				continue;
		} else if (search_style == SHORT_FICTION) {
			if (rec->rec_type == NOVEL)
				continue;
		} else if (search_style != SEARCHALL) {
			if (rec->rec_award != search_style)
				continue;
		}
		add_to_list(rec);
	}


	year = 1900;
	for(; year<1995; year++) {
		int category_hits;
		int winners;
		int first_of_year;
		struct record *tmp;
		int loop;

		first_of_year = 1;
		for (loop=0; loop<=4; loop++) {

			if (!loop && (search_style == SHORT_FICTION))
				continue;
			if (loop && (search_style == LONG_FICTION))
				continue;

			switch(loop) {
			case 0: categories = NOVEL;
				break;
			case 1: categories = NOVELLA;
				break;
			case 2: categories = NOVELETTE;
				break;
			case 3: categories = SHORT_STORY;
				break;
			case 4: categories = SHORT_FICTION;
				break;
			}

			category_hits = 0;
			winners = 0;
			tmp = head;

			while(tmp) {
				if (year && (year != tmp->rec_year)) {
					tmp = tmp->rec_next;
					continue;
				}
				if ((tmp->rec_type & categories) == 0) {
					tmp = tmp->rec_next;
					continue;
				}
				if (first_of_year) {
					first_of_year = 0;
					switch(loop) {
					case 0:	if (search_style == SEARCHALL)
							printf("\n%d  <Novel>\n", year);
						else
							printf("\n%d  ", year);
						break;
					case 1:	printf("\n%d  <Novella>\n", year);
						break;
					case 2:	printf("\n%d  <Novelette>\n", year);
						break;
					case 3:	printf("\n%d  <Short Story>\n", year);
						break;
					case 4:	printf("\n%d  <Short Fiction>\n", year);
						break;
					}
					if (search_style == LONG_FICTION)
						print_first_record(tmp);
					else
						print_record(tmp);
				} else if (category_hits == 0) {
					switch(loop) {
					case 0:	if (search_style == SEARCHALL)
							printf("      <Novel>\n");
						break;
					case 1:	printf("      <Novella>\n", year);
						break;
					case 2:	printf("      <Novelette>\n");
						break;
					case 3:	printf("      <Short Story>\n");
						break;
					case 4:	printf("      <Short Fiction>\n");
						break;
					}
					print_record(tmp);
				} else {
					print_record(tmp);
				}
				category_hits++;
				if (tmp->rec_winner)
					winners++;
				tmp = tmp->rec_next;
			}
		}
	}

	exit(0);
}
