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

#define DBASE "dbases/books.dbase"
#define OPTARGS "AbClopPrRswy:t:a:c:g:T:?"

#define MAX_TITLE_WIDTH	35
#define MAX_AUTHOR_WIDTH 20
#define CURRENT_YEAR    1995

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

typedef struct award {
	int	aw_award;
	int	aw_year;
	int	aw_category;
	int	aw_level;
	struct	award *aw_next;
} award_t;


typedef struct list {
	char	li_type[5];
	int	li_numb;
} list_t;

typedef struct record {
	char	rec_author[256];
	char	rec_title[256];
	int	rec_year;
	int	rec_winner;
	int	rec_points;
	int	rec_ownit;
	int	rec_readit;
	award_t	*rec_awards;
	struct	record *rec_next;
	struct	record *rec_prev;
	struct	record *rec_aulink;
} record_t;

typedef struct author {
	char	au_author[256];
	char	au_lastname[256];
	int	au_yob;
	int	au_noms;
	int	au_wins;
	int	au_first;
	int	au_last;
	int	au_peak;
	int	au_points;
	struct author *au_prev;
	struct author *au_next;
	struct record *au_list;
} author_t;

/*
 * Currently support award types
 */
#define AW_HUGO		1
#define AW_NEBULA	2
#define AW_LOCUS_SF	3
#define AW_LOCUS_HOR	4
#define AW_LOCUS_FANT	5
#define AW_CAMPBELL	6
#define AW_STOKER	7
#define AW_CLARKE	8
#define AW_PKDICK	9
#define AW_WFA		10
#define AW_MAXTYPES	10

#define AW_NOVEL		1
#define AW_NOVELLA		2
#define AW_NOVELETTE		3
#define AW_SHORT_STORY		4
#define AW_SHORT_FICTION	5
#define AW_FIRST_NOVEL		6
#define AW_MAXCATS		7

#define PRINT_BY_AUTH		1
#define PRINT_BY_YEAR		2
#define PRINT_RAW		3
#define PRINT_HUGOS		4
#define PRINT_NEBULAS		5
#define PRINT_LOCUS_SF		6
#define PRINT_LOCUS_HOR		7
#define PRINT_LOCUS_FANT	8
#define PRINT_ENTRIES		9
#define PRINT_CAMPBELL		10
#define PRINT_STOKER		11
#define PRINT_CLARKE		12
#define PRINT_PKDICK		13
#define PRINT_WFA		14
#define PRINT_TOP_AUTHORS	15

static void print_by_year();
static void print_top_authors();
static void create_aulist();
static void print_list2(int award);
