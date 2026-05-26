#ident "%W%	%G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DBASE "dbases/nesfa.dbase"
#define OPTARGS "hn?:s:c:a:t:"
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

#define WEBDAM          0x0001  /* [a] */
#define ASFANTH         0x0002  /* [b] */
#define SFHALL          0x0004  /* [c] */
#define HEALY           0x0008  /* [d] */
#define GROFF           0x0010  /* [e] */
#define ASIMOV          0x0020  /* [f] */
#define HARTWELL        0x0040  /* [g] */
#define HUGO            0x0080  /* [h] */
#define NESFA           0x0100  /* [i] */
#define NEBULA          0x0200  /* [n] */
#define NEBULAWIN       0x0400  /* [N] ([N] = winner) */
#define HUGOWIN         0x0800  /* [H] ([H] = winner) */
#define UNKNOWN         0x8000

#define NOMINATIONS	1
#define WINS		2
#define SEARCHALL	3
#define HEAT		4
#define AUTH_ERRORS	5
#define TITLE_ERRORS	6

#define CURRENCY_WEIGHT	3.0
#define NOM_WEIGHT	1.5
#define WIN_WEIGHT	3.0

extern char * optarg;
float sensitivity;

typedef struct auth {
	char		au_author[256];
	int		au_noms;
	int		au_entries;
	int		au_wins;
	float		au_heat;
	struct auth	*au_next;
	struct auth	*au_prev;
	struct record	*au_list;
} auth_t;

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


struct auth *head;
struct auth *end;

static void
usage()
{
	printf("dbsearch [-s (n|w|h) (nominations|winners|heat)]\n");
	printf("         [-a] <sensitivity> (0.0 - 2.0)\n");
	printf("         [-b] (perform title sort)\n");
	printf("         [-t] <sensitivity> (0.0 - 2.0)\n");
	printf("         [-c] (n|nv|nt|ss|!n|!nv|!nt|!ss)\n");
	printf("         [-h] (Hugos only)\n");
	printf("         [-n] (Nebulas only)\n");
	exit(1);
}


float
similar(string1, string2)
	char *string1;
	char *string2;
{
	char *tmp1;
	char *tmp2;
	int  total = 0;
	int  length;
	float ratio;

	tmp1 = string1;
	tmp2 = string2;
	while((*tmp1) && (*tmp2)) {
		if (*tmp1 == *tmp2)
			total++;
		tmp1++;
		tmp2++;
	}

	/*
	 * Align the tmp pointers to the end of the strings
	 */
	tmp1 = string1;
	while(*tmp1)
		tmp1++;
	tmp1--;
	tmp2 = string2;
	while(*tmp2)
		tmp2++;
	tmp2--;

	while((*tmp1) && (*tmp2)) {
		if (*tmp1 == *tmp2)
			total++;
		tmp1--;
		tmp2--;
	}

	if (strlen(string1) > strlen(string2))
		length = strlen(string1);
	else
		length = strlen(string2);

	ratio = ((float)total)/length;
	return(ratio);
}

static void
find_author_errors()
{
	struct auth *rec1;
	struct auth *rec2;

	rec1 = head;
	while(rec1) {
		rec2 = rec1->au_next;
		while(rec2) {
			if (strcmp(rec1->au_author, rec2->au_author)) {
				if (similar(rec1->au_author, rec2->au_author) > sensitivity) {
					printf("%s\n", rec1->au_author);
					printf("%s\n\n", rec2->au_author);
				}
			}
			rec2 = rec2->au_next;
		}
		rec1 = rec1->au_next;
	}
}

static void
find_title_errors()
{
	struct auth *rec1;
	struct auth *rec2;
	struct record *tmp1;
	struct record *tmp2;

	rec1 = head;
	while(rec1) {
		tmp1 = rec1->au_list;
		while(tmp1) {
			rec2 = rec1;
			while(rec2) {
				tmp2 = rec2->au_list;
				while(tmp2) {
					if (strcmp(tmp1->rec_title, tmp2->rec_title)) {
						if (similar(tmp1->rec_title, tmp2->rec_title) > 0.8) {
							printf("%s\n", tmp1->rec_title);
							printf("%s\n\n", tmp2->rec_title);
						}
					}
					tmp2 = tmp2->rec_list;
				}
				rec2 = rec2->au_next;
			}
			tmp1 = tmp1->rec_list;
		}
		rec1 = rec1->au_next;
	}
}


static void
sort_by_auth()
{
	struct auth *curmax;
	struct auth *curposit;
	struct auth *nextsort;

	nextsort = head;
	while (nextsort) {

		/*
		 * Find curmax, the next target
		 */
		curposit = curmax = nextsort;
		while(curposit) {
			if ( strcmp(curposit->au_author, curmax->au_author) < 0)
				curmax = curposit;
			curposit = curposit->au_next;
		}

		if (curmax == nextsort) {
			nextsort = curmax = nextsort->au_next;
			continue;
		}

		/*
		 * Delete curmax from the list
		 */
		if (curmax->au_next == NULL) {
			curmax->au_prev->au_next = NULL;
		} else if (curmax->au_prev == NULL) {
			head = curmax->au_next;
		} else {
			curmax->au_prev->au_next = curmax->au_next;
			curmax->au_next->au_prev = curmax->au_prev;
		}
		curmax->au_prev = NULL;
		curmax->au_next = NULL;

		/*
		 * Insert at position nextsort
		 */
		if (nextsort->au_next == NULL) {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
			return;
		} else if (nextsort->au_prev == NULL) {
			curmax->au_next = head;
			curmax->au_prev = NULL;
			head = curmax;
			nextsort->au_prev = curmax;
		} else {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
		}

	}

}

static void
sort_by_nom()
{
	struct auth *curmax;
	struct auth *curposit;
	struct auth *nextsort;

	nextsort = head;
	while (nextsort) {

		/*
		 * Find curmax, the next target
		 */
		curposit = curmax = nextsort;
		while(curposit) {
			if ( curposit->au_noms > curmax->au_noms)
				curmax = curposit;
			curposit = curposit->au_next;
		}

		if (curmax == nextsort) {
			nextsort = curmax = nextsort->au_next;
			continue;
		}

		/*
		 * Delete curmax from the list
		 */
		if (curmax->au_next == NULL) {
			curmax->au_prev->au_next = NULL;
		} else if (curmax->au_prev == NULL) {
			head = curmax->au_next;
		} else {
			curmax->au_prev->au_next = curmax->au_next;
			curmax->au_next->au_prev = curmax->au_prev;
		}
		curmax->au_prev = NULL;
		curmax->au_next = NULL;

		/*
		 * Insert at position nextsort
		 */
		if (nextsort->au_next == NULL) {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
			return;
		} else if (nextsort->au_prev == NULL) {
			curmax->au_next = head;
			curmax->au_prev = NULL;
			head = curmax;
			nextsort->au_prev = curmax;
		} else {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
		}

	}

}

static void
sort_by_win()
{
	struct auth *curmax;
	struct auth *curposit;
	struct auth *nextsort;

	nextsort = head;
	while (nextsort) {

		/*
		 * Find curmax, the next target
		 */
		curposit = curmax = nextsort;
		while(curposit) {
			if ( curposit->au_wins > curmax->au_wins)
				curmax = curposit;
			curposit = curposit->au_next;
		}

		if (curmax == nextsort) {
			nextsort = curmax = nextsort->au_next;
			continue;
		}

		/*
		 * Delete curmax from the list
		 */
		if (curmax->au_next == NULL) {
			curmax->au_prev->au_next = NULL;
		} else if (curmax->au_prev == NULL) {
			head = curmax->au_next;
		} else {
			curmax->au_prev->au_next = curmax->au_next;
			curmax->au_next->au_prev = curmax->au_prev;
		}
		curmax->au_prev = NULL;
		curmax->au_next = NULL;

		/*
		 * Insert at position nextsort
		 */
		if (nextsort->au_next == NULL) {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
			return;
		} else if (nextsort->au_prev == NULL) {
			curmax->au_next = head;
			curmax->au_prev = NULL;
			head = curmax;
			nextsort->au_prev = curmax;
		} else {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
		}

	}

}

static void
sort_by_heat()
{
	struct auth *curmax;
	struct auth *curposit;
	struct auth *nextsort;

	nextsort = head;
	while (nextsort) {

		/*
		 * Find curmax, the next target
		 */
		curposit = curmax = nextsort;
		while(curposit) {
			if ( curposit->au_heat < curmax->au_heat)
				curmax = curposit;
			curposit = curposit->au_next;
		}

		if (curmax == nextsort) {
			nextsort = curmax = nextsort->au_next;
			continue;
		}

		/*
		 * Delete curmax from the list
		 */
		if (curmax->au_next == NULL) {
			curmax->au_prev->au_next = NULL;
		} else if (curmax->au_prev == NULL) {
			head = curmax->au_next;
		} else {
			curmax->au_prev->au_next = curmax->au_next;
			curmax->au_next->au_prev = curmax->au_prev;
		}
		curmax->au_prev = NULL;
		curmax->au_next = NULL;

		/*
		 * Insert at position nextsort
		 */
		if (nextsort->au_next == NULL) {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
			return;
		} else if (nextsort->au_prev == NULL) {
			curmax->au_next = head;
			curmax->au_prev = NULL;
			head = curmax;
			nextsort->au_prev = curmax;
		} else {
			curmax->au_next = nextsort;
			curmax->au_prev = nextsort->au_prev;
			curmax->au_prev->au_next = curmax;
			nextsort->au_prev = curmax;
		}

	}

}

static void
calc_heat_byall()
{
	struct auth *tmp;
	struct record *tmp2;
	int earliest;
	int latest;

	tmp = head;
	while(tmp) {

		tmp2 = tmp->au_list;
		if (tmp2->rec_year == 0)
			earliest = CURRENT_YEAR;
		while(tmp2) {
			if (tmp2->rec_year && (tmp2->rec_year < earliest))
				earliest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}

		tmp2 = tmp->au_list;
		latest = tmp2->rec_year;
		while(tmp2) {
			if (tmp2->rec_year > latest)
				latest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}
		if (tmp->au_entries > 1) {
			float distribution;
			float currency;
			float win_factor;

			distribution = ((float)(latest - earliest))/((float)(tmp->au_entries - 1));
			currency     = CURRENCY_WEIGHT * (float)(1 + CURRENT_YEAR - latest);
			win_factor   = (float)(tmp->au_entries + (NOM_WEIGHT * tmp->au_noms) + (WIN_WEIGHT * tmp->au_wins));
			tmp->au_heat = (distribution + currency)/win_factor;
		} else {
			tmp->au_heat = 999.0;
		}
		tmp = tmp->au_next;
	}
}

static void
calc_heat_bynom()
{
	struct auth *tmp;
	struct record *tmp2;
	int earliest;
	int latest;

	tmp = head;
	while(tmp) {

		tmp2 = tmp->au_list;
		if (tmp2->rec_year == 0)
			earliest = CURRENT_YEAR;
		while(tmp2) {
			if (tmp2->rec_year && (tmp2->rec_year < earliest))
				earliest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}

		tmp2 = tmp->au_list;
		latest = tmp2->rec_year;
		while(tmp2) {
			if (tmp2->rec_year > latest)
				latest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}
		if (tmp->au_noms > 1) {
			float distribution;
			float currency;
			float win_factor;

			distribution = ((float)(latest - earliest))/((float)(tmp->au_noms - 1));
			currency     = CURRENCY_WEIGHT * (float)(1 + CURRENT_YEAR - latest);
			win_factor   = (float)(tmp->au_noms + WIN_WEIGHT * tmp->au_wins);
			tmp->au_heat = (distribution + currency)/win_factor;
		} else {
			tmp->au_heat = 999.0;
		}
		tmp = tmp->au_next;
	}
}

static void
calc_heat_bywin()
{
	struct auth *tmp;
	struct record *tmp2;
	int earliest;
	int latest;

	tmp = head;
	while(tmp) {

		tmp2 = tmp->au_list;
		earliest = tmp2->rec_year;
		while(tmp2) {
			if (tmp2->rec_winner && (tmp2->rec_year < earliest))
				earliest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}

		tmp2 = tmp->au_list;
		latest = tmp2->rec_year;
		while(tmp2) {
			if (tmp2->rec_winner && (tmp2->rec_year > latest))
				latest = tmp2->rec_year;
			tmp2 = tmp2->rec_list;
		}
		if (tmp->au_wins > 1) {
			tmp->au_heat =  ((float)(latest - earliest))/((float)(tmp->au_wins - 1));
		} else {
			tmp->au_heat = 999.0;
		}
		tmp->au_heat += (float)(CURRENT_YEAR - latest);
		tmp = tmp->au_next;
	}
}

static void
dump_authors(type)
	int type;
{
	struct auth *tmp;

	printf("*** #%d\n", type);
	tmp = head;
	while(tmp) {
		printf("%s\n", tmp->au_author);
		tmp = tmp->au_next;
	}
	printf("\n");
}

static void
add_to_list(rec)
	struct record *rec;
{
	struct auth *tmp;
	struct record *tmp3;
	struct record *tmp4;
	int result;


	if (head == NULL) {
		head = (struct auth *)malloc(sizeof(struct auth));
		if (head == NULL) {
			printf("Can't malloc enough memory\n");
			exit(1);
		}
		head->au_next = NULL;
		head->au_prev = NULL;
		head->au_list = rec;
		head->au_entries = 1;
		if (rec->rec_award & (NEBULA|NEBULAWIN|HUGO|HUGOWIN)) 
			head->au_noms = 1;
		else
			head->au_noms = 0;
		if (rec->rec_winner)
			head->au_wins = 1;
		else
			head->au_wins = 0;
		strcpy(head->au_author, rec->rec_author);
		end = head;
		return;
	}

	/*
	 * First search to see if author is present
	 */
	tmp = head;
	while(tmp) {
		if (strcmp(tmp->au_author, rec->rec_author) == 0) {

			/*
			 * Found a matching author
			 */
			tmp->au_entries++;
			if (rec->rec_award & (NEBULA|NEBULAWIN|HUGO|HUGOWIN)) {
				if (rec->rec_award & NEBULA)
					tmp->au_noms++;
				if (rec->rec_award & NEBULAWIN)
					tmp->au_noms++;
				if (rec->rec_award & HUGO)
					tmp->au_noms++;
				if (rec->rec_award & HUGOWIN)
					tmp->au_noms++;
			}
			if (rec->rec_winner) {
				if (rec->rec_award & NEBULAWIN)
					tmp->au_wins++;
				if (rec->rec_award & HUGOWIN)
					tmp->au_wins++;
			}

			tmp3 = tmp->au_list;
			tmp4 = NULL;
			while(tmp3) {
				if (rec->rec_year < tmp3->rec_year) {
					if (tmp4 == NULL) {
						rec->rec_list = tmp->au_list;
						tmp->au_list = rec;
					} else {
						rec->rec_list  = tmp4->rec_list;
						tmp4->rec_list = rec;
					}
					return;
				}
				tmp4 = tmp3;
				tmp3 = tmp3->rec_list;
			}
			tmp4->rec_list = rec;
			return;
		} else {
			tmp = tmp->au_next;
		}
	}

	/*
	 * Must be a new author - add to the end of the list.
	 */
	tmp = (struct auth *)malloc(sizeof(struct auth));
	if (tmp == NULL) {
		printf("Can't malloc enough memory\n");
		exit(1);
	}

	end->au_next = tmp;
	tmp->au_prev = end;
	tmp->au_next = NULL;
	end = tmp;
	tmp->au_list = rec;
	tmp->au_entries = 1;
	if (rec->rec_award & (NEBULA|NEBULAWIN|HUGO|HUGOWIN)) 
		tmp->au_noms = 1;
	if (rec->rec_winner)
		tmp->au_wins = 1;
	else
		tmp->au_wins = 0;
	strcpy(tmp->au_author, rec->rec_author);
}


static void
print_record(rec)
	struct record *rec;
{
	int length;

	printf("  %4d  ", rec->rec_year);

	printf("[");
	length = 0;
	if (rec->rec_award & NEBULA) {
		printf("n");
		length++;
	}
	if (rec->rec_award & NEBULAWIN) {
		printf("N");
		length++;
	}
	if (rec->rec_award & WEBDAM) {
		printf("a");
		length++;
	}
	if (rec->rec_award & ASFANTH) {
		printf("b");
		length++;
	}
	if (rec->rec_award & SFHALL) {
		printf("c");
		length++;
	}
	if (rec->rec_award & HEALY) {
		printf("d");
		length++;
	}
	if (rec->rec_award & GROFF) {
		printf("e");
		length++;
	}
	if (rec->rec_award & ASIMOV) {
		printf("f");
		length++;
	}
	if (rec->rec_award & HARTWELL) {
		printf("g");
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
	if (rec->rec_award & NESFA) {
		printf("z");
		length++;
	}
	printf("]");
	for(;length<15; length++)
		printf(" ");

	switch(rec->rec_type) {
	case NOVEL:		printf("n   ");
				break;
	case NOVELLA:		printf("nv  ");
				break;
	case NOVELETTE:		printf("nt  ");
				break;
	case SHORT_STORY:	printf("ss  ");
				break;
	case SHORT_FICTION:	printf("sf  ");
				break;
	case COLLECTION:	printf("c   ");
				break;
	case ANTHOLOGY:		printf("a   ");
				break;
	case SERIES:		printf("se  ");
				break;
	case UNKNOWN:		printf("unk ");
				break;
	}

	if (rec->rec_winner)
		printf("* ");
	else
		printf("  ");

	printf("%s\n", rec->rec_title);
}

static void
print_full()
{
	struct auth *rec;
	struct record *tmp;

	rec = head;
	while(rec) {
		printf("%s:\n", rec->au_author);
		tmp = rec->au_list;
		while(tmp) {
			print_record(tmp);
			tmp = tmp->rec_list;
		}
		printf("\n");
		rec = rec->au_next;
	}
}

static void
print_winners()
{
	struct auth *rec;
	struct record *tmp;
	int length;
	int counter = 0;
	int currentmax = 9999;

	rec = head;
	while(rec) {
		if (rec->au_wins == 0) {
			rec = rec->au_next;
			continue;
		}

		if (rec->au_wins < currentmax) {
			counter++;
			currentmax = rec->au_wins;
		}

		if ( counter > 99) {
			printf("[%d] %s", counter,  rec->au_author);
		} else if ( counter > 9) {
			printf(" [%d] %s", counter,  rec->au_author);
		} else {
			printf("  [%d] %s", counter,  rec->au_author);
		}
		length = 39 - strlen(rec->au_author);
		if (length < 0)
			length = 0;

		for (;length; length--)
			printf(" ");

		if (rec->au_wins < 10)
			printf("-  %d Wins, ", rec->au_wins);
		else if (rec->au_wins == 1)
			printf("- %d Win,  ", rec->au_wins);
		else
			printf("- %d Wins, ", rec->au_wins);

		if (rec->au_noms < 10)
			printf("  %d Nominations\n", rec->au_noms);
		else
			printf(" %d Nominations\n", rec->au_noms);
		rec = rec->au_next;
	}
}

static void
print_noms()
{
	struct auth *rec;
	struct record *tmp;
	int length;
	int counter = 0;
	int currentmax = 9999;

	rec = head;
	while(rec) {
		if (rec->au_noms < 2) {
			rec = rec->au_next;
			continue;
		}

		if (rec->au_noms < currentmax) {
			counter++;
			currentmax = rec->au_noms;
		}

		if ( counter > 99) {
			printf("[%d] %s", counter,  rec->au_author);
		} else if ( counter > 9) {
			printf(" [%d] %s", counter,  rec->au_author);
		} else {
			printf("  [%d] %s", counter,  rec->au_author);
		}
		length = 39 - strlen(rec->au_author);
		if (length < 0)
			length = 0;

		for (;length; length--)
			printf(" ");

		if (rec->au_noms < 10)
			printf("-  %d Nominations, ", rec->au_noms);
		else
			printf("- %d Nominations, ", rec->au_noms);

		if (rec->au_wins < 10)
			printf("  %d Wins\n", rec->au_wins);
		else
			printf(" %d Wins\n", rec->au_wins);
		rec = rec->au_next;
	}
}

static void
print_heat()
{
	struct auth *rec;
	struct record *tmp;
	int length;
	int ranking = 0;

	rec = head;
	while(rec) {
		if (rec->au_heat < 900.0) {

			ranking++;
			if (ranking<10)
				printf("  [%d] %s", ranking, rec->au_author);
			else if (ranking<100)
				printf(" [%d] %s", ranking, rec->au_author);
			else
				printf("[%d] %s", ranking, rec->au_author);

			length = 39 - strlen(rec->au_author);
			if (length < 0)
				length = 0;
	
				for (;length; length--)
				printf(" ");

			printf("  %2.4f\n", rec->au_heat);
		}
		rec = rec->au_next;
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
	if (strcmp(buffer, "n") == 0) {
		rec->rec_type = NOVEL;
	} else if (strcmp(buffer, "nv") == 0) {
		rec->rec_type = NOVELLA;
	} else if (strcmp(buffer, "nt") == 0) {
		rec->rec_type = NOVELETTE;
	} else if (strcmp(buffer, "ss") == 0) {
		rec->rec_type = SHORT_STORY;
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
	} else if (strcmp(buffer, "sf") == 0) {
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
	if (strchr(buffer, 'a'))
		rec->rec_award |= WEBDAM;
	if (strchr(buffer, 'b'))
		rec->rec_award |= ASFANTH;
	if (strchr(buffer, 'c'))
		rec->rec_award |= SFHALL;
	if (strchr(buffer, 'd'))
		rec->rec_award |= HEALY;
	if (strchr(buffer, 'e'))
		rec->rec_award |= GROFF;
	if (strchr(buffer, 'f'))
		rec->rec_award |= ASIMOV;
	if (strchr(buffer, 'g'))
		rec->rec_award |= HARTWELL;
	if (strchr(buffer, 'z'))
		rec->rec_award |= NESFA;



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

		case 's':	if (strcmp(optarg, "n") == 0) {
					search_style = NOMINATIONS;
				} else if (strcmp(optarg, "w") == 0) {
					search_style = WINS;
				} else if (strcmp(optarg, "h") == 0) {
					search_style = HEAT;
				}
				options++;
				break;

                case 'c':       if (strcmp(optarg, "n") == 0) {
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

		case 'a':	sscanf(optarg, "%f", &sensitivity);
				printf("Scanning for author errors. Sensitivity = %2.4f\n", sensitivity);
				search_style = AUTH_ERRORS;
				options++;
				break;

		case 't':	sscanf(optarg, "%f", &sensitivity);
				printf("Scanning for title errors. Sensitivity = %2.4f\n", sensitivity);
				search_style = TITLE_ERRORS;
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
		if (hugos_only && (rec->rec_award != HUGO))
			continue;
		if (nebulas_only && (rec->rec_award != NEBULA))
			continue;
#ifdef REMOVE
		if ((rec->rec_type & categories) == 0)
			continue;
#endif
		add_to_list(rec);
	}

	switch(search_style) {
	case HEAT:		calc_heat_byall();
				sort_by_heat();
				print_heat();
				break;

	case SEARCHALL:		sort_by_auth();
				print_full();
				break;
	case NOMINATIONS:	sort_by_nom();
				print_noms();
				break;
	case WINS:		sort_by_win();
				print_winners();
				break;
	case AUTH_ERRORS:	find_author_errors();
				break;
	case TITLE_ERRORS:	find_title_errors();
				break;
	}

	exit(0);
}
