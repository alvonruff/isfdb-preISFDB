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
char	filter_author[256]	= {0};
char	filter_title[256]	= {0};
char	filter_year[256]	= {0};

extern char *optarg;

static void
usage()
{
	printf("novels [-t title] [-a author] [-y year]\n");
	exit(1);
}


int
main(argc, argv)
	int	argc;
	char	*argv[];
{
	FILE	*fp;
	int	line_number = 1;
	int	option;

	while ( (option = getopt(argc, argv, OPTARGS)) != -1) {
		switch(option) {
		case 'a':	(void)strcpy(filter_author, optarg);
				break;

		case 't':	(void)strcpy(filter_title, optarg);
				break;

		case 'y':	(void)strcpy(filter_year, optarg);
				break;

		case '?':	usage();
				break;
		}
	}

	fp = fopen("dbases/novels.dbase", "r+");
	if (fp == NULL) {
		perror("Couldn't open novels.dbase");
		exit(1);
	}

	while(1) {
		char	input;
		int	index;

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

		if (filter_title[0] && (!strstr(tmptitle, filter_title)) ) {
			continue;
		} 
		if (filter_author[0] && (!strstr(tmpauthor, filter_author)) ) {
			continue;
		} 
		if (filter_year[0] && (!strstr(tmpyear, filter_year)) ) {
			continue;
		} 
		printf("%s\t%s\t%s\n", tmptitle, tmpauthor, tmpyear);
	}

finish:

	fclose(fp);
        exit(0);
        return(0);
}
