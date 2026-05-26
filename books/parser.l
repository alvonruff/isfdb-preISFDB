%{
/*
 * #ident "%W%	%G% %Q%"
 *
 *     (C) COPYRIGHT 1994 Al von Ruff
 *         ALL RIGHTS RESERVED
 *
 */

#include "parser.tab.h"
#include <stdio.h>

int	read_digit;		/* Last integer read	*/
char	read_name[256];		/* Last string read	*/
extern int line_number;		/* Current line number	*/

%}

newline [\n]
digit	[0-9]
number	{digit}+
lstrict	[A-Za-z]
lother	[ .,\'\:\-\!\(\)\*\?\&]
stuff	[\%\$\~\`]
string  ({lother}|{digit}|{lstrict})+
ascii   {lstrict}+

%%
{newline}		{ line_number++; }
"|"			{ return(T_BAR); }
"["			{ return(T_LBRACKET); }
"]"			{ return(T_RBRACKET); }
"_"			{ return(T_UNDER); }
"#"			{ while( input() != '\n' );
			  line_number++;
		  	}
{stuff}			{ return(T_STUFF); }
{number}		{ (void)sscanf(yytext,"%d", &read_digit);
#ifdef DEBUG
			  printf("lex int: <%d>\n", read_digit);
#endif
		  	  return(T_INT); }
{ascii}			{ (void)strcpy(read_name,yytext);
#ifdef DEBUG
			  printf("lex asc: <%s>\n", read_name);
#endif
			  return(T_ASCII); }
{string}		{ (void)strcpy(read_name,yytext);
#ifdef DEBUG
			  printf("lex str: <%s>\n", read_name);
#endif
			  return(T_STRING); }
%%
