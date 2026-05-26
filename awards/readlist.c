/*
 * #ident "%W%	%G% %Q%"
 *
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */


#include <sys/fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#define DBASE "dbases/readlist.dbase"
#define OPTARGS "bhlnoprw?y:t:a:c:g:T:"

#define MAX_TITLE_WIDTH	35
#define MAX_AUTHOR_WIDTH 20
#define CURRENT_YEAR    1994

#define INC_REC_POINTS(a) {		\
	switch((a)) {			\
	case 1: rec->rec_points += 90;	\
		break;			\
	case 2: rec->rec_points += 45;	\
		break;			\
	case 3: rec->rec_points += 43;	\
		break;			\
	case 4: rec->rec_points += 41;	\
		break;			\
	case 5: rec->rec_points += 39;	\
		break;			\
	default: rec->rec_points += (39-(a));	\
	}			\
}

#define BUMPLEN(a, b) {		\
	if ( (b) < 10) {	\
		(a) += 3;	\
	} else if ((b) < 100) {	\
		(a) += 4;	\
	} else {		\
		(a) += 5;	\
	}			\
}

typedef struct record {
	char	rec_author[256];
	char	rec_title[256];
	int	rec_year;
	int	rec_winner;
	int	rec_points;
	int rec_ownit;
	int rec_readit;

	int	rec_hugo;
	int	rec_nebula;
	int	rec_locus_sf;
	int	rec_locus_hor;
	int	rec_locus_fant;
	int	rec_campbell;
	int	rec_stoker;
	int	rec_clarke;
	int	rec_pkdick;
	int	rec_wfa;

	struct record *rec_next;
	struct record *rec_prev;
} record_t;

typedef struct award {
	int	aw_award;
} award_t;

struct record *head;
struct record *end;
char *optarg;

#ifdef SVR4
#define Getopt(a,b,c)	getopt((a), (b), (c))
#else
char
Getopt(argc, argv, optargs)
	int argc;
	char *argv[];
	char *optargs;
{
	static int curarg=1;
	char retval;
	char *tmp;

	if (curarg == argc) {
		return(-1);
	}

	while( argv[curarg][0] != '-') {
		curarg++;
		if (curarg == argc) {
			return(-1);
		}
	}

	retval = argv[curarg][1];
	tmp = strchr(optargs, retval);
	if (tmp) {
		if (tmp[1] == ':') {
			curarg++;
			optarg = argv[curarg];
		} else {
			optarg = NULL;
		}
		curarg++;
		return(retval);
	} else {
		curarg++;
		return(0);
	}
}
#endif

/*
 * Add to list, maintaining a year, category, and title sort
 */
void
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
		 * Sort by points
		 */
		while (tmp && (rec->rec_points < tmp->rec_points))
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
	(void)printf("      [-w] (winners)\n");
	exit(1);
}


void
print_entry(rec)
	struct record *rec;
{
	printf("%s|", rec->rec_title);
	printf("%s|", rec->rec_author);
	printf("%d|", rec->rec_year);

	if (rec->rec_campbell) {
		printf("[ca_%d_n_%d]", rec->rec_campbell, rec->rec_year);
	}
	if (rec->rec_clarke) {
		printf("[cl_%d_n_%d]", rec->rec_clarke, rec->rec_year);
	}
	if (rec->rec_hugo) {
		printf("[hu_%d_n_%d]", rec->rec_hugo, rec->rec_year);
	}
	if (rec->rec_locus_fant) {
		printf("[lf_%d_n_%d]", rec->rec_locus_fant, rec->rec_year);
	}
	if (rec->rec_locus_hor) {
		printf("[lh_%d_n_%d]", rec->rec_locus_hor, rec->rec_year);
	}
	if (rec->rec_locus_sf) {
		printf("[ls_%d_n_%d]", rec->rec_locus_sf, rec->rec_year);
	}
	if (rec->rec_nebula) {
		printf("[ne_%d_n_%d]", rec->rec_nebula, rec->rec_year);
	}
	if (rec->rec_pkdick) {
		printf("[pk_%d_n_%d]", rec->rec_pkdick, rec->rec_year);
	}
	if (rec->rec_stoker) {
		printf("[st_%d_n_%d]", rec->rec_stoker, rec->rec_year);
	}
	if (rec->rec_wfa) {
		printf("[wf_%d_n_%d]", rec->rec_wfa, rec->rec_year);
	}

	if (rec->rec_ownit) {
		printf("|y");
	} else {
		printf("|n");
	}
	if (rec->rec_readit) {
		printf("|y");
	} else {
		printf("|n");
	}
	printf("\n");
}

void
print_record(rec, dopoints, dolist)
	struct record *rec;
	int dopoints;
	int dolist;
{
	int length = 0;
	int loop;

	if (dopoints) {
		printf("%3d ", rec->rec_points);
	}

	if (dolist) {
		printf("[");
		if (rec->rec_campbell) {
			printf("ca%d", rec->rec_campbell);
			BUMPLEN(length, rec->rec_campbell);
		}
		if (rec->rec_clarke) {
			if (length) {
				printf(",");
				length++;
			}
			printf("cl%d", rec->rec_clarke);
			BUMPLEN(length, rec->rec_clarke);
		}
		if (rec->rec_hugo) {
			if (length) {
				printf(",");
				length++;
			}
			printf("hu%d", rec->rec_hugo);
			BUMPLEN(length, rec->rec_hugo);
		}
		if (rec->rec_locus_sf) {
			if (length) {
				printf(",");
				length++;
			}
			printf("ls%d", rec->rec_locus_sf);
			BUMPLEN(length, rec->rec_locus_sf);
		}
		if (rec->rec_locus_hor) {
			if (length) {
				printf(",");
				length++;
			}
			printf("lh%d", rec->rec_locus_hor);
			BUMPLEN(length, rec->rec_locus_hor);
		}
		if (rec->rec_locus_fant) {
			if (length) {
				printf(",");
				length++;
			}
			printf("lf%d", rec->rec_locus_fant);
			BUMPLEN(length, rec->rec_locus_fant);
		}
		if (rec->rec_nebula) {
			if (length) {
				printf(",");
				length++;
			}
			printf("ne%d", rec->rec_nebula);
			BUMPLEN(length, rec->rec_nebula);
		}
		if (rec->rec_pkdick) {
			if (length) {
				printf(",");
				length++;
			}
			printf("pk%d", rec->rec_pkdick);
			BUMPLEN(length, rec->rec_pkdick);
		}
		if (rec->rec_stoker) {
			if (length) {
				printf(",");
				length++;
			}
			printf("st%d", rec->rec_stoker);
			BUMPLEN(length, rec->rec_stoker);
		}
		if (rec->rec_wfa) {
			if (length) {
				printf(",");
				length++;
			}
			printf("wf%d", rec->rec_wfa);
			BUMPLEN(length, rec->rec_wfa);
		}
		printf("]");
		for(loop=length; loop<20; loop++) {
			printf(" ");
		}
	}


	printf("%4d ", rec->rec_year);
	printf("%s ", rec->rec_title);
	length = strlen(rec->rec_title);
	if (length <= MAX_TITLE_WIDTH) {
		for(length = MAX_TITLE_WIDTH-length; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	} else {
		printf(" --\n");
		for(length = MAX_TITLE_WIDTH+32; length; length--)
			 printf(" ");
		printf("%s\n", rec->rec_author);
	}
}

void
print_records(int dolist)
{
	struct record *tmp;

	tmp = head;
	while(tmp) {
		print_record(tmp, 0, dolist);
		tmp = tmp->rec_next;
	}
}

void
print_by_points(int dolist)
{
	struct record *tmp;

	tmp = head;
	while(tmp) {
		print_record(tmp, 1, dolist);
		tmp = tmp->rec_next;
	}
}

void
print_by_points_and_year(int dolist)
{
	struct record *tmp;
	int year;

	for (year=1953; year <= CURRENT_YEAR; year++) {
		tmp = head;
		printf("\n[%d]:\n", year);
		while(tmp) {
			if (tmp->rec_year == year) {
				print_record(tmp, 1, dolist);
			}
			tmp = tmp->rec_next;
		}
	}
}

struct record *
parse_record(fp)
	FILE *fp;
{
	int in;
	int level;
	int keep_going;
	char *tmp;
	char buffer[32];
	char buffer2[32];
	struct record *rec;

	rec = (struct record *)malloc((size_t)sizeof(struct record));
	if (rec == NULL) {
		printf("Can't malloc enough memory\n");
		exit(1);
	}
	rec->rec_points		= 0;
	rec->rec_hugo		= 0;
	rec->rec_nebula		= 0;
	rec->rec_locus_sf	= 0;
	rec->rec_locus_hor	= 0;
	rec->rec_locus_fant	= 0;
	rec->rec_campbell	= 0;
	rec->rec_stoker		= 0;
	rec->rec_clarke		= 0;
	rec->rec_pkdick		= 0;
	rec->rec_wfa		= 0;
	rec->rec_ownit      = 0;
	rec->rec_readit     = 0;

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

	keep_going = 1;
	while(keep_going) {

		/*
		 * Parse Type of Award
		 */
		tmp = buffer;
		/*CONSTCOND*/
		while(1) {
			in = getc(fp);
			if (in == -1) {
				return (0);
			} else if (in == '|') {
				goto skipitall;
			} else if (isdigit(in)) {
				break;
			} else {
				*tmp = (char)in;
				tmp++;
			}
		}
		*tmp = 0;

		/*
		 * Parse nomination level
		 */
		tmp = buffer2;
		*tmp = (char)in;
		tmp++;
		/*CONSTCOND*/
		while(1) {
			in = getc(fp);
			if (in == -1) {
				return (0);
			} else if (isalpha(in)) {
				ungetc(in, fp);
				break;
			} else if (in == '|') {
				keep_going = 0;
				break;
			} else {
				*tmp = (char)in;
				tmp++;
			}
		}
		*tmp = 0;

		sscanf(buffer2, "%d", &level );
		if (level == 1)
			rec->rec_winner++;

		if (strcmp(buffer, "hu") == 0) {
			rec->rec_hugo = level;
			INC_REC_POINTS(rec->rec_hugo);
		}
		if (strcmp(buffer, "ne") == 0) {
			rec->rec_nebula = level;
			INC_REC_POINTS(rec->rec_nebula);
		} 
		if (strcmp(buffer, "ls") == 0) {
			rec->rec_locus_sf = level;
			INC_REC_POINTS(rec->rec_locus_sf);
		}
		if (strcmp(buffer, "lh") == 0) {
			rec->rec_locus_hor = level;
			INC_REC_POINTS(rec->rec_locus_hor);
		} 
		if (strcmp(buffer, "lf") == 0) {
			rec->rec_locus_fant = level;
			sscanf(buffer2, "%d", &(rec->rec_locus_fant) );
			INC_REC_POINTS(rec->rec_locus_fant);
		} 
		if (strcmp(buffer, "ca") == 0) {
			rec->rec_campbell = level;
			INC_REC_POINTS(rec->rec_campbell);
		} 
		if (strcmp(buffer, "st") == 0) {
			rec->rec_stoker = level;
			INC_REC_POINTS(rec->rec_stoker);
		} 
		if (strcmp(buffer, "cl") == 0) {
			rec->rec_clarke = level;
			INC_REC_POINTS(rec->rec_clarke);
		} 
		if (strcmp(buffer, "pk") == 0) {
			rec->rec_pkdick = level;
			INC_REC_POINTS(rec->rec_pkdick);
		}
		if (strcmp(buffer, "wf") == 0) {
			rec->rec_wfa = level;
			INC_REC_POINTS(rec->rec_wfa);
		}
	}
skipitall:
	in = getc(fp);
	if (in == 'y') {
		rec->rec_ownit = 1;
	}
	in = getc(fp);
	in = getc(fp);
	if (in == 'y') {
		rec->rec_readit = 1;
	}
	in = getc(fp);
end:
	return(rec);
}


void
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


void

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


void
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

#ifdef REMOVE
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

                        /*
                         * Found a category - now look for dup.
                         */
                        tmp2 = tmp1->rec_next;
                        match = 0;
                        while(tmp2) {
                                if ( strcmp(tmp2->rec_title, tmp1->rec_title)
== 0) {
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
                                        for(length = MAX_AUTHOR_WIDTH-length;
length; length--)
                                                printf(" ");
                                } else {
                                        printf("\n");
                                        for(length = MAX_AUTHOR_WIDTH+11;
length; length--)
                                                printf(" ");
                                }
                        }
                        tmp1 = tmp1->rec_next;
                }
        }
}
#endif



int
main(argc, argv)
	int argc;
	char *argv[];
{
	int option;
	char title[256] = {0};
	char author[256] = {0};
	int year         = 0;
	int winners_only = 0;
	int options	 = 0;
	int any_found    = 0;
	int print_raw	 = 0;
	int throttle     = 0;
	int dolist       = 1;
	int print_only   = 0;
	int buy_list     = 0;
	int own_list     = 0;
	int read_list    = 0;
	FILE *fp;
	struct record *rec;

	while ((option = Getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {

		case '?':	usage();
				break;

		case 'a': 	(void)strcpy(author, optarg);
				options++;
				print_raw = 1;
				break;

		case 'b':	buy_list = 1;
				print_raw = 1;
				options++;
				break;

		case 'o':	own_list = 1;
				print_raw = 1;
				options++;
				break;

		case 'r':	read_list = 1;
				print_raw = 1;
				options++;
				break;

		case 'l':	print_raw = 1;
				options++;
				break;

		case 'p':	print_only = 1;
				options++;
				break;

		case 't': 	(void)strcpy(title, optarg);
				options++;
				print_raw = 1;
				break;

		case 'T':	(void)sscanf(optarg,"%d", &throttle);
				options++;
				break;

		case 'w':	winners_only = 1;
				options++;
				break;

		case 'y': 	(void)sscanf(optarg,"%d", &year);
				options++;
				print_raw = 1;
				break;

		}
	}

	fp = fopen(DBASE, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}

	while((rec = parse_record(fp)) != NULL) {

		if (year && (year != rec->rec_year))
			continue;

		if (winners_only && !rec->rec_winner)
			continue;

		if (title[0] && !(strstr(rec->rec_title, title))) 
			continue;
			
		if (author[0] && !(strstr(rec->rec_author, author))) 
			continue;
			
		if (throttle && (rec->rec_points < throttle))
			continue;

		if (buy_list && (rec->rec_ownit))
			continue;

		if (own_list && !(rec->rec_ownit))
			continue;

		if (read_list && !rec->rec_ownit)
			continue;
		if (read_list && rec->rec_readit)
			continue;

		if (print_only) {
			print_entry(rec);
		} else {
			any_found++;
			add_to_list(rec);
		}
	}

	if (print_only) {
		exit(0);
	} else if (!any_found) {
		printf("No matches found\n\n");
	} else {
		if (print_raw) {
			print_by_points(0);
		} else {
			print_by_points_and_year(dolist);
		}

		printf("\n\n---------------------------------------------------------------------\n");
		printf("This listing was machine generated from a common database. Please\n");
		printf("report any inaccuracies or errors to avonruff@REDACTED\n");
	}

	exit(0);
	return(0);
}
