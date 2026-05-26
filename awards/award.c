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

#define DBASE "dbases/awards.dbase"
#define OPTARGS "hnw?y:t:a:c:g:"

#define SHORT_STORY	0x01
#define NOVELETTE	0x02
#define NOVELLA		0x04
#define NOVEL		0x10
#define NEBULA		0x20
#define HUGO		0x40
#define SHORT_FICTION	(SHORT_STORY|NOVELETTE|NOVELLA)
#define ALL		(SHORT_STORY|NOVELETTE|NOVELLA|NOVEL)
#define MAX_TITLE_WIDTH	35
#define MAX_AUTHOR_WIDTH 20
#define CURRENT_YEAR    1995

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
	struct record *rec_prev;
} record_t;

struct record *head;
struct record *end;


/*
 * Add to list, maintaining a year, category, and title sort
 */
static void
add_to_list(rec)
	struct record *rec;
{
	struct record *tmp;

	if (head == NULL) {
		head = end = rec;
		rec->rec_next = NULL;
		rec->rec_prev = NULL;
	} else {

		/*
		 * Start at beginning of list
		 */
		tmp = head;

		/*
		 * Sort by year
		 */
		while (tmp && (rec->rec_year > tmp->rec_year))
			tmp = tmp->rec_next;

		/*
		 * Sort by category
		 */
		while (tmp && (rec->rec_year == tmp->rec_year) && (rec->rec_type < tmp->rec_type))
			tmp = tmp->rec_next;

		if (tmp) {
			rec->rec_next = tmp;
			rec->rec_prev = tmp->rec_prev;
			tmp->rec_prev = rec;
			if (rec->rec_prev) {
				rec->rec_prev->rec_next = rec;
			} else {
				head = rec;
			}
		} else {
			end->rec_next = rec;
			rec->rec_next = NULL;
			rec->rec_prev = end;
			end = rec;
		}

	}
}


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
	(void)printf("      [-g] (1|2|3|4)\n");
	(void)printf("            1 = Hugo Novel Nominees\n");
	(void)printf("            2 = Hugo Short Fiction Nominees\n");
	(void)printf("            3 = Nebula Novel Nominees\n");
	(void)printf("            4 = Nebula Short Fiction Nominees\n");
	exit(1);
}



static void
print_record(rec)
	struct record *rec;
{
	int length;

	if (rec->rec_award & NEBULA) {
		if (rec->rec_winner)
			printf("[N]");
		else
			printf("[n]");
        } else if (rec->rec_award & HUGO) {
		if (rec->rec_winner)
			printf("[H]");
		else
			printf("[h]");
	}

	printf("   ");
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
	}

	printf("%4d ", rec->rec_year);
	printf("%s  ", rec->rec_title);
	length = strlen(rec->rec_title);
	if (length <= MAX_TITLE_WIDTH) {
		for(length = MAX_TITLE_WIDTH-length; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	} else {
		printf(" --\n");
		for(length = MAX_TITLE_WIDTH+16; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	}
}

static void
print_records()
{
	struct record *tmp;

	tmp = head;
	while(tmp) {
		print_record(tmp);
		tmp = tmp->rec_next;
	}
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
	tmp = rec->rec_author;
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
	if (strcmp(buffer, "Novel") == 0) {
		rec->rec_type = NOVEL;
	} else if (strcmp(buffer, "Novella") == 0) {
		rec->rec_type = NOVELLA;
	} else if (strcmp(buffer, "Novelette") == 0) {
		rec->rec_type = NOVELETTE;
	} else if (strcmp(buffer, "Short Story") == 0) {
		rec->rec_type = SHORT_STORY;
	} else if (strcmp(buffer, "Short Fiction") == 0) {
		rec->rec_type = SHORT_FICTION;
	} else {
		(void)printf("Bogus Story Type = (%s)\n", buffer);
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
	(void)sscanf(buffer,"%d", &(rec->rec_year));

	/*
	 * Parse Winner
	 */
	in = getc(fp);
	if (in == 'W') {
		rec->rec_winner = 1;
		in = getc(fp);
	} else {
		rec->rec_winner = 0;
	}

	/*
	 * Parse Type
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

	if (strcmp(buffer, "NEBULA") == 0) {
		rec->rec_award = NEBULA;
	} else if (strcmp(buffer, "HUGO") == 0) {
		rec->rec_award = HUGO;
	} else {
		(void)printf("Bogus Award Type = (%s)\n", buffer);
		return(0);
	}
	return(rec);
}

static void
print_first_genrecord(rec)
        struct record *rec;
{
        int length;

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
}



static void
print_genrecord(rec)
        struct record *rec;
{
        int length;

        if (rec->rec_winner)
                printf("      * ");
        else
                printf("        ");

        printf("%s  ", rec->rec_title);
        length = strlen(rec->rec_title);
        if (length <= MAX_TITLE_WIDTH) {
                for(length = MAX_TITLE_WIDTH-length; length; length--)
                         printf(" ");
                printf("%s\n", rec->rec_author);
        } else {
                printf(" --\n");
                for(length = MAX_TITLE_WIDTH+10; length; length--)
                         printf(" ");
                printf("%s\n", rec->rec_author);
        }
}


static void
genlist1(hugo)
	int hugo;
{
	int year;
	int lastyear;
	int first_of_year = 1;
	int category_hits;
	int winners;
	struct record *tmp;

	if (hugo) {
		year = 1953;
		lastyear = CURRENT_YEAR;
		printf("                         [Hugo Novel Nominees List]\n\n");
	} else {
		year = 1965;
		lastyear = CURRENT_YEAR-1;
		printf("                        [Nebula Novel Nominees List]\n\n");
	}

	for( ; year<=lastyear; year++) {

		tmp		= head;
		category_hits	= 0;
		winners		= 0;
		first_of_year	= 1;

		while(tmp) {
			if (year && (year != tmp->rec_year)) {
				tmp = tmp->rec_next;
				continue;
			}
			if (first_of_year) {
				first_of_year = 0;
				printf("\n%d  ", year);
				print_first_genrecord(tmp);
			} else {
				print_genrecord(tmp);
			}
			category_hits++;
			if (tmp->rec_winner)
				winners++;
			tmp = tmp->rec_next;
		}
		if (winners == 0 && category_hits)
			printf("      * [[No Award]]\n");
		if (first_of_year) {
			if (year != CURRENT_YEAR)
				printf("\n%d    [none give]\n", year);
		}
	}
}

static void
genlist2(hugo)
	int hugo;
{
	int year, loop, categories, lastyear;
	int first_of_year = 1;
	int category_hits;
	int winners;
	struct record *tmp;

	if (hugo) {
		year = 1953;
		lastyear = CURRENT_YEAR;
		printf("                     [Hugo Short Fiction Nominees List]\n");
	} else {
		year = 1965;
		lastyear = CURRENT_YEAR - 1;
		printf("                    [Nebula Short Fiction Nominees List]\n");
	}

	for( ; year<=lastyear; year++) {

		first_of_year	= 1;
		for (loop=0; loop<4; loop++) {

			switch(loop) {
			case 0: categories = NOVELLA;
				break;
			case 1: categories = NOVELETTE;
				break;
			case 2: categories = SHORT_STORY;
				break;
			case 3: categories = SHORT_FICTION;
				break;
			}

			category_hits	= 0;
			winners		= 0;
			tmp		= head;

			while(tmp) {
				if (year && (year != tmp->rec_year)) {
					tmp = tmp->rec_next;
					continue;
				}
				if (tmp->rec_type != categories) {
					tmp = tmp->rec_next;
					continue;
				}
				if (first_of_year) {
					first_of_year = 0;
					switch(loop) {
					case 0:	printf("\n%d  [Novella]\n", year);
						break;
					case 1:	printf("\n%d  [Novelette]\n", year);
						break;
					case 2:	printf("\n%d  [Short Story]\n", year);
						break;
					case 3:	printf("\n%d  [Short Fiction]\n", year);
						break;
					}
				} else if (category_hits == 0) {
					switch(loop) {
					case 0:	printf("      [Novella]\n");
						break;
					case 1:	printf("      [Novelette]\n");
						break;
					case 2:	printf("      [Short Story]\n");
						break;
					case 3:	printf("      [Short Fiction]\n");
						break;
					}
				}
				print_genrecord(tmp);
				category_hits++;
				if (tmp->rec_winner)
					winners++;
				tmp = tmp->rec_next;
			}
			if (winners == 0 && category_hits)
				printf("      * [[No Award]]\n");
		}
		if (first_of_year) {
			printf("\n%d  [none give]\n", year);
		}
	}
}

static void
genlist3(hugo)
	int hugo;
{
	int year, loop, categories, lastyear;
	int first_of_year = 1;
	int category_hits;
	int winners;
	struct record *tmp;

	if (hugo) {
		year = 1953;
		lastyear = CURRENT_YEAR;
		printf("                     [Hugo Fiction Nominees List]\n");
	} else {
		year = 1965;
		lastyear = CURRENT_YEAR - 1;
		printf("                    [Nebula Fiction Nominees List]\n");
	}

	for( ; year<=lastyear; year++) {

		first_of_year	= 1;
		for (loop=0; loop<5; loop++) {

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

			category_hits	= 0;
			winners		= 0;
			tmp		= head;

			while(tmp) {
				if (year && (year != tmp->rec_year)) {
					tmp = tmp->rec_next;
					continue;
				}
				if (tmp->rec_type != categories) {
					tmp = tmp->rec_next;
					continue;
				}
				if (first_of_year) {
					first_of_year = 0;
					switch(loop) {
					case 0:	printf("\n%d  [Novel]\n", year);
						break;
					case 1:	printf("\n%d  [Novella]\n", year);
						break;
					case 2:	printf("\n%d  [Novelette]\n", year);
						break;
					case 3:	printf("\n%d  [Short Story]\n", year);
						break;
					case 4:	printf("\n%d  [Short Fiction]\n", year);
						break;
					}
				} else if (category_hits == 0) {
					switch(loop) {
					case 0:	printf("      [Novel]\n");
						break;
					case 1:	printf("      [Novella]\n");
						break;
					case 2:	printf("      [Novelette]\n");
						break;
					case 3:	printf("      [Short Story]\n");
						break;
					case 4:	printf("      [Short Fiction]\n");
						break;
					}
				}
				print_genrecord(tmp);
				category_hits++;
				if (tmp->rec_winner)
					winners++;
				tmp = tmp->rec_next;
			}
			if (winners == 0 && category_hits)
				printf("      * [[No Award]]\n");
		}
		if (first_of_year) {
			printf("\n%d  [none give]\n", year);
		}
	}
}

static void
genlist4()
{
        int loop;
        int match;
        int length;
        int category   = 0;
        struct record *tmp1;
        struct record *tmp2;

        printf("                         [Multiple Winner List]\n");
        for (loop=0; loop<4; loop++) {
                switch(loop) {
                case 0: category = NOVEL;
                        printf("[Novel]\n");
                        break;
                case 1: category = NOVELLA;
                        printf("\n[Novella]\n");
                        break;
                case 2: category = NOVELETTE;
                        printf("\n[Novelette]\n");
                        break;
                case 3: category = SHORT_STORY;
                        printf("\n[Short Story]\n");
                        break;
                }
                tmp1 = head;
                while(tmp1) {

                        if (tmp1->rec_type != category) {
                                tmp1 = tmp1->rec_next;
                                continue;
                        }

                        /*
                         * Found a category - now look for dup.
                         */
                        tmp2 = tmp1->rec_next;
                        match = 0;
                        while(tmp2) {
                                if ( strcmp(tmp2->rec_title, tmp1->rec_title) == 0) {
                                        match = 1;
                                        break;
                                }
                                tmp2 = tmp2->rec_next;
                        }
                        if (match) {
				printf("  %d - ", tmp1->rec_year);
                                printf("%s  ", tmp1->rec_author);
                                length = strlen(tmp1->rec_author);
                                if (length <= MAX_AUTHOR_WIDTH) {
                                        for(length = MAX_AUTHOR_WIDTH-length; length; length--)
                                                printf(" ");
                                } else {
                                        printf("\n");
                                        for(length = MAX_AUTHOR_WIDTH+11; length; length--)
                                                printf(" ");
                                }
                                if (tmp1->rec_type == NOVEL)
                                        printf("_%s_\n", tmp1->rec_title);
                                else
                                        printf("\"%s\"\n", tmp1->rec_title);
                        }
                        tmp1 = tmp1->rec_next;
                }
        }
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
	int options	 = 0;
	int any_found    = 0;
	int genlist      = 0;
	FILE *fp;
	struct record *rec;

	while ((option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {

		case '?':	usage();
				break;

		case 'a': 	(void)strcpy(author, optarg);
				options++;
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
				options++;
				break;

		case 'g':	if (strcmp(optarg, "1") == 0) {
					hugos_only = 1;
					categories = NOVEL;
					genlist = 1;
				} else if (strcmp(optarg, "2") == 0) {
					hugos_only = 1;
					categories = SHORT_FICTION;
					genlist = 2;
				} else if (strcmp(optarg, "3") == 0) {
					nebulas_only = 1;
					categories = NOVEL;
					genlist = 3;
				} else if (strcmp(optarg, "4") == 0) {
					nebulas_only = 1;
					categories = SHORT_FICTION;
					genlist = 4;
				} else if (strcmp(optarg, "5") == 0) {
					hugos_only = 1;
					categories = ALL;
					genlist = 5;
				} else if (strcmp(optarg, "6") == 0) {
					nebulas_only = 1;
					categories = ALL;
					genlist = 6;
				} else if (strcmp(optarg, "7") == 0) {
					categories = ALL;
					winners_only = 1;
					genlist = 7;
				}
				options++;
				break;

		case 'h': 	if (nebulas_only) {
					(void)printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					hugos_only = 1;
				}
				options++;
				break;

		case 'n': 	if (hugos_only) {
					(void)printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					nebulas_only = 1;
				}
				options++;
				break;

		case 't': 	(void)strcpy(title, optarg);
				options++;
				break;

		case 'w':	winners_only = 1;
				options++;
				break;

		case 'y': 	(void)sscanf(optarg,"%d", &year);
				options++;
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

	while((rec = parse_record(fp))) {

		if (year && (year != rec->rec_year))
			continue;

		if (winners_only && !rec->rec_winner)
			continue;

		if (hugos_only && (rec->rec_award != HUGO))
			continue;

		if (nebulas_only && (rec->rec_award != NEBULA))
			continue;

		if ((rec->rec_type & categories) == 0)
			continue;

		if (title[0] && !(strstr(rec->rec_title, title))) 
			continue;
			
		if (author[0] && !(strstr(rec->rec_author, author))) 
			continue;
			
		any_found++;
		add_to_list(rec);
	}

	switch(genlist) {
	case 0:	print_records();
		break;
	case 1: genlist1(1);
		break;
	case 2: genlist2(1);
		break;
	case 3: genlist1(0);
		break;
	case 4: genlist2(0);
		break;
	case 5: genlist3(1);
		break;
	case 6: genlist3(0);
		break;
	case 7: genlist4();
		break;
	}

	if (!any_found) {
		printf("No matches found\n\n");
	} else {
		printf("\n\n---------------------------------------------------------------------\n");
		printf("This listing was machine generated from a common database. Please\n");
		printf("report any inaccuracies or errors to avonruff@REDACTED\n");
	}

	exit(0);
	return(0);
}
