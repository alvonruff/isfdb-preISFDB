/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void
ascii_to_rec(char *title, char *author, char *year, char *type)
{
	char	*ptr;
	char	*string_brk;
	char	return_title[96];
	char	return_author[48];
	char	return_year[8];
	char	return_type[8];

	strcpy(return_title, title);
	for (ptr=return_title; *ptr; ptr++) {
		if (*ptr == '_')
			*ptr = ' ';
	}

	strcpy(return_author, author);
	for (ptr=return_author; *ptr; ptr++) {
		if (*ptr == '_')
			*ptr = ' ';
	}

	strcpy(return_year, year);
	strcpy(return_type, type);

	printf("%s|%s|%s|%s\n", 
		return_title,
		return_author,
		return_year,
		return_type);
}


int
main(argc, argv)
	int	argc;
	char	*argv[];
{
	int		loop;

	printf("Content-type: text/html%c%c",10,10);

#ifdef REMOVE
	printf("<pre>\n");
	for(loop=0; loop<argc; loop++) {
		printf("%s\n", argv[loop]);
	}
	printf("</pre>\n");

	if (argc != 5) {
		printf("Bad input value\n");
		exit(1);
	}
#endif

	ascii_to_rec( argv[1], argv[2], argv[3], argv[4] );
        exit(0);
        return(0);
}
