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
char	filter_title[96]  = {0};
char	filter_author[48] = {0};
char	filter_year[8]	  = {0};
char	filter_type[8]	  = {0};

extern char	*optarg;
int	errno;

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
	struct search	*se_next;
} search_t;
search_t	*title_list  = NULL;
search_t	*series_list = NULL;
search_t	*novel_list;
search_t	*short_list;

static void add_series(char *title, char *author, char *year, char *series);

static void
usage()
{
	printf("authors -a author\n");
	exit(1);
}


static void
ascii_to_rec(char *author)
{
	char	*ptr;
	char	*string_brk;

	strcpy(filter_author, author);
	for (ptr=filter_author; *ptr; ptr++) {
		if (*ptr == '_')
			*ptr = ' ';
	}
}


static void
print_series(search_t **oldlist )
{
	search_t	*tmp;
	search_t	*tmp2;
	search_t	*tmp3;
	char		*current_series = NULL;
	search_t	*newlist = NULL;

	while (*oldlist) {
		tmp = *oldlist;
		tmp2 = NULL;
		if (current_series)
			printf("\n");
		current_series = tmp->se_series;
		printf("        %s\n", current_series);
		while(tmp) {
			if ( strcmp(tmp->se_series, current_series) == 0) {
				if (tmp2 == NULL) {
					*oldlist = tmp->se_next;
				} else {
					tmp2->se_next = tmp->se_next;
				}
				tmp3 = tmp->se_next;
				tmp->se_next = newlist;
				newlist = tmp;
				printf("                %s (%s)\n", 
					tmp->se_title, tmp->se_year);
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
print_novels(search_t *list, search_t *series_list )
{
	search_t	*tmp;
	search_t	*tmp2;
	int		printit;

	tmp = list;
	while( tmp ) {
		printit = 1;
		tmp2 = series_list;
		while (tmp2) {
			if ( strcmp(tmp->se_title, tmp2->se_title) == 0) {
				printit = 0;
			}
			tmp2 = tmp2->se_next;
		}
		if (printit) {
			if ( strcmp(tmp->se_author, filter_author) == 0) {
				printf("%s (%s)", tmp->se_title, tmp->se_year);
			} else {
				printf("%s (%s)  [%s]", tmp->se_title, 
					tmp->se_year, tmp->se_author);
			}
			if ( strcmp(tmp->se_type, "c ") == 0) {
				printf(" [C]\n");
			} else {
				printf("\n");
			}
		}
		tmp = tmp->se_next;
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
get_series(char *author)
{
	FILE	*fp;
	int	line_number = 1;
	int	index;
	char	input;
	author_t	*tmp;

	/*
	 * Book | Author | Year | Series
	 */
	fp = fopen("dbases/series.dbase", "r");
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
			add_series(tmptitle, tmpauthor, tmpyear, tmpseries);
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
	fp = fopen("dbases/authors.dbase", "r");
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
add_series(char *title, char *author, char *year, char *series)
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
	strcpy(tmp->se_series, series);

	tmp->se_next = series_list;
	series_list = tmp;
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

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("Couldn't open dbase [%s]. Errno = %d", filename, errno);
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
	int		result;
	search_t	*tmp;
	author_t	*auth;
	char		path[256];

	printf("Content-type: text/html%c%c",10,10);

	if (argc != 2) {
		printf("Bad author input\n");	
		exit(1);
	}

	printf("<pre>");
	ascii_to_rec( argv[1] );

	search_file("dbases/novels.dbase", "n ");
	search_file("dbases/collections.dbase", "c ");
	novel_list = title_list;
	title_list = NULL;

	search_file("dbases/shortfiction.dbase", "sf");
	short_list = title_list;
	title_list = NULL;


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

	get_series(filter_author);
	if ( series_list ) {
		sort_search_list( &series_list );
		printf("\nSeries:\n");
		print_series( &series_list );
	}

	if ( novel_list ) {
		sort_search_list( &novel_list );
		printf("\nNovels:\n");
		print_novels(novel_list, series_list );
	}

	if ( short_list ) {
		sort_search_list( &short_list );
		printf("\nShort Fiction:\n");
		tmp = short_list;
		while(tmp) {
			printf("%s (%s)\n", tmp->se_title, tmp->se_year);
			tmp = tmp->se_next;
		}
	}
	printf("</pre>");
        return(0);
}
