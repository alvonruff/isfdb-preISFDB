#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DBASE "dbases/awards.dbase"
#define AGES  "dbases/ages.dbase"
#define OPTARGS "bhn?:c:"
#define CURRENT_YEAR 1994

#define SHORT_STORY     0x001
#define NOVELETTE       0x002
#define NOVELLA         0x004
#define NOVEL           0x010
#define NEBULA          0x020
#define HUGO            0x040
#define SHORT_FICTION   0x080
#define DOUBLE_WINNERS	0x100
#define ALL             (SHORT_STORY|NOVELETTE|NOVELLA|NOVEL)

#define LONG_FICTION	1
#define SEARCHALL	3

/*
 * Original Format was 30
 */
#define MAX_TITLE_WIDTH 32
#define MAX_AUTHOR_WIDTH 20

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
} record_t;


struct record *head;
struct record *end;

static void
usage()
{
	printf("genlist  [-c] (l | s) (long/short fiction)\n");
	printf("         [-h] (Hugos only)\n");
	printf("         [-n] (Nebulas only)\n");
	printf("         [-b] (won Both)\n");
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
print_first_record(rec)
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
		printf("Bogus Award Type = (%s)\n", buffer);
		return(0);
	}
	rec->rec_next = NULL;
	rec->rec_noms = 1;
	if (rec->rec_winner)
		rec->rec_wins = 1;
	else
		rec->rec_wins = 0;
	return(rec);
}

static void
do_double_winners()
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
			printf("<Novel>\n");
			break;
		case 1: category = NOVELLA;
			printf("\n<Novella>\n");
			break;
		case 2: category = NOVELETTE;
			printf("\n<Novelette>\n");
			break;
		case 3: category = SHORT_STORY;
			printf("\n<Short Story>\n");
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
				printf("  %s  ", tmp1->rec_author);
				length = strlen(tmp1->rec_author);
				if (length <= MAX_AUTHOR_WIDTH) {
					for(length = MAX_AUTHOR_WIDTH-length; length; length--)
			 			printf(" ");
				} else {
					printf("\n");
					for(length = MAX_AUTHOR_WIDTH+4; length; length--)
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
	int search_style = SEARCHALL;
	int options	 = 0;
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
				options++;
				break;

		case 'h': 	if (nebulas_only) {
					printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					hugos_only = 1;
				}
				options++;
				break;

		case 'n': 	if (hugos_only) {
					printf("Can't select both Hugos only AND Nebulas only\n");
					exit(1);
				} else {
					nebulas_only = 1;
				}
				options++;
				break;

		case 'b':	search_style = DOUBLE_WINNERS;
				options++;
				break;
		}
	}

	if (options == 0)
		usage();


	printf("[This file is from the Sf-Lovers Archives at Rutgers University.  It is\n");
	printf("provided as part of a free service in connection with distribution of\n");
	printf("Sf-Lovers Digest.  This file is currently maintained by the moderator of\n");
	printf("the Digest.  It may be freely copied or redistributed in whole or in part\n");
	printf("as long as this notice and any copyright notices or other identifying\n");
	printf("headers or trailers remain intact.  If you would like to know more about\n");
	printf("Sf-Lovers Digest, send mail to SF-LOVERS-REQUEST@RUTGERS.EDU.]\n\n");
	printf("This listing was machine generated from a common database. Please report\n");
	printf("any inaccuracies or errors to avonruff@REDACTED\n\n");

	if (hugos_only) {
		if (search_style == LONG_FICTION)
			printf("                         [Hugo Novel Nominees List]\n");
		else if (search_style == SHORT_FICTION)
			printf("                     [Hugo Short Fiction Nominees List]\n");
	} else if (nebulas_only) {
		if (search_style == LONG_FICTION)
			printf("                         [Nebula Novel Nominees List]\n");
		else if (search_style == SHORT_FICTION)
			printf("                     [Nebula Short Fiction Nominees List]\n");
	}

	fp = fopen(DBASE, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}


	while((rec = parse_record(fp))) {
		if (hugos_only && (rec->rec_award != HUGO))
			continue;
		if (nebulas_only && (rec->rec_award != NEBULA))
			continue;
		if ((search_style == LONG_FICTION) && (rec->rec_type != NOVEL))
			continue;
		if ((search_style == SHORT_FICTION) && (rec->rec_type == NOVEL))
			continue;
		if ((search_style == DOUBLE_WINNERS) && !(rec->rec_winner))
			continue;
		add_to_list(rec);
	}

	if (search_style == DOUBLE_WINNERS) {
		do_double_winners();
		exit(0);
	}

	if (hugos_only) {
		year = 1953;
	} 
	if (nebulas_only) {
		year = 1965;
	} 

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
			if (winners == 0 && category_hits)
				printf("      * <<No Award>>\n");
		}
		if (first_of_year) {
			if (search_style == LONG_FICTION)
				printf("\n%d    [none give]\n", year);
			else
				printf("\n%d  [none give]\n", year);
		}
	}

	printf("\nEvelyn C. Leeper\n");
	printf("Email Redacted\n");
	printf("Email Redacted\n");
	printf("Phone Number Redacted\n");
	exit(0);
}
