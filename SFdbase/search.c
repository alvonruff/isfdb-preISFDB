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
#include <ctype.h>

#define OPTARGS "t:a:y:?"

char	tmptitle[256];
char	tmpauthor[256];
char	tmpyear[256];
char	tmptitle2[256];
char	tmpauthor2[256];
char	tmpyear2[256];
char	filter_author[256]	= {0};
char	filter_title[256]	= {0};
char	filter_year[256]	= {0};

extern char *optarg;

typedef struct search {
	char		*se_title;
	char		*se_author;
	struct search	*se_next;
} search_t;
search_t	*title_list  = NULL;
search_t	*author_list = NULL;

static void
usage()
{
	printf("novels [-t title] [-a author] [-y year]\n");
	exit(1);
}


static void
add_author(char *title, char *author)
{
	search_t	*tmp;

	tmp = author_list;
	while(tmp) {
		if ( strcmp(author, tmp->se_author) == 0 ) {
			return;
		}
		tmp = tmp->se_next;
	}

	tmp = (search_t *)malloc( sizeof(search_t) );
	if ( tmp == NULL ) {
		perror("out of memory");
		exit(1);
	}
	tmp->se_title  = (char *)malloc( strlen(title)+1 );
	tmp->se_author = (char *)malloc( strlen(author)+1 );
	if ( (tmp->se_title == NULL ) || ( tmp->se_title == NULL )) {
		perror("out of memory");
		exit(1);
	}

	strcpy(tmp->se_title, title);
	strcpy(tmp->se_author, author);

	tmp->se_next = author_list;
	author_list = tmp;
}


static void
add_title(char *title, char *author)
{
	search_t	*tmp;

	tmp = title_list;
	while(tmp) {
		if ( strcmp(title, tmp->se_title) == 0 ) {
			return;
		}
		tmp = tmp->se_next;
	}

	tmp = (search_t *)malloc( sizeof(search_t) );
	if ( tmp == NULL ) {
		perror("out of memory");
		exit(1);
	}
	tmp->se_title  = (char *)malloc( strlen(title)+1 );
	tmp->se_author = (char *)malloc( strlen(author)+1 );
	if ( (tmp->se_title == NULL ) || ( tmp->se_title == NULL )) {
		perror("out of memory");
		exit(1);
	}

	strcpy(tmp->se_title, title);
	strcpy(tmp->se_author, author);

	tmp->se_next = title_list;
	title_list = tmp;
}


static void
search_file(char *filename)
{
	FILE	*fp;
	int	line_number = 1;
	int	index;
	char	input;

	fp = fopen(filename, "r+");
	if (fp == NULL) {
		perror("Couldn't open shortfiction.dbase");
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
		 * Read to End-Of-Line
		 */
		input = 0;
		while( input != '\n') {
			input = getc(fp);
			if (input == -1)
				goto finish;
		}
		line_number++;

		if (filter_title[0]) {
			index = 0;
			while( tmptitle[index] != 0 ) {
				tmptitle2[index] = tolower( tmptitle[index] );
				index++;
			}
			tmptitle2[index] = 0;
			if (strstr(tmptitle2, filter_title)) {
				add_title(tmptitle, tmpauthor);
				continue;
			}
		} 
		if (filter_author[0]) {
			char	*origauth;
			char	*ptr;

			origauth = tmpauthor;
			while( strstr(origauth, " and ") ) {

				/*
				 * If the work was by more than one author,
				 * check each author as a target.
				 */
				index = 0;
				ptr = (char *)strstr(origauth, " and ");
				*ptr = 0;
				while( origauth[index] != 0 ) {
					tmpauthor2[index] = tolower( origauth[index] );
					index++;
				}
				tmpauthor2[index] = 0;
				if (strstr(tmpauthor2, filter_author)) {
					add_author(tmptitle, origauth);
				}
				origauth = ptr + 5;
			}
			index = 0;
			while( origauth[index] != 0 ) {
				tmpauthor2[index] = tolower( origauth[index] );
				index++;
			}
			tmpauthor2[index] = 0;
			if (strstr(tmpauthor2, filter_author)) {
				add_author(tmptitle, origauth);
			}
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
	int	option;
	int	index;

	while ( (option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {
		case 'a':	(void)strcpy(filter_author, optarg);
				index = 0;
				while( filter_author[index] != 0 ) {
					filter_author[index] = tolower( filter_author[index] );
					index++;
				}
				break;

		case 't':	(void)strcpy(filter_title, optarg);
				index = 0;
				while( filter_title[index] != 0 ) {
					filter_title[index] = tolower( filter_title[index] );
					index++;
				}
				break;

		case 'y':	(void)strcpy(filter_year, optarg);
				break;

		case '?':	usage();
				break;
		}
	}

	search_file("dbases/novels.dbase");
	search_file("dbases/shortfiction.dbase");

	if (filter_title[0]) {
		search_t *tmp;

		tmp = title_list;
		while(tmp) {
			printf("%s\t%s\n", tmp->se_title, tmp->se_author);
			tmp = tmp->se_next;
		}
	}
	if (filter_author[0]) {
		search_t *tmp;

		tmp = author_list;
		while(tmp) {
			printf("%s\n", tmp->se_author);
			tmp = tmp->se_next;
		}
	}
        exit(0);
        return(0);
}
