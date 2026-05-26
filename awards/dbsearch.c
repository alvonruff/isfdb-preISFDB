#ident "%W%	%G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DBASE "dbases/awards.dbase"
#define AGES  "dbases/ages.dbase"
#define OPTARGS "dbhmn?:s:c:a:t:y:"
#define CURRENT_YEAR 1995

#define SHORT_STORY     0x001
#define NOVELETTE       0x002
#define NOVELLA         0x004
#define NOVEL           0x010
#define NEBULA          0x020
#define HUGO            0x040
#define NEBULAWIN       0x080
#define HUGOWIN         0x100
#define SHORT_FICTION   (SHORT_STORY|NOVELETTE|NOVELLA)
#define ALL             (SHORT_STORY|NOVELETTE|NOVELLA|NOVEL)

#define NOMINATIONS	1
#define WINS		2
#define SEARCHALL	3
#define HEAT		4
#define AUTH_ERRORS	5
#define TITLE_ERRORS	6
#define MERGE		7
#define YEAR_DATA	8
#define PEAK_DATA	9
#define AGE_DATA	10

#define CURRENCY_WEIGHT	3.0
#define WIN_WEIGHT	1.5

extern char * optarg;
float sensitivity;
int agebucket[100];
int smoothbucket[100];
char vscreen[80][24];
int agetotal = 0;
int agetick = 0;

typedef struct auth {
	char		au_author[256];
	int		au_yob;
	int		au_noms;
	int		au_wins;
	int		au_first;
	int		au_last;
	int		au_peak;
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

typedef struct birth {
	char		b_author[256];
	int		b_year;
	struct birth	*b_next;
} birth_t;

birth_t *b_head;
birth_t *b_end;

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
do_merge(tmp1, tmp2)
	struct record *tmp1;
	struct record *tmp2;
{
	if ((tmp1->rec_award == HUGO) && tmp1->rec_winner) {
		tmp1->rec_award = HUGOWIN;
	}
	if ((tmp2->rec_award == HUGO) && tmp2->rec_winner) {
		tmp2->rec_award = HUGOWIN;
	}
	if ((tmp1->rec_award == NEBULA) && tmp1->rec_winner) {
		tmp1->rec_award = NEBULAWIN;
	}
	if ((tmp2->rec_award == NEBULA) && tmp2->rec_winner) {
		tmp2->rec_award = NEBULAWIN;
	}
	if (tmp1->rec_year < tmp2->rec_year) {
		tmp1->rec_award |= tmp2->rec_award;
		tmp2->rec_title[0] = 0;
		tmp2->rec_year     = -1;
	} else {
		tmp2->rec_award |= tmp1->rec_award;
		tmp1->rec_title[0] = 0;
		tmp1->rec_year     = -1;
	}
}

static void
merge()
{
	struct auth *rec1;
	struct auth *rec2;
	struct record *tmp1;
	struct record *tmp2;

	/*
	 * Go thru and delete duplicate records - defer to the
	 * record with the earlier date
	 */
	rec1 = head;
	while(rec1) {
		tmp1 = rec1->au_list;
		while(tmp1) {
			rec2 = rec1;
			while(rec2) {
				tmp2 = rec2->au_list;
				while(tmp2) {
					if ( (tmp1 != tmp2) &&
					     (strcmp(tmp1->rec_title, tmp2->rec_title) == 0) ) {
						/*
						 * Okay the titles match - now check the type
						 */
						if ((tmp1->rec_type == NOVEL) && 
						    (tmp2->rec_type == NOVEL)) {
							do_merge(tmp1, tmp2);
						} else if ((tmp1->rec_type != NOVEL) && 
							   (tmp2->rec_type != NOVEL)) {
							do_merge(tmp1, tmp2);
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
sort_by_peak()
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
			if ( curposit->au_peak > curmax->au_peak)
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
calc_heat_bynom()
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
			if (tmp2->rec_year < earliest)
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


	agetotal++;
	if (head == NULL) {
		head = (struct auth *)malloc(sizeof(struct auth));
		if (head == NULL) {
			printf("Can't malloc enough memory\n");
			exit(1);
		}
		head->au_next = NULL;
		head->au_prev = NULL;
		head->au_list = rec;
		head->au_noms = 1;
		head->au_yob  = 0;
		head->au_first = 999;
		head->au_last  = 0;
		head->au_peak  = 0;
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
			tmp->au_noms++;
			if (rec->rec_winner)
				tmp->au_wins++;

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
	tmp->au_noms = 1;
	tmp->au_yob  = 0;
	tmp->au_first = 999;
	tmp->au_last  = 0;
	tmp->au_peak  = 0;
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

	printf("  %d  ", rec->rec_year);

	if (rec->rec_award == NEBULA)
		printf("Nebula  ");
	else if (rec->rec_award == HUGO)
		printf("Hugo    ");

	switch(rec->rec_type) {
	case NOVEL:		printf("Novel         ");
				break;
	case NOVELLA:		printf("Novella       ");
				break;
	case NOVELETTE:		printf("Novelette     ");
				break;
	case SHORT_STORY:	printf("Short Story   ");
				break;
	case SHORT_FICTION:	printf("Short Fiction ");
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
		printf("%s (%d Nominations, %d Wins):\n", rec->au_author, 
			rec->au_noms, rec->au_wins);
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
print_newrec(rec)
	struct record *rec;
{
	int length;

	if (rec->rec_year == -1)
		return;

	printf("%s|", rec->rec_title);
	printf("%s|", rec->rec_author);

	switch(rec->rec_type) {
	case NOVEL:		printf("n|");
				break;
	case NOVELLA:		printf("nv|");
				break;
	case NOVELETTE:		printf("nt|");
				break;
	case SHORT_STORY:	printf("ss|");
				break;
	case SHORT_FICTION:	printf("sf|");
				break;
	}

	printf("%d|", rec->rec_year);
	printf("[");
	if (rec->rec_award & HUGO) {
		printf("h");
	}
	if (rec->rec_award & HUGOWIN) {
		printf("H");
	}
	if (rec->rec_award & NEBULA) {
		printf("n");
	}
	if (rec->rec_award & NEBULAWIN) {
		printf("N");
	}
	printf("]\n");
}

static void
print_new()
{
	struct auth *rec;
	struct record *tmp;

	rec = head;
	while(rec) {
		tmp = rec->au_list;
		while(tmp) {
			print_newrec(tmp);
			tmp = tmp->rec_list;
		}
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
	rec->rec_list = NULL;
	rec->rec_noms = 1;
	if (rec->rec_winner)
		rec->rec_wins = 1;
	else
		rec->rec_wins = 0;
	return(rec);
}


struct birth *
parse_age(fp)
	FILE *fp;
{
	int in;
	char *tmp;
	char buffer[32];
	struct birth *rec;

	rec = (struct birth *)malloc(sizeof(struct birth));
	if (rec == NULL) {
		printf("Can't malloc enough memory\n");
		exit(1);
	}

	/*
	 * Parse Author
	 */
	tmp = rec->b_author;
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
	sscanf(buffer,"%d", &(rec->b_year));
	return(rec);
}

static void
peak_data()
{
	FILE *fp;
	struct birth *brec;
	struct auth *rec;
	struct record *tmp;
	int loop;
	int peaktotal = 0;
	int nomtotal = 0;
	int firsttotal = 0;
	int lasttotal = 0;
	int totalpeaks = 0;

	/*
	 * Read in the age database
	 */
	fp = fopen(AGES, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}


	while((brec = parse_age(fp))) {
		brec->b_next = b_head;
		b_head = brec;
	}

	/*
	 * Fill in the au_yob fields
	 */
	rec = head;
	while(rec) {
		brec = b_head;
		while(brec) {
			if (strcmp(brec->b_author, rec->au_author) == 0) {
				rec->au_yob = brec->b_year;
			}
			brec = brec->b_next;
		}
		rec = rec->au_next;
	}

	printf("Author                             Noms First Last Peak (Years Since Last Nom)\n");
	for(loop=0;loop<70; loop++)
		printf("-");
	printf("\n");

	rec = head;
	while(rec) {
		int totalage;
		int age;

		if (rec->au_yob == 0) {
			rec = rec->au_next;
			continue;
		}

		totalage = 0;
		tmp = rec->au_list;
		while(tmp) {
			if (tmp->rec_year == 0) {
				tmp = tmp->rec_list;
				continue;
			}
			age = (tmp->rec_year - rec->au_yob);
			if (age < rec->au_first) 
				rec->au_first = age;
			if (age > rec->au_last)
				rec->au_last = age;
			totalage += age;
			tmp = tmp->rec_list;
		}
		rec->au_peak = totalage/rec->au_noms;
		if (totalage) {
			peaktotal += rec->au_peak;
			nomtotal += rec->au_noms;
			firsttotal += rec->au_first;
			lasttotal += rec->au_last;
			totalpeaks++;
		}
		rec = rec->au_next;
	}

	sort_by_peak();
	rec = head;
	while(rec) {
		int age;

		if(rec->au_first == 999) {
			rec = rec->au_next;
			continue;
		}
		printf("%s", rec->au_author);
		for(loop=strlen(rec->au_author); loop<35; loop++)
			printf(" ");
		age = (CURRENT_YEAR-(rec->au_yob));
		printf("%02d   %02d    %02d   %02d   (%02d)\n", rec->au_noms, rec->au_first,
			rec->au_last, rec->au_peak, age-(rec->au_last));
		rec = rec->au_next;
	}


	for(loop=0;loop<70; loop++)
		printf("-");
	printf("\nAverage:");
	for(loop=strlen("Average:"); loop<35; loop++)
		printf(" ");
	printf("%02d   %02d    %02d   %02d\n", nomtotal/totalpeaks, firsttotal/totalpeaks,
		lasttotal/totalpeaks, peaktotal/totalpeaks);
}


static void
age_data(int max_age)
{
	FILE *fp;
	struct birth *brec;
	struct auth *rec;
	struct record *tmp;
	int loop;
	int peaktotal = 0;
	int nomtotal = 0;
	int firsttotal = 0;
	int lasttotal = 0;
	int totalpeaks = 0;

	/*
	 * Read in the age database
	 */
	fp = fopen(AGES, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}


	while((brec = parse_age(fp))) {
		brec->b_next = b_head;
		b_head = brec;
	}

	/*
	 * Fill in the au_yob fields
	 */
	rec = head;
	while(rec) {
		brec = b_head;
		while(brec) {
			if (strcmp(brec->b_author, rec->au_author) == 0) {
				rec->au_yob = brec->b_year;
			}
			brec = brec->b_next;
		}
		rec = rec->au_next;
	}

	rec = head;
	while(rec) {
		int totalage;
		int age;

		if (rec->au_yob == 0) {
			rec = rec->au_next;
			continue;
		}

		totalage = 0;
		tmp = rec->au_list;
		while(tmp) {
			if (tmp->rec_year == 0) {
				tmp = tmp->rec_list;
				continue;
			}
			age = (tmp->rec_year - rec->au_yob);
			if (age < max_age) {
				printf("%02d ", age);
				print_newrec(tmp);
			}
			tmp = tmp->rec_list;
		}
		rec = rec->au_next;
	}
}


static void
year_data()
{
	FILE *fp;
	struct birth *brec;
	struct auth *rec;
	struct record *tmp;
	int loop;

	/*
	 * Read in the age database
	 */
	fp = fopen(AGES, "r");
	if (fp == NULL) {
		perror("Can't open database");
		exit(1);
	}


	while((brec = parse_age(fp))) {
		brec->b_next = b_head;
		b_head = brec;
	}

	/*
	 * Fill in the au_yob fields
	 */
	rec = head;
	while(rec) {
		brec = b_head;
		while(brec) {
			if (strcmp(brec->b_author, rec->au_author) == 0) {
				rec->au_yob = brec->b_year;
			}
			brec = brec->b_next;
		}
		rec = rec->au_next;
	}

	for(loop=0; loop<100; loop++) {
		agebucket[loop] = 0;
	}

	rec = head;
	while(rec) {
		int age;

#ifdef NO_FILTER
		if ((rec->au_yob<1930) || (rec->au_yob>1939) ) {
#else
		if (rec->au_yob == 0) {
#endif
			rec = rec->au_next;
			continue;
		}

		tmp = rec->au_list;
		while(tmp) {
			agetick++;
			agebucket[(tmp->rec_year - rec->au_yob)]++;
			tmp = tmp->rec_list;
		}
		rec = rec->au_next;
	}

	/*
	 * Smooth the data
	 */
	for(loop=1; loop<99; loop++) {
		smoothbucket[loop] = (agebucket[loop-1] + agebucket[loop] + agebucket[loop+1])/3;
	}
	for(loop=0; loop<100; loop++) {
		agebucket[loop] = smoothbucket[loop];
	}

	printf("Percent Noms used = %d\n", (100 * agetick)/agetotal );
#ifdef REMOVE
	for(loop=0; loop<100; loop++) {
		printf("%d   %d\n", loop, agebucket[loop]);
	}
#endif
}

static void
print_screen()
{
	int x,x2,y,loop,loop2;
	int leftmargin = 5;
	int botmargin  = 1;
	int width      = 80 - leftmargin;
	int height     = 24 - botmargin;
	int maxnoms    = 0;
	int minage     = 20;
	int maxage     = 80;
	int yscale;
	float ytrans;

	for(loop=0; loop<100; loop++) {
		if ( agebucket[loop] > maxnoms)
			maxnoms = agebucket[loop];
	}
	yscale = ((maxnoms/5) * 5) + 5;

	ytrans = (float)(height-botmargin) / (float)yscale;

	for(x=0; x<80; x++)
		for(y=0; y<24; y++)
			vscreen[x][y] = ' ';

	/*
	 * Draw the vertical lines
	 */
	for(loop=minage; loop<=maxage; loop += 10) {
		char buf[5];
		char *tmp;

		x = leftmargin + loop - minage;
		for(y=0; y<(height-botmargin); y++) {
			vscreen[x][y] = '|';
		}

		sprintf(buf,"%d", loop);
		tmp = buf;
		x2 = loop;
		while(*tmp) {
			vscreen[(leftmargin + x2 - minage)][height] = *tmp;
			x2++; tmp++;
		}
	}

	/*
	 * Draw the horizontal lines
	 */
	for(loop=0; loop<=yscale; loop += 5) {
		char buf[5];
		char *tmp;

		y = (height - botmargin) - (int)( (float)loop * ytrans);
		for(x=minage; x<=maxage; x++) {
			x2 = leftmargin + x - minage;
			vscreen[x2][y] = '-';
		}

		sprintf(buf,"%d", loop);
		tmp = buf;
		x2 = leftmargin - strlen(buf) - 1;
		while(*tmp) {
			vscreen[x2][y] = *tmp;
			x2++; tmp++;
		}
	}

	/*
	 * Draw the intersection points
	 */
	for(loop=minage; loop<=maxage; loop += 10) {
		x = leftmargin + loop - minage;
		for(loop2=0; loop2<=yscale; loop2 += 5) {
			y = (height - botmargin) - (int)( (float)loop2 * ytrans);
			vscreen[x][y] = '+';
		}
	}

	/*
	 * Draw the data
	 */
	for(loop=minage; loop<maxage; loop++) {
		y = (height - botmargin) - (int)( (float)agebucket[loop] * ytrans);
		x = leftmargin + loop - minage;
		for(;y<(height-botmargin); y++)
			vscreen[x][y] = '#';
	}

	for(y=0; y<24; y++) {
		for(x=0; x<80; x++) {
			printf("%c", vscreen[x][y]);
		}
		printf("\n");
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
	int max_age	 = 0;
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

		case 'm': 	search_style = MERGE;
				break;

		case 'b': 	search_style = YEAR_DATA;
				break;

		case 'd': 	search_style = PEAK_DATA;
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

		case 'y':	sscanf(optarg, "%d", &max_age);
				search_style = AGE_DATA;
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
		if ((rec->rec_type & categories) == 0)
			continue;
		add_to_list(rec);
	}

	switch(search_style) {
	case HEAT:		calc_heat_bynom();
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
	case MERGE:		merge();
				print_new();
				break;
	case YEAR_DATA:		year_data();
				print_screen();
				break;
	case PEAK_DATA:		peak_data();
				break;
	case AGE_DATA:		age_data(max_age);
				break;
	}

	exit(0);
}
