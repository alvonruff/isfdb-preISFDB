/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OPTARGS "t:a:y:?"

char	tmptitle[256];
char	tmpauthor[256];
char	tmpyear[256];
char	tmptitle2[256];
char	tmpauthor2[256];
char	tmpseries[256];
char	tmpyear2[256];
char	filter_author[256]	= {0};
char	filter_year[256]	= {0};

extern char *optarg;

typedef struct author {
	char	au_legalname[256];
	char	au_birthplace[32];
	char	au_birthdate[16];
	char	au_deathdate[16];
} author_t;

typedef struct search {
	char		se_title[256];
	char		se_author[256];
	char		se_series[256];
	char		se_year[12];
	char		se_type[12];
	int		se_numyear;
	int		se_marker;
	struct search	*se_next;
} search_t;
search_t	*title_list  = NULL;
search_t	*series_list = NULL;
search_t	*novel_list;
search_t	*short_list;

static void
usage()
{
	printf("authors -a author\n");
	exit(1);
}

static void
clear_markers(search_t *list)
{
	search_t	*tmp;

	tmp = list;
	while( tmp ) {
		tmp->se_marker = 0;
		tmp = tmp->se_next;
	}
}

static void
print1_novel(search_t *novel)
{
	if ( strcmp(novel->se_author, filter_author) == 0) {
		printf("%s (%s)", novel->se_title, novel->se_year);
	} else {
		printf("%s (%s)  [%s]", novel->se_title, 
			novel->se_year, novel->se_author);
	}
	if ( strcmp(novel->se_type, "c ") == 0) {
		printf(" [C]\n");
	} else {
		printf("\n");
	}
}

static void
print_series(search_t *list )
{
	search_t	*tmp;
	char		*current_series;
	int		foundseries = 1;
	int		noseriesyet = 1;

	while( foundseries ) {
		foundseries = 0;
		tmp = list;
		current_series = NULL;
		while (tmp) {
			if ( tmp->se_marker ) {
				tmp = tmp->se_next;
				continue;
			}
			if ( strcmp(tmp->se_type, "sf") == 0 ) {
				tmp = tmp->se_next;
				continue;
			}
			if (tmp->se_series[0] && (current_series == NULL)) {
				current_series = tmp->se_series;
				if (noseriesyet) {
					noseriesyet = 0;
					printf("\nSeries:\n        %s\n", current_series);
				} else {
					printf("\n        %s\n", current_series);
				}
			}
			if ( tmp->se_series[0] && strcmp(tmp->se_series, current_series) == 0) {
				printf("                ");
				print1_novel(tmp);
				foundseries = 1;
				tmp->se_marker = 1;
			}
			tmp = tmp->se_next;
		}
	}
}


static void
print_novels(search_t *list)
{
	search_t	*tmp;
	int		firsttime = 1;

	tmp = list;
	while( tmp ) {
		if ( tmp->se_marker ) {
			tmp = tmp->se_next;
			continue;
		}
		if ( strcmp(tmp->se_type, "sf") == 0 ) {
			tmp = tmp->se_next;
			continue;
		}
		if ( strcmp(tmp->se_author, filter_author) == 0) {
			if (firsttime) {
				firsttime = 0;
				printf("\nNovels:\n");
			}
			tmp->se_marker = 1;
			print1_novel(tmp);
		}
		tmp = tmp->se_next;
	}
}


static void
print_with(search_t *list )
{
	search_t	*tmp;
	char		*ptr;
	char		current_with[80];
	int		foundwith = 1;
	int		noseriesyet = 1;
	char		newauthor[80];

	while( foundwith ) {
		foundwith = 0;
		tmp = list;
		current_with[0] = 0;
		while (tmp) {

			if ( tmp->se_marker ) {
				tmp = tmp->se_next;
				continue;
			}

			if ( strcmp(tmp->se_type, "sf") == 0) {
				tmp = tmp->se_next;
				continue;
			}

			/*
			 * Figure out who the other author is and
			 * point ptr at it.
			 */
			strcpy(newauthor, tmp->se_author);
			ptr = (char *)strstr(newauthor, " and ");
			if( ptr == NULL) {
				tmp->se_marker = 1;
				tmp = tmp->se_next;
				continue;
			}
			*ptr = 0;
			ptr += 5;
			if ( strcmp(ptr, filter_author) == 0 ) {
				ptr = newauthor;
			}

			if ( current_with[0] == 0 ) {
				strcpy(current_with, ptr);
				printf("\nNovels With %s:\n", ptr);
			}
			if ( strcmp(ptr, current_with) == 0) {
				printf("%s (%s)\n", tmp->se_title, tmp->se_year);
				foundwith = 1;
				tmp->se_marker = 1;
			}
			tmp = tmp->se_next;
		}
	}
}

static void
print_shortfiction(search_t *list)
{
	search_t	*tmp;
	int		firsttime = 1;

	tmp = list;
	while( tmp ) {
		if ( tmp->se_marker ) {
			tmp = tmp->se_next;
			continue;
		}
		if ( strcmp(tmp->se_type, "sf") ) {
			tmp = tmp->se_next;
			continue;
		}
		if ( strcmp(tmp->se_author, filter_author) == 0) {
			if (firsttime) {
				firsttime = 0;
				printf("\nShort Fiction:\n");
			}
			tmp->se_marker = 1;
			printf("%s (%s)\n", tmp->se_title, tmp->se_year);
		}
		tmp = tmp->se_next;
	}
}


static void
print_shortwith(search_t *list )
{
	search_t	*tmp;
	char		*ptr;
	char		current_with[80];
	int		foundwith = 1;
	int		noseriesyet = 1;
	char		newauthor[80];

	while( foundwith ) {
		foundwith = 0;
		tmp = list;
		current_with[0] = 0;
		while (tmp) {

			if ( tmp->se_marker ) {
				tmp = tmp->se_next;
				continue;
			}

			if ( strcmp(tmp->se_type, "sf") ) {
				tmp = tmp->se_next;
				continue;
			}

			/*
			 * Figure out who the other author is and
			 * point ptr at it.
			 */
			strcpy(newauthor, tmp->se_author);
			ptr = (char *)strstr(newauthor, " and ");
			if( ptr == NULL) {
				tmp->se_marker = 1;
				tmp = tmp->se_next;
				continue;
			}
			*ptr = 0;
			ptr += 5;
			if ( strcmp(ptr, filter_author) == 0 ) {
				ptr = newauthor;
			}

			if ( current_with[0] == 0 ) {
				strcpy(current_with, ptr);
				printf("\nShort Fiction With %s:\n", ptr);
			}
			if ( strcmp(ptr, current_with) == 0) {
				printf("%s (%s)\n", tmp->se_title, tmp->se_year);
				foundwith = 1;
				tmp->se_marker = 1;
			}
			tmp = tmp->se_next;
		}
	}
}


static void
sort_search_list(search_t **oldlist)
{
	search_t	*tmp;
	search_t	*tmp2;
	search_t	*tmp3;
	int		minyear = 3000;
	int		maxyear = 0;
	int		loop;
	search_t	*newlist = NULL;

	tmp = *oldlist;
	while(tmp) {
		sscanf(tmp->se_year, "%d", &(tmp->se_numyear) );
		if ( tmp->se_numyear <= minyear)
			minyear = tmp->se_numyear;
		if ( tmp->se_numyear >= maxyear)
			maxyear = tmp->se_numyear;
		tmp = tmp->se_next;
	}

	for(loop=maxyear; loop>=minyear; loop--) {
		tmp = *oldlist;
		tmp2 = NULL;
		while(tmp) {
			if (tmp->se_numyear == loop) {
				if (tmp2 == NULL) {
					*oldlist = tmp->se_next;
				} else {
					tmp2->se_next = tmp->se_next;
				}
				tmp3 = tmp->se_next;
				tmp->se_next = newlist;
				newlist = tmp;
				tmp = tmp3;
			} else {
				tmp2 = tmp;
				tmp = tmp->se_next;
			}
		}
	}
	*oldlist = newlist;
}


static void
get_series(char *author,  search_t *list)
{
	FILE	*fp;
	int	line_number = 1;
	int	index;
	char	input;
	author_t	*tmp;

	/*
	 * Book | Author | Year | Series
	 */
	fp = fopen("dbases/series.dbase", "r+");
	if (fp == NULL) {
		perror("Couldn't open dbase");
		exit(1);
	}

	while(1) {
		/*
		 * Read in the Title
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmptitle[index++] = input;
			if (index > 255) {
				printf("Title size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmptitle[index-1] = 0;

		/*
		 * Read in the Author
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpauthor[index++] = input;
			if (index > 255) {
				printf("Author size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpauthor[index-1] = 0;

		/*
		 * Read in the Year
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpyear[index++] = input;
			if (index > 255) {
				printf("Year size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpyear[index-1] = 0;

		/*
		 * Read in the Series
		 */
		input = 0;
		index = 0;
		while( (input != '|') && (input != '\n') ) {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpseries[index++] = input;
			if (index > 255) {
				printf("Series size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpseries[index-1] = 0;

		/*
		 * Read to End-Of-Line
		 */
		while( input != '\n') {
			input = getc(fp);
			if (input == -1)
				goto finish;
		}
		line_number++;
		if (strstr(tmpauthor, author)) {
			search_t	*tmp;

			tmp = list;
			while(tmp) {
				if ( (strcmp(tmptitle, tmp->se_title) == 0) &&
				     (strcmp(tmpauthor, tmp->se_author) == 0) &&
				     (strcmp(tmpyear, tmp->se_year) == 0) ) {
					strcpy(tmp->se_series, tmpseries);
				}
				tmp = tmp->se_next;
			}
		}
	}

finish:
	fclose(fp);
}

author_t *
get_author(char *author)
{
	FILE	*fp;
	int	line_number = 1;
	int	index;
	char	input;
	author_t	*tmp;

	/*
	 * Writing Name | Actual Name | BirthPlace | birthdate m/d/y | deathdate m/d/y |
	 */
	fp = fopen("dbases/authors.dbase", "r+");
	if (fp == NULL) {
		perror("Couldn't open dbase");
		exit(1);
	}

	while(1) {

		/*
		 * Read in the Author Name
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpauthor[index++] = input;
			if (index > 255) {
				printf("Title size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpauthor[index-1] = 0;
		if ( strcmp(tmpauthor, author) == 0 ) {
			tmp = (author_t *)malloc( sizeof(author_t) );

			/*
			 * Read in Legal Name
			 */
			input = 0;
			index = 0;
			while( input != '|') {
				input = getc(fp);
				if (input == -1)
					goto finish;
				tmp->au_legalname[index++] = input;
				if (index > 255) {
					printf("Legal name exceeds 256 characters on line %d\n", line_number);
					exit(1);
				}
			}
			tmp->au_legalname[index-1] = 0;

			/*
			 * Read in Birthplace
			 */
			input = 0;
			index = 0;
			while( input != '|') {
				input = getc(fp);
				if (input == -1)
					goto finish;
				tmp->au_birthplace[index++] = input;
				if (index > 31) {
					printf("Birthplace exceeds 32 characters on line %d\n", line_number);
					exit(1);
				}
			}
			tmp->au_birthplace[index-1] = 0;

			/*
			 * Read in Birthdate
			 */
			input = 0;
			index = 0;
			while( input != '|') {
				input = getc(fp);
				if (input == -1)
					goto finish;
				tmp->au_birthdate[index++] = input;
				if (index > 15) {
					printf("Birthplace exceeds 16 characters on line %d\n", line_number);
					exit(1);
				}
			}
			tmp->au_birthdate[index-1] = 0;

			/*
			 * Read in Deathdate
			 */
			input = 0;
			index = 0;
			while( (input != '|') && (input != '\n') ) {
				input = getc(fp);
				if (input == -1)
					goto finish;
				tmp->au_deathdate[index++] = input;
				if (index > 15) {
					printf("Birthplace exceeds 16 characters on line %d\n", line_number);
					exit(1);
				}
			}
			tmp->au_deathdate[index-1] = 0;
			fclose(fp);
			return(tmp);
		} else {
			/*
			 * Wrong author - read to EOL
			 */
			while( input != '\n') {
				input = getc(fp);
				if (input == -1)
					goto finish;
			}
		}
	}

finish:
	fclose(fp);
	return(NULL);
}


static void
add_title(char *title, char *author, char *year, char *exten)
{
	search_t	*tmp;

	tmp = (search_t *)malloc( sizeof(search_t) );
	if ( tmp == NULL ) {
		perror("out of memory");
		exit(1);
	}

	strcpy(tmp->se_title, title);
	strcpy(tmp->se_author, author);
	strcpy(tmp->se_year, year);
	strcpy(tmp->se_type, exten);
	tmp->se_series[0] = 0;

	tmp->se_next = title_list;
	title_list = tmp;
}


static void
search_file(char *filename, char *exten)
{
	FILE	*fp;
	int	line_number = 1;
	int	index;
	char	input;

	fp = fopen(filename, "r+");
	if (fp == NULL) {
		perror("Couldn't open dbase");
		exit(1);
	}

	while(1) {
		/*
		 * Read in the Title
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmptitle[index++] = input;
			if (index > 255) {
				printf("Title size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmptitle[index-1] = 0;

		/*
		 * Read in the Author
		 */
		input = 0;
		index = 0;
		while( input != '|') {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpauthor[index++] = input;
			if (index > 255) {
				printf("Author size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpauthor[index-1] = 0;

		/*
		 * Read in the Year
		 */
		input = 0;
		index = 0;
		while( (input != '|') && (input != '\n') ) {
			input = getc(fp);
			if (input == -1)
				goto finish;
			tmpyear[index++] = input;
			if (index > 255) {
				printf("Year size exceeds 256 characters on line %d\n", line_number);
				exit(1);
			}
		}
		tmpyear[index-1] = 0;

		/*
		 * Read to End-Of-Line
		 */
		while( input != '\n') {
			input = getc(fp);
			if (input == -1)
				goto finish;
		}
		line_number++;
		if (strstr(tmpauthor, filter_author)) {
			add_title(tmptitle, tmpauthor, tmpyear, exten);
		}
	}

finish:
	fclose(fp);
}


int
main(argc, argv)
	int	argc;
	char	*argv[];
{
	int		option;
	int		index;
	search_t	*tmp;
	author_t	*auth;

	while ( (option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {
		case 'a':	(void)strcpy(filter_author, optarg);
				break;

		case '?':	usage();
				break;
		}
	}

	search_file("dbases/novels.dbase", "n ");
	search_file("dbases/collections.dbase", "c ");
	search_file("dbases/shortfiction.dbase", "sf");
	sort_search_list( &title_list );
	get_series(filter_author, title_list);
	clear_markers(title_list);

	auth = get_author(filter_author);
	if (auth) {
		printf("\n%s     (%s, %s-%s)\n",
			auth->au_legalname,
			auth->au_birthplace,
			auth->au_birthdate,
			auth->au_deathdate);
	} else {
		printf("\n%s\n", filter_author);
	}

	print_series( title_list );

	print_novels(title_list);
	print_with(title_list);
	print_shortfiction(title_list);
	print_shortwith(title_list);
        exit(0);
        return(0);
}
