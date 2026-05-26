%{
#ident "%W%	%G% %Q%"
/*
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include <stdio.h>
#include "books.h"

extern char *yytext;
extern char read_name[];
extern int  read_digit;

#define E_TITLE		1
#define E_BAR		2
#define E_AUTHOR	3
#define E_YEAR		4
#define E_AWARDTYPE	5
#define E_AWARDNUM	6
#define E_AWARDCAT	7
#define E_AWARDYEAR	8
#define E_LBRACKET	9
#define E_RBRACKET	10
#define E_UNDER		11

int	line_number	= 1;		/* Current line number			*/
int	y_error		= E_TITLE;

char	current_title[256];
char	current_author[256];
int	current_year;
int	current_own;
int	current_read;

char	current_awardtype[5];	/* Like hu, ne, etc...		*/
int	current_awardnum;	/* 1= win, etc...		*/
char	current_awardcat[5];	/* n, nv, nt, ss, etc...	*/
int	current_awardyear;	/* 1994, duh ...		*/

%}

%token T_BAR T_STRING T_INT T_STUFF T_ASCII T_UNDER T_LBRACKET T_RBRACKET

%%
books		: books 	{ y_error = E_TITLE; }
		  book
		| 
		;

book		: b_title 	{ y_error = E_BAR; }
		  T_BAR 	{ y_error = E_AUTHOR; }
		  b_author 	{ y_error = E_BAR; }
		  T_BAR 	{ y_error = E_YEAR; }
		  b_pubyear 	{ y_error = E_BAR; }
		  T_BAR
		  b_awards 	{ y_error = E_BAR; }
		  T_BAR 
		  b_own 	{ y_error = E_BAR; }
		  T_BAR 
		  b_read	{ insert_record(current_title, current_author, 
					current_year, current_own, current_read); }
		;

b_title		: T_STRING	{ $$ = (int)push_string(current_title); }
		| T_INT		{ sprintf(read_name, "%d", read_digit); 
				  $$ = (int)push_string(current_title); }
		| T_ASCII	{ $$ = (int)push_string(current_title); }
		;

b_author	: T_STRING	{ $$ = (int)push_string(current_author); }
		| T_ASCII	{ $$ = (int)push_string(current_author); }
		;

b_pubyear	: T_INT		{ $$ = current_year = read_digit; }
		;

b_awards	: b_awards 	{ y_error = E_LBRACKET; }
		  b_award
		| /* empty */
		;

b_award		: T_LBRACKET	{ y_error = E_AWARDTYPE; }
		  T_ASCII 	{ y_error = E_UNDER; 
				  push_string(current_awardtype); }
		  T_UNDER	{ y_error = E_AWARDNUM; }
		  T_INT		{ y_error = E_UNDER; 
				  current_awardnum = read_digit; }
		  T_UNDER	{ y_error = E_AWARDCAT; }
		  T_ASCII	{ y_error = E_UNDER; 
				  push_string(current_awardcat); }
		  T_UNDER	{ y_error = E_AWARDYEAR; }
		  T_INT		{ y_error = E_RBRACKET; 
				  current_awardyear = read_digit; }
		  T_RBRACKET	{ insert_award( current_awardtype, current_awardnum, 
					current_awardcat, current_awardyear);
				}
		;

b_own		: T_ASCII	{ if (read_name[0] == 'y') {
					current_own = 1;
				  } else {
					current_own = 0;
				  }
				}
		;

b_read		: T_ASCII	{ if (read_name[0] == 'y') {
					current_read = 1;
				  } else {
					current_read = 0;
				  }
				}
		;
%%

yyerror(s)
char *s;
{
	fprintf(stderr,"\nError in configuration file, line %d: ", line_number);

	switch(y_error)
	{
	case E_TITLE    : fprintf(stderr,"Bad title string\n");
			  break;

	case E_BAR      : fprintf(stderr,"\'|\' expected\n");
			  break;

	case E_LBRACKET      : fprintf(stderr,"\'[\' expected\n");
			  break;

	case E_RBRACKET      : fprintf(stderr,"\']\' expected\n");
			  break;

	case E_UNDER      : fprintf(stderr,"\'_\' expected\n");
			  break;

	case E_AUTHOR   : fprintf(stderr,"Bad author string\n");
			  break;

	case E_YEAR     : fprintf(stderr,"Bad year\n");
			  break;

	case E_AWARDTYPE : fprintf(stderr,"Bad award type\n");
			  break;

	case E_AWARDNUM : fprintf(stderr,"Bad award number\n");
			  break;

	case E_AWARDCAT : fprintf(stderr,"Bad award category\n");
			  break;

	case E_AWARDYEAR : fprintf(stderr,"Bad award year\n");
			  break;

	default		: fprintf(stderr,"???\n");
	}

	exit(1);
}

yywrap()
{
	return (1);
}

#ifdef ORIG
char *
#else
int
#endif
push_string(char *target)
{
        strcpy(target, read_name);
        return((int)target);
}
