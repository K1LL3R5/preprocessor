#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define BUFSIZE 128
char buf[BUFSIZE];
int bufp = 0;

#define getch() (bufp>0)?(buf[--bufp]):(getchar())
#define ungetch(c) (buf[++bufp]=c) \
		: (printf("ungetch: too many chars in stack\n"))

#define WORDBUFSIZE 128
char *wstack[BUFSIZE];
int   wstackp = 0;

char *file_text[BUFSIZE];   /* NOT IN USE: UTILIZE ONCE TEXT-INPUT IS IMPLEMENTED */

#define pushword(word) (wstackp < WORDBUFSIZE) ? wstack[wstackp++]=word \
								: printf("pushword: Stack Full\n");
#define popword() (wstackp > 0) ? wstack[--wstackp] \
					: printf("popword: Stack empty\n");

typedef struct _token {
	int t_type;
	char *t_identifier;
	char *t_value;
	bool t_isPrivate;
	struct _token *t_next;
} *Token;

Token CreateToken(int token_type, char *identifier, char *value, bool isPrivate)
{
	Token newToken = (Token)malloc(sizeof(struct _token));
	newToken->t_type 	   = token_type;
	newToken->t_identifier = identifier;
	newToken->t_value      = value;
	newToken->t_isPrivate  = isPrivate;
	newToken->t_next       = (Token)NULL;

	return(newToken);
}

/* When switching to file input, the only change required will be 
 * changing all getch() calls to getnext() calls, which will retrieve
 * the next character within the current line to be parsed. */
int parse_input()
{
	char *line          = (char*)NULL;   /* Holds current line to be parsed */
	char *cbuf          = (char*)NULL;   /* holds a collected identifier/value/statement */
	int cbufcnt         = 0;    		 /* number of characters within the cbuf buffer */
	char Char      		= 0;  			 /* Holds current character being parsed/lexed */
	bool statement      = false;		 /* indicates whether current line is a statement */
	bool def_ident 		= false;		 /* indicates whether a define identifier is being collected */
	bool def_value      = false;		 /* indicates whether a define value is being collected */
	bool def_assignment = false;		 /* indicates whether or not current line is an assignment */
	bool inc_inclusion  = false;		 /* indicates whether current line is an inclusion */
	bool inc_filename   = false;		 /* indicates whether an include filename is being collected */
	bool inword         = false;		 /* indicates whether currently in a word or out of a word */
	bool bol  			= true;          /* indicates whether we're at the beginning of a line of input */
	bool multiline      = false;         /* indicates whether a backslash '\' has been specified for a multi-line statement */
	bool statement_ident_collected = false;    /* indicates whether a statement identifier has been collected */
	bool isString       = false;

	while ((Char=getch())!=EOF)
	{
		/* PLEASE FIND A MORE EFFICIENT MEMORY ALLOCATOR FOR THE CHARACTER BUFFER */
		if (cbuf==(char*)NULL) 
		{
			cbuf = (char*)malloc(WORDBUFSIZE);
		}
		/* ---------------------------------------------------------------------- */

		if (isspace(Char) && isString == false)
		{
			Char = getch(); /* Skip whitespaces  */
		}

		if (Char=='#' && bol==true) /* Only recognize hash if we're at the beginning of a non-whiteline line of input */
		{
			statement = true;		/* Indicate we're now in a statement */
			Char = getch();			/* And skip it, since this char isnt important elsewhere */
		}
		
		if (statement_ident_collected == false)
		{
			if (strcmp(cbuf, "define"))
			{
				cbuf = (char*)NULL;
				statement_ident_collected = true;
				def_ident = true;		// Indicate we're now parsing a define statement, and will be retrieving the identifier
				def_assignment = true;  // indicate we're not in an assignment
			}
			else if (strcmp(cbuf, "include"))
			{
				statement_ident_collected = true;
				cbuf = (char*)NULL;
				inc_inclusion = true;	// Indicate we're now parsing an inclusion 
			}
			else
			{
				printf("cpp: Unknown statement parsed {Line %i Char %i}\n", 0, 0);
				free(cbuf);
				return(-1);
			}
		}

		if (isalnum(Char))
		{
			inword = true;			/* Indicate we're currently inside of a 'word' -- I.E we're not a whiteline or a hash or ctrl char */
		}

		/* PLEASE FIND A MORE EFFICIENT MEMORY ALLOCATOR FOR THE CHARACTER BUFFER */
		if (cbuf==(char*)NULL) 
		{
			cbuf = (char*)malloc(WORDBUFSIZE);
		}
		/* ---------------------------------------------------------------------- */

		if (statement == true)
		{
			switch(Char)
			{
				case '=':
					if (def_assignment == true)
					{
						if (def_ident == true)
						{
							if (def_value == true)
							{
								printf("cpp: Invalid syntax parsed {Line %i Char %i}\n", 0, 0);

								free(cbuf);
								return(-1);
							}

							printf("DEBUG: IDENTIFIER %s PUSHED\n", cbuf);
							pushword(cbuf);
							cbuf = (char*)NULL;
						}

						
					}
					else
					{
						printf("cpp: Erraneous '=' Parsed {Line %i Char %i}\n", 0, 0);

						free(cbuf);
						return(-1);
					}
				break;

				case '\n':
					if (inc_filename == true)
					{
						printf("cpp: Incomplete Include statement {Line %i Char %i}\n", 0, 0);

						free(cbuf);
						return(-1);
					}
					else if (def_assignment == true)
					{
						printf("cpp: Incomplete assignment {Line %i Char %i}\n", 0, 0);

						free(cbuf);
						return(-1);
					}
					else if (multiline == true)
					{
						Char = getch(); /* Skip the newline, a backslash has been included. Continue statement on next line */
					}
					else 
					{
						if (def_assignment == true)
						{
							if (def_value == true)
							{
								if (def_ident == true)
								{
									printf("cpp: Incorrect syntax parsed {Line %i Char %i}", 0, 0);
									printf("DEBUG - def_ident AND def_value raised\n");
									
									free(cbuf);
									return(-1);
								}
								pushword(cbuf); /* Push define value */
								def_value = false; /* Indicate we're no longer collecting a define value */
							}
							else
							{
								printf("cpp: Null value in statement\n");

								free(cbuf);
								return(-1);
							}
						}
					}
				break;

				case '<':

				break;

				default:
					cbuf[cbufcnt++] = Char;
				break;
			}
		}
	}
	return(-1);
}

int main(void)
{
	parse_input();
	return(0);
}