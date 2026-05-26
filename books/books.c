/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "books.h"

extern char	*optarg;

struct list award_table[] = {
	{ "hu",	AW_HUGO},
	{ "ne",	AW_NEBULA},
	{ "ls",	AW_LOCUS_SF},
	{ "lh",	AW_LOCUS_HOR},
	{ "lf",	AW_LOCUS_FANT},
	{ "ca",	AW_CAMPBELL},
	{ "st",	AW_STOKER},
	{ "cl",	AW_CLARKE},
	{ "pk",	AW_PKDICK},
	{ "wf",	AW_WFA},
};

struct list cat_table[] = {
	{ "n",	AW_NOVEL},
	{ "nv",	AW_NOVELLA},
	{ "nt",	AW_NOVELETTE},
	{ "ss",	AW_SHORT_STORY},
	{ "sf",	AW_SHORT_FICTION},
	{ "fn",	AW_FIRST_NOVEL},
};

award_t		*aw_head = NULL;
award_t		*aw_tail = NULL;
record_t	*rec_head = NULL;
record_t	*rec_tail = NULL;
author_t	*au_head = NULL;
author_t	*au_end = NULL;


int		balanced_mode		= 0;
int		popular_mode		= 0;
int		critical_mode		= 0;
int		filter_year		= 0;
int		filter_throttle		= 0;
int		filter_winners_only	= 0;
int		filter_dont_own		= 0;
int		filter_own		= 0;
int		filter_read		= 0;
int		filter_did_read		= 0;
int		filter_hugos		= 0;
char		filter_author[256]	= {0};
char		filter_title[256]	= {0};

static void
usage()
{
	printf("books \n");
	printf("      [-a author]       (only authors with this substring)\n");
	printf("      [-A]              (list everything by author)\n");
	printf("      [-b]              (display buy list)\n");
	printf("      [-C]              (Critical Mode)\n");
	printf("      [-g ca|cl|hu|ne|lf|lh|ls|st]\n");
	printf("                        (generate an award listing\n");
	printf("      [-l]              (dump the entire database sorted by score)\n");
	printf("      [-o]              (display own list)\n");
	printf("      [-p]              (print out database (debug))\n");
	printf("      [-P]              (Popular Mode)\n");
	printf("      [-r]              (display read list)\n");
	printf("      [-s]              (top Scoring authors)\n");
	printf("      [-t title]        (only titles with this substring)\n");
	printf("      [-w]              (winners only)\n");
	printf("      [-T threshold]    (use only scores > threshold)\n");
	printf("      [-y year]         (specific year only)\n");
	exit(1);
}



static void
free_rec(record_t *rec)
{
}

static void
inc_rec_points(award_t *aw, record_t *rec)
{
	int level = aw->aw_level;
	int points;
	int age;

	switch(level) {
        case 1: if(popular_mode) {
			points = 140;
		} else if (critical_mode) {
			points = 50;
		} else {
			points = 90;
		}
                break;
        case 2: points = 45;
                break;
        case 3: points = 43;
                break;
        case 4: points = 41;
                break;
        case 5: points = 39;
                break;
        default: 
		points = (39-level);
        }

	if (popular_mode) {
		int age;

		if ( (aw->aw_award == AW_HUGO) ||
		     (aw->aw_award == AW_LOCUS_SF) ||
		     (aw->aw_award == AW_LOCUS_FANT) ||
		     (aw->aw_award == AW_LOCUS_HOR) ) {
			points *= 4;
		}
		age = CURRENT_YEAR - rec->rec_year;
		if (age > 10) {
			points = 0;
		} else {
			points -= age;
		}
	} else if (critical_mode) {
		if ( (aw->aw_award != AW_HUGO) &&
		     (aw->aw_award != AW_LOCUS_SF) &&
		     (aw->aw_award != AW_LOCUS_FANT) &&
		     (aw->aw_award != AW_LOCUS_HOR) ) {
			points *= 4;
		}
		age = CURRENT_YEAR - rec->rec_year;
	}

	if (points < 0)
		points = 0;

	rec->rec_points += points;
}

void
insert_award(char *type, int level, char *cat, int year)
{
	int loop;
	award_t *aw;

	aw = (award_t *)malloc(sizeof(award_t));
	for(loop=0; loop<AW_MAXTYPES; loop++) {
		if (strcmp(award_table[loop].li_type, type) == 0) {
			aw->aw_award = award_table[loop].li_numb;
			break;
		}
	}

	for(loop=0; loop<AW_MAXCATS; loop++) {
		if (strcmp(cat_table[loop].li_type, cat) == 0) {
			aw->aw_category = cat_table[loop].li_numb;
			break;
		}
	}

	aw->aw_level = level;
	aw->aw_year  = year;

	aw->aw_next = NULL;
	if (aw_head) {
		aw_tail->aw_next = aw;
		aw_tail = aw;
	} else {
		aw_head = aw_tail = aw;
	}
}


void
insert_record(char *title, char *author, int year, int own, int read)
{
	record_t	*rec;
	award_t		*aw;

	rec = (record_t *)malloc(sizeof(record_t));
	strcpy(rec->rec_title, title);
	strcpy(rec->rec_author, author);
	rec->rec_year   = year;
	rec->rec_ownit  = own;
	rec->rec_readit = read;
	rec->rec_winner = 0;
	rec->rec_points = 0;
	rec->rec_awards = aw_head;
	aw_head = NULL;
	aw_tail = NULL;

	aw = rec->rec_awards;
	while(aw) {
#ifdef OLDWAY
		INC_REC_POINTS(aw->aw_level);
#endif
		inc_rec_points(aw, rec);
		if (aw->aw_level == 1)
			rec->rec_winner = 1;
		aw = aw->aw_next;
	}

	if (filter_year && (filter_year != rec->rec_year)) {
		free_rec(rec);
		return;
	}
	if (filter_winners_only && !rec->rec_winner) {
		free_rec(rec);
		return;
	}
	if (filter_throttle && (rec->rec_points < filter_throttle) ) {
		free_rec(rec);
		return;
	}
	if (filter_author[0] && !(strstr(rec->rec_author, filter_author)) ) {
		free_rec(rec);
		return;
	}
	if (filter_title[0] && !(strstr(rec->rec_title, filter_title)) ) {
		free_rec(rec);
		return;
	}
	if (filter_own && rec->rec_ownit) {
		free_rec(rec);
		return;
	}
	if (filter_dont_own && rec->rec_ownit) {
		free_rec(rec);
		return;
	}
	if (filter_read && (!rec->rec_ownit || rec->rec_readit)) {
		free_rec(rec);
		return;
	}
	if (filter_did_read && !rec->rec_readit) {
		free_rec(rec);
		return;
	}
	if (filter_hugos) {
		award_t *aw;
		int	notokay = 1;

		aw = rec->rec_awards;
		while(aw) {
			if (aw->aw_award == AW_HUGO)
				notokay = 0;
			aw = aw->aw_next;
		}
		if (notokay) {
			free_rec(rec);
			return;
		}
	}

	if (rec_head) {
		rec_tail->rec_next = rec;
		rec->rec_next = NULL;
		rec_tail = rec;
#ifdef REMOVE
		rec->rec_next = rec_head;
		rec_head = rec;
#endif
	} else {
		rec_head = rec_tail = rec;
	}
}

/*
 * STRCMP() does a a caseless comparison, so "de Lint" doesn't get
 * alphabetized after "Zelazny"
 */
int
STRCMP(char *str1, char *str2)
{
	char buf1[256];
	char buf2[256];
	char *tmp1;
	char *tmp2;

	tmp1 = str1;
	tmp2 = buf1;
	while (*tmp1) {
		*tmp2 = toupper(*tmp1);
		tmp1++; tmp2++;
	}
	*tmp2 = 0;

	tmp1 = str2;
	tmp2 = buf2;
	while (*tmp1) {
		*tmp2 = toupper(*tmp1);
		tmp1++; tmp2++;
	}
	*tmp2 = 0;
	return(strcmp(buf1, buf2) );
}


static void
insert_author(author_t *au)
{
	author_t *au2;

	if (au_head) {
		au2 = au_head;
		while(au2) {
			int strcmp1 = STRCMP(au->au_lastname, au2->au_lastname);
			int strcmp2 = strcmp(au->au_author, au2->au_author);

			if ( ((strcmp1 <  0) && (au2->au_prev)) ||
			     ((strcmp1 == 0) && (strcmp2 < 0) && (au2->au_prev)) ) {
				au->au_prev  = au2->au_prev;
				au2->au_prev = au;
				au->au_prev->au_next = au;
				au->au_next = au2;
				return;
			} else if ( ((strcmp1 <  0) && !(au2->au_prev)) ||
				    ((strcmp1 == 0) && (strcmp2 < 0)  && !(au2->au_prev)) ) {
				au->au_prev  = NULL;
				au2->au_prev = au;
				au->au_next  = au2;
				au_head      = au;
				return;
			}
			au2 = au2->au_next;
		}

		au_end->au_next = au;
		au->au_prev = au_end;
		au->au_next = NULL;
		au_end = au;
	} else {
		au->au_next = NULL;
		au->au_prev = NULL;
		au_head = au;
		au_end  = au;
	}
}

static void
dumplist()
{
	record_t	*rec;
	award_t		*aw;

	rec = rec_head;
	while(rec) {
		printf("%s,%s,%d,%d,%d,%d,%d\n", rec->rec_title,
			rec->rec_author, rec->rec_year, rec->rec_winner,
			rec->rec_points, rec->rec_ownit, rec->rec_readit);
		aw = rec->rec_awards;
		while(aw) {
			printf("     %s %d %s %d\n", award_table[aw->aw_award - 1].li_type,
			aw->aw_year, cat_table[aw->aw_category - 1].li_type, aw->aw_level);
			aw = aw->aw_next;
		}
		rec = rec->rec_next;
	}
}


#define MAX_TITLE_WIDTH 35
#define INDENT_NO	 0
#define INDENT_YES	 1
#define POINTS_NO	 0
#define POINTS_YES	 1
#define DOYEAR_NO	 0
#define DOYEAR_YES	 1
#define DOAUTH_NO	 0
#define DOAUTH_YES	 1


static void
print_entry(record_t *rec)
{
	award_t *aw;

	/*
	 * The Alchemists|Geary Gravel|1984|[pk_3_n_1984]|n|n
	 */
	printf("%s|", rec->rec_title);
	printf("%s|", rec->rec_author);
	printf("%d|", rec->rec_year);
#ifndef REMOVE_THIS_STUFF
	printf("n\n");
	return;
#endif

	aw = rec->rec_awards;
	while(aw) {
		printf("[");
		printf("%s_", award_table[aw->aw_award - 1].li_type);
		printf("%d_", aw->aw_level);
		printf("%s_", cat_table[aw->aw_category - 1].li_type);
#ifndef REMOVE
		if (aw->aw_award == AW_CLARKE)
			printf("%d", aw->aw_year + 1);
		else
#endif
			printf("%d", aw->aw_year);
		printf("]");
		aw = aw->aw_next;
	}
	printf("|");

	if(rec->rec_ownit)
		printf("y|");
	else
		printf("n|");
	if(rec->rec_readit)
		printf("y\n");
	else
		printf("n\n");
}


static void
print_record_orig(rec)
        struct record *rec;
{
        int		length;
	award_t		*aw;


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

	aw = rec->rec_awards;
	printf("     ");
	while(aw) {
		printf("[%s%d%s%d]", award_table[aw->aw_award - 1].li_type,
		aw->aw_year, cat_table[aw->aw_category - 1].li_type, aw->aw_level);
		aw = aw->aw_next;
	}
	printf("\n");
}


static void
print_record(record_t *rec, int indent, int dopoints, int doyear, int doauth, int offset)
{
	int length;

	if (indent) {
		printf("    ");
		offset += 4;
	}
	if (dopoints) {
		printf("%3d ", rec->rec_points);
		offset += 4;
	}
	if (doyear) {
		printf("%4d ", rec->rec_year);
		offset += 5;
	}
	printf("%s ", rec->rec_title);

	if (doauth) {
		length = strlen(rec->rec_title);
		if (length <= MAX_TITLE_WIDTH) {
			for(length = MAX_TITLE_WIDTH-length; length; length--)
				printf(" ");
			printf("%s\n", rec->rec_author);
		} else {
			printf(" --\n");
			for(length = MAX_TITLE_WIDTH+offset+1; length; length--)
				printf(" ");
			printf("%s\n", rec->rec_author);
		}
	} else {
		printf("\n");
	}
}

static void
print_entries()
{
	record_t	*rec;

	rec = rec_head;
	while(rec) {
		print_entry(rec);
		rec = rec->rec_next;
	}
}


static void
print_raw()
{
	record_t	*rec;
	record_t	*rec2;
	int		maxpoints = 0;
	int		loop;

	rec = rec_head;
	while(rec) {
		if (rec->rec_points > maxpoints) {
			maxpoints = rec->rec_points;
		}
		rec = rec->rec_next;
	}

	for(loop=maxpoints; loop>0; loop--) {
		rec = rec_head;
		rec2 = NULL;
		while(rec) {
			if (rec->rec_points == loop) {
				print_record(rec, INDENT_YES, POINTS_YES, DOYEAR_YES, DOAUTH_YES, 0);
				fflush(stdout); fflush(stderr);
				if (rec2) {
					rec2->rec_next = rec->rec_next;
				}
			}
			rec2 = rec;
			rec = rec->rec_next;
		}
	}
}


static void
print_by_author2()
{
	record_t	*rec;
	author_t	*au;
	int		maxyear = 0;
	int		minyear = 3000;
	int		loop;

	au = au_head;
	while(au) {
		printf("\n%s:\n", au->au_author);
		rec = au->au_list;
		while(rec) {
			if (rec->rec_year > maxyear)
				maxyear = rec->rec_year;
			if (rec->rec_year < minyear)
				minyear = rec->rec_year;
			rec = rec->rec_aulink;
		}
		for(loop=minyear; loop<=maxyear; loop++) {
			rec = au->au_list;
			while(rec) {
				if (rec->rec_year == loop) {
					print_record_orig(rec);
#ifdef REMOVE
					print_record(rec, INDENT_YES, POINTS_YES, DOYEAR_YES, DOAUTH_NO, 0);
#endif
				}
				rec = rec->rec_aulink;
			}
		}
		au = au->au_next;
	}
}


static void
print_by_author()
{
	record_t	*rec;
	author_t	*au;
	int		maxpoints;
	int		loop;

	au = au_head;
	while(au) {
		printf("%s:\n", au->au_author);
		rec = au->au_list;
		maxpoints = 0;
		while(rec) {
			if (rec->rec_points > maxpoints)
				maxpoints = rec->rec_points;
			rec = rec->rec_aulink;
		}
		for(loop=maxpoints; loop>0; loop--) {
			rec = au->au_list;
			while(rec) {
				if (rec->rec_points == loop) {
					print_record(rec, INDENT_YES, POINTS_YES, DOYEAR_YES, DOAUTH_NO, 0);
				}
				rec = rec->rec_aulink;
			}
		}
		au = au->au_next;
	}
}

int		year_printed;
int		cat_printed;

static void
print_cat(int cat)
{
	switch(cat) {
	case AW_NOVEL:		printf("  [Novel]\n");
				break;
	case AW_NOVELLA: 	printf("  [Novella]\n");
				break;
	case AW_NOVELETTE:	printf("  [Novelette]\n");
				break;
	case AW_SHORT_STORY: 	printf("  [Short Story]\n");
				break;
	case AW_SHORT_FICTION:	printf("  [Short Fiction]\n");
				break;
	case AW_FIRST_NOVEL:	printf("  [First Novel]\n");
				break;
	}
}



static void
do_cat(int category, int year, int award)
{
	record_t	*rec;
	record_t	*rec2;
	award_t		*aw;

	rec  = rec_head;
	rec2 = NULL;
	while(rec) {
		aw = rec->rec_awards;
		while(aw) {
			if ((aw->aw_year == year) && 
			    (aw->aw_category == category) &&
			    (aw->aw_award == award) ) {
				if (!year_printed) {
					printf("\n[%d]\n", year);
					year_printed = 1;
				}
				if (!cat_printed) {
					print_cat(category);
					cat_printed = 1;
				}
				if (aw->aw_level == 1) {
					printf("  * ");
				} else {
					printf("    ");
				}
				print_record(rec, INDENT_NO, POINTS_NO, DOYEAR_NO, DOAUTH_YES, 4);
#ifdef REMOVE
				if (rec2) {
					rec2->rec_next = rec->rec_next;
				}
#endif
				break;
			}
			aw = aw->aw_next;
		}
		rec2 = rec;
		rec  = rec->rec_next;
	}
}

static void
print_list(int award)
{
	int		year;
	int		lastyear = CURRENT_YEAR;
	int		loop;
	int		category;
	int		winners;
	int		category_hits;
	award_t		*aw;
	record_t	*rec;
	record_t	*rec2;

	printf("                         ");

	switch(award) {
	case AW_HUGO:	year = 1953;
			printf("[Hugo Nominees List]\n\n");
			break;
	case AW_CAMPBELL:
			year = 1973;
			printf("[Campbell Nominees List]\n\n");
			break;
	case AW_CLARKE:
			year = 1987;
			printf("[Clarke Nominees List]\n\n");
			break;
	case AW_NEBULA:	year = 1965;
			printf("[Nebula Nominees List]\n\n");
			break;
	case AW_LOCUS_SF:
			year = 1973;
			printf("[Locus SF Nominees List]\n\n");
			break;
	case AW_LOCUS_FANT:
			year = 1978;
			printf("[Locus Fantasy Nominees List]\n\n");
			break;
	case AW_LOCUS_HOR:
			year = 1989;
			printf("[Locus Horror Nominees List]\n\n");
			break;
	case AW_STOKER:
			year = 1988;
			printf("[Bram Stoker Nominees List]\n\n");
			break;
	default:	year = 1900;
			printf("[??? Nominees List]\n\n");
	}

	for( ; year<=lastyear; year++) {
		year_printed  = 0;
		for (loop=0; loop<=5; loop++) {
			switch(loop) {
			case 0: category = AW_NOVEL;
				break;
			case 1: category = AW_NOVELLA;
				break;
			case 2: category = AW_NOVELETTE;
				break;
			case 3: category = AW_SHORT_STORY;
				break;
			case 4: category = AW_SHORT_FICTION;
				break;
			case 5: category = AW_FIRST_NOVEL;
				break;
			}
			cat_printed = 0;
			do_cat(category, year, award);
		}
	}
}

static void
print_list2(int award)
{
	record_t	*rec;
	award_t		*aw;

	rec  = rec_head;
	while(rec) {
		rec  = rec->rec_next;
		aw = rec->rec_awards;
		while(aw) {
			if (aw->aw_award == award) {
				printf("%s|", rec->rec_title);
				printf("%s|", rec->rec_author);
				printf("%d|", aw->aw_year);
				printf("n|%d|\n", aw->aw_level);
				break;
			}
			aw = aw->aw_next;
		}
	}
}


static void
print_top_authors()
{
	record_t	*rec;
	record_t	*rec2;
	author_t	*au;
	int		maxpoints = 0;
	int		loop;
	int		loop2;
	int		year_printed;

	au = au_head;
	while(au) {
		if (au->au_points > maxpoints)
			maxpoints = au->au_points;
		au = au->au_next;
	}

	for(loop=maxpoints; loop>0; loop--) {
		au = au_head;
		while(au) {
			if (au->au_points == loop) {
				printf("%05d %s\n", au->au_points, au->au_author);
			}
			au = au->au_next;
		}
	}
}


static void
print_by_year()
{
	record_t	*rec;
	record_t	*rec2;
	author_t	*au;
	int		maxpoints;
	int		maxyear = 0;
	int		minyear = 10000;
	int		loop;
	int		loop2;
	int		year_printed;

	rec = rec_head;
	while(rec) {
		if (rec->rec_year < minyear)
			minyear = rec->rec_year;
		if (rec->rec_year > maxyear)
			maxyear = rec->rec_year;
		rec = rec->rec_next;
	}

	for (loop=minyear; loop<=maxyear; loop++) {
		year_printed = 0;
		rec = rec_head;
		maxpoints = 0;
		while(rec) {
			if (rec->rec_year != loop) {
				rec = rec->rec_next;
				continue;
			}
			if (rec->rec_points > maxpoints)
				maxpoints = rec->rec_points;
			rec = rec->rec_next;
		}
		for(loop2=maxpoints; loop2>0; loop2--) {
			rec = rec_head;
			rec2 = NULL;
			while(rec) {
				if (rec->rec_year != loop) {
					rec2 = rec;
					rec = rec->rec_next;
					continue;
				}
				if (rec->rec_points == loop2) {
					if (!year_printed) {
						printf("\n[%d]:\n", loop);
						year_printed = 1;
					}

					print_record(rec, INDENT_NO, POINTS_YES, DOYEAR_NO, DOAUTH_YES, 0);

					/*
					 * Jettison this record to speed things up
					 */
					if (rec2) {
						rec2->rec_next = rec->rec_next;
					}
				}
				rec2 = rec;
				rec = rec->rec_next;
			}
		}
	}
}




static void
create_aulist()
{
	record_t	*rec;
	award_t		*aw;
	author_t	*au;

	rec = rec_head;
	while(rec) {

		/*
		 * First try to find author
		 */
		au = au_head;
		while(au) {
			if (strcmp(au->au_author, rec->rec_author) == 0) {
				break;
			}
			au = au->au_next;
		}

		/*
		 * If not there, create one
		 */
		if (!au) {
			char *tmp;

			au = (author_t *)malloc(sizeof(author_t));
			strcpy(au->au_author, rec->rec_author);

			/*
			 * Find the author's last name
			 */
			tmp = strstr(au->au_author, " and ");
			if (tmp) {
				tmp--;
			} else {
				tmp = au->au_author + strlen(au->au_author) - 1;
			}
keepgoing:
			while ((unsigned int)tmp > (unsigned int)au->au_author) {
				if (*tmp == ' ') {

					/*
					 * Special case names
					 */
					if ((strcmp(tmp, " Guin") == 0) && strstr(au->au_author, "Le Guin") ) {
						tmp--;
						goto keepgoing;
					} else if ((strcmp(tmp, " Lint") == 0) && strstr(au->au_author, "de Lint") ) {
						tmp--;
						goto keepgoing;
					} else if ((strcmp(tmp, " Haven") == 0) && strstr(au->au_author, "De Haven") ) {
						tmp--;
						goto keepgoing;
					} else if (strcmp(tmp, " Jr.") == 0) {
						tmp--;
						goto keepgoing;
					} else {
						tmp++;
						break;
					}
				}
				tmp--;
			}
			strcpy(au->au_lastname, tmp);

			au->au_yob   = 0;
			au->au_noms  = 0;
			au->au_first = 0;
			au->au_last  = 0;
			au->au_peak  = 0;
			au->au_wins  = 0;
			au->au_list  = NULL;
			au->au_points= 0;
			insert_author(au);
		}

		/*
		 * Traverse awards, and update author stats
		 */
		aw = rec->rec_awards;
		while(aw) {
			au->au_noms++;
			if (aw->aw_level == 1)
				au->au_wins++;
			aw = aw->aw_next;
		}
		au->au_points += rec->rec_points;

		rec->rec_aulink = au->au_list;
		au->au_list = rec;
		rec = rec->rec_next;
	}
}


int
main(argc,argv)
	int	argc;
	char *argv[];
{
	FILE	*fp;
	int	option;
	int	print_style = 0;

	while ( (option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {
		case 'a':	(void)strcpy(filter_author, optarg);
				print_style = PRINT_BY_AUTH;
				break;

		case 'A':	print_style = PRINT_BY_AUTH;
				break;

		case 'b':       filter_dont_own = 1;
				print_style = PRINT_RAW;
				break;

		case 'C':	critical_mode = 1;
				break;

		case 'g':	if (strcmp(optarg,"hu") == 0) {
					print_style = PRINT_HUGOS;
					filter_hugos = 1;
				} else if (strcmp(optarg, "ca") == 0) {
					print_style = PRINT_CAMPBELL;
				} else if (strcmp(optarg, "cl") == 0) {
					print_style = PRINT_CLARKE;
				} else if (strcmp(optarg, "ne") == 0) {
					print_style = PRINT_NEBULAS;
				} else if (strcmp(optarg, "ls") == 0) {
					print_style = PRINT_LOCUS_SF;
				} else if (strcmp(optarg, "lf") == 0) {
					print_style = PRINT_LOCUS_FANT;
				} else if (strcmp(optarg, "lh") == 0) {
					print_style = PRINT_LOCUS_HOR;
				} else if (strcmp(optarg, "st") == 0) {
					print_style = PRINT_STOKER;
				} else if (strcmp(optarg, "pk") == 0) {
					print_style = PRINT_PKDICK;
				} else if (strcmp(optarg, "wf") == 0) {
					print_style = PRINT_WFA;
				} else {
					printf("Bad generator code\n");
					exit(1);
				}
				break;

		case 'l':       print_style = PRINT_RAW;
				break;

		case 'o':       filter_own = 1;
				print_style = PRINT_RAW;
				break;

		case 'p':       print_style = PRINT_ENTRIES;
				break;

		case 'P':	popular_mode = 1;
				break;

		case 'r':       filter_read = 1;
				print_style = PRINT_RAW;
				break;

		case 'R':       filter_did_read = 1;
				print_style = PRINT_RAW;
				break;

		case 's': 	print_style = PRINT_TOP_AUTHORS;
				break;

		case 't':	(void)strcpy(filter_title, optarg);
				print_style = PRINT_RAW;
				break;

		case 'T':       (void)sscanf(optarg,"%d", &filter_throttle);
				break;

		case 'w':       filter_winners_only = 1;
				break;

		case 'y':       (void)sscanf(optarg,"%d", &filter_year);
				print_style = PRINT_BY_YEAR;
				break;
		}
	}

	fp = freopen(DBASE, "r", stdin);
	if (fp == NULL) {
		fprintf(stderr, "Can't open %s\n", DBASE);
		exit(1);
	}

	yyparse();
	create_aulist();

	switch(print_style) {
	case 0:			print_by_year();
				break;

	case PRINT_BY_AUTH:	print_by_author2();
				break;

	case PRINT_BY_YEAR:	print_by_year();
				break;

	case PRINT_RAW:		print_raw();
				break;

	case PRINT_CAMPBELL:	print_list(AW_CAMPBELL);
				break;

	case PRINT_CLARKE:	print_list(AW_CLARKE);
				break;

	case PRINT_HUGOS:	print_list(AW_HUGO);
				break;

	case PRINT_NEBULAS:	print_list(AW_NEBULA);
				break;

	case PRINT_LOCUS_SF:	print_list(AW_LOCUS_SF);
				break;

	case PRINT_LOCUS_FANT:	print_list(AW_LOCUS_FANT);
				break;

	case PRINT_LOCUS_HOR:	print_list(AW_LOCUS_HOR);
				break;

	case PRINT_STOKER:	print_list(AW_STOKER);
				break;

	case PRINT_PKDICK:	print_list(AW_PKDICK);
				break;

	case PRINT_WFA:		print_list(AW_WFA);
				break;

	case PRINT_ENTRIES:	print_entries();
				break;

	case PRINT_TOP_AUTHORS:	print_top_authors();
				break;
	}

	(void)fclose(fp);
        return(0);
}
