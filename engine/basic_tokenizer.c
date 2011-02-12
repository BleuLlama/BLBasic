/* basic_tokenizer
 *
 *   BleuLlama BASIC
 *   Copyright (c) 2011 Scott Lawrence
 */

/* LICENSE:
 *
 * Copyright (C) 2011 by Scott Lawrence
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>		/* printf */
#include <stdlib.h>		/* malloc */
#include <string.h>		/* strdup, strncpy */
#include <ctype.h>		/* tolower */

#include "basic_tokenizer.h"
#include "basic_evaluator.h"
#include "basic_errors.h"
#include "basic_string.h"

/* ******************************************************************************* */


/* extractQuotedFromString
 *
 *	return a string of the contents of the current quoted text
 *	on failure, return NULL
 */
char * extractQuotedFromString( char * line )
{
	char * ret;
	int i = 0;

	if( !line ) return strdup( "" );
	/* this assumes that *line == '"' */
	line++;

	/* count size */
	while( line[i] != '\0' && line[i] != '"' ) i++;

	/* check for EOL */
	if( line[i] == '\0' ) {
		return NULL;
	}

	ret = (char *)malloc( sizeof( char ) * (i+1) );
	strncpy( ret, line, i );
	ret[i] = '\0';

	return ret;
}


/* extractNumberFromString
 *
 *	return a string of the contents of the current integer
 */
char * extractNumberFromString( char * line )
{
	char * ret;
	int i = 0;

	if( !line ) return strdup( "" );
	
	/* count size */
	while( line[i] != '\0' && isdigit( line[i] )) i++;

	ret = (char *)malloc( sizeof( char ) * (i+1) );
	strncpy( ret, line, i );
	ret[i] = '\0';

	return ret;
}



/* ******************************************************************************* */

/* create new nodes */
basic_token * newToken( void )
{
	basic_token * bt = (basic_token *)malloc( sizeof(basic_token) );
	if( !bt ) return NULL;

	bt->token_id = 0;
	bt->l_data = 0;
	bt->c_data = NULL;
	bt->next = NULL;
	bt->nextData = NULL;
	return bt;
}

basic_token * appendToken( basic_token * existingTokens, basic_token * theNewToken )
{
	basic_token * mp;

	if( !theNewToken ) return existingTokens;	/* nothing to add. bail out */
	if( !existingTokens ) return theNewToken;	/* nothing to start with, make a new one */

	mp = existingTokens;

	while( mp->next ) mp = mp->next;
	mp->next = theNewToken;

	return existingTokens;
}



basic_line * newLine( void )
{
	basic_line * bl = (basic_line *)malloc( sizeof(basic_line) );
	if( !bl ) return NULL;

	bl->lineNumber = kNoLineNumber;
	bl->temp = -1;
	bl->tokens = NULL;
	bl->continuation = NULL;
	bl->continuationParent = NULL;
	bl->next = NULL;
	bl->prev = NULL;
	bl->nextData = NULL;
	return bl;
}


void clearRuntime( basic_program * bp )
{
	if( !bp ) return;

        bp->runningLine = NULL;
        bp->nextLineNumberToExecute = 0;
	bp->nGosubLevels = 0;
}



basic_program * newProgram( void )
{
	basic_program * bp = (basic_program *)malloc( sizeof(basic_program) );
	if( !bp ) return NULL;

	bp->exitNow = 0;
	bp->listing = NULL;
	bp->variables = NULL;
	bp->traceOn = 0;

	bp->firstData = NULL;
	bp->currentData = NULL;

	clearRuntime( bp );

	return bp;
}


/* uncreate node trees */
void deleteTokens( basic_token * tokens )
{
	basic_token * n;
	while( tokens )
	{
		if( tokens->c_data ) free( tokens->c_data );
		n = tokens->next;
		free( tokens );
		tokens = n;
	}
}

void deleteLines( basic_line * lines )
{
	basic_line * n;
	while( lines ) {
		if( lines->tokens ) deleteTokens( lines->tokens );
		if( lines->continuation ) deleteLines( lines->continuation );
		n = lines->next;
		free( lines );
		lines = n;
	}
}

void deleteProgram( basic_program * bp )
{
	free( bp->listing );
	free( bp );
}


/* ******************************************************************************* */
/* bookkeepping */

/* countSteps
 *
 * 	count the number of tokens in a line
 * 	NOTE: this is a fictitious number, with value based on data structure content
 */
long countSteps( basic_line * bl )
{
	long count = 1;	/* the line number is one step */

	basic_token * t;

	if( !bl ) return 0;

	if( !bl->tokens ) return 0;

	t = bl->tokens;

	while( t )
	{
		if( t->c_data ) count += strlen( t->c_data );
		else count++;

		t = t->next;
	}
	return count;
}


/* programSize 
 *
 * 	count the number of steps in a program
 * 	NOTE: this is a fictitious number, with value based on data structure content
 */
long programSize( basic_program * bp )
{
	long count = 0;
	basic_line * l;

	if( !bp ) return 0;
	if( !bp->listing ) return 0;

	l = bp->listing;
	while( l )
	{
		count += countSteps( l );
		l = l->next;
	}

	return count;
}


/* ******************************************************************************* */
/* some stuff to help with dealing with token lists */

/* lineArgc
 * 	return the number of elements in the token list of this line
 */
int lineArgc( basic_line * bl )
{
	int ret = 0;
	basic_token * tt;

	if( !bl ) return 0;

	tt = bl->tokens;

	while( tt ) {
		ret++;
		tt = tt->next;
	}

	return ret;
}


/* argvToken
 * 	retrieves the token at the specified index
 */
basic_token * argvToken( basic_line * bl, int idx )
{
	basic_token * tt = bl->tokens;

	while( idx > 0 && tt ) 
	{
		tt = tt->next;
		idx--;
	}
	return tt;
}

/* argvLong
 * 	retrieves the long data at the specified index
 */
long argvLong( basic_line * bl, int idx )
{
	basic_token * tt = argvToken( bl, idx );
	if( !tt ) return 0;
	return tt->l_data;
}


/* argvChar
 * 	retrieves the string data at the specified index
 */
char * argvChar( basic_line * bl, int idx )
{
	basic_token * tt = argvToken( bl, idx );
	if( !tt ) return NULL;
	return tt->c_data;
}


/* argvTokenID
 * 	retrieves the string data at the specified index
 */
int argvTokenID( basic_line * bl, int idx )
{
	basic_token * tt = argvToken( bl, idx );
	if( !tt ) return -1;
	return tt->token_id;
}


/* argvMatch
 *
 *	matches a basic_line passed in with various possible token lists
 *	A = aplhanumeric (variable name)
 *	Q = Quoted	 (immediate string)
 *	0 = number       (immediate value) (zero)
 *	V = Alphanumeric (variable name)  OR Number (immediate Value)
 *	; = Semicolon
 *	, = comma
 *	- = dash
 *	E = require end of list
 * 	ex:
 *		int matchedLIST = argvMatch( tl, { "E", "0E", "0-E", "-0E", "00E", "0-0E", NULL } );
 *
 * 	returns >= 0 on match
 *	returns -1 on not found (invalid parameter list)
 */
int argvMatch( basic_token * tl, char ** matchList )
{
/*
	char *s1;
	char *s2;
*/
	char ** ml = matchList;
	char buf[512];
	basic_token * inx = tl;
	int pos = 0;
	char add;

	char lastc;

	if( !matchList ) return kErrorNonsense;

	/* okay. here's what we're going to do here */
	/* 1 build a list of what we have in the token list */
	/* (a simplified type-only version) */
	while( pos < 511 && inx )
	{
		switch( inx->token_id ) {
		case( kToken_Alpha ):		add = 'A'; 	break;
		case( kToken_Number ):		add = '0'; 	break;
		case( kToken_Colon ):		add = ':'; 	break;
		case( kToken_Semicolon ):	add = ';'; 	break;
		case( kToken_Quote ):		add = 'Q'; 	break;
		case( kToken_Comma ):		add = ','; 	break;
		case( kToken_Dash ):		add = '-'; 	break;
		case( kToken_Equals ):		add = '='; 	break;
		default:
			add = 'T';	/* TOKEN probably... */
			break;
		}

		buf[pos++] = add;
		inx=inx->next;
	}
	buf[pos++] = 'E';
	buf[pos] = '\0';

	/* 2 scroll through the matchList and see what matches */
	pos = 0;
	ml = matchList;
	while( *ml ) {
		char * tml = *ml;
		if( strlen( tml ) > 1 ) lastc = tml[ strlen( *ml )-1 ];
		else lastc = tml[0];

		/* first, let's do an explicit exact check */
		if( lastc == 'E' && !strcmp( *ml, buf )) {
			return pos;

		} else if( lastc != 'E' && !strncmp( *ml, buf, strlen( *ml )) ) {
			return pos;
		}

#ifdef NEVER
		/* okay, maybe that didn't work out. let's check again */
		s1 = *ml; /* user provided */
		s2 = buf; /* our generated list */
		while( s1 && s2 )
		{
			char c1 = *s1;
			char c2 = *s2;

			/* for each character,
 			 * if c1 != c2 
 			 * 	if( c1 is a 'v' and c2 isn't '0' or 'A' )
 			 * 		then next;
 			 */
			if( c1 != c2 ) {
				if( c1 == 'V' ) {
					if( c2 != '0' && c2 != 'A' ) {
						break;
					}
				}
			}

			s1++; s2++;
		}
#endif
		pos++;
		ml++;
	}

	/* okay. nothing was found. let's check for one of the same length */
	ml = matchList;
	while( *ml ) {
		if( strlen( *ml ) == strlen( buf )) return( kErrorParamType );
		ml++;
	}


	/* nothing applicable, send this error message */
	return( kErrorParams );
}


/* ******************************************************************************* */

/* findLineNumber
 *
 * 	finds a specific line number in a program
 */
basic_line * findLineNumber( basic_program * bp, long lineNo )
{
	basic_line * l;

	if( !bp ) return NULL;
	if( !bp->listing ) return NULL;

	l = bp->listing;

	while( l )
	{
		if( l->lineNumber == lineNo ) return l;
		l = l->next;
	}

	return NULL;
}

/* findLowestLineNumber
 *
 * 	finds the first line with the number equal to or greater than the specified line
 */
basic_line * findLowestLineNumber( basic_program * bp, long lineNo )
{
	basic_line * l;
	if( !bp ) return NULL;
	if( !bp->listing ) return NULL;

	if( lineNo <= 0 ) return bp->listing;

	l = bp->listing;

	while( l )
	{
		if( l->lineNumber >= lineNo ) return l;
		l = l->next;
	}
	return NULL;
}


/* removeLineNumber
 *
 * 	removes the specified line number from the list
 */
void removeLineNumber( basic_program * bp, long lineNo )
{
	basic_line * l;

	if( !bp ) return;
	if( !bp->listing ) return;

	/* the few places we can be:

	1) first element on the list
	2) middle of the list
	3) end of the list
	*/

	l = bp->listing;
	while( l )
	{
		if( l->lineNumber == lineNo ) {

			/* check for head of the line */
			if( l == bp->listing ) {
				/* head of the list */
				bp->listing = bp->listing->next;
				if( bp->listing ) {
					bp->listing->prev = NULL;
				}
			} else if( l->next ) { 
				/* item is in the middle of the list */
				l->prev->next = l->next;
				l->next->prev = l->prev;
			} else {
				/* item is the tail of the list */
				l->prev->next = NULL;
			}

			/* just in cases */
			l->prev = NULL;
			l->next = NULL;

			/* remove the line! */
			deleteLines( l );
			return;
		}
		l = l->next;
	}
}


/* insertLine
 *
 * 	adds the specified line in to the program
 */
void insertLine( basic_program * bp, basic_line * theLine )
{
	basic_line * l;

	if( !bp || !theLine ) return;

	/* the few places we can be:
	-> NULL			1) no list, insert at the head

	-> Z -> NULL		2) insert at head, before first item

	-> A -> Z -> NULL	3) insert in the middle

	-> A -> NULL		4) insert at tail
	*/


	/* 1 */
	if( !bp->listing ) {
		/* first line! */
		bp->listing = theLine;
		theLine->prev = NULL;
		theLine->next = NULL;
		return;
	}


	/* 2 */
	if( bp->listing->lineNumber > theLine->lineNumber )
	{
		/* insert at the head, with content past it */
		theLine->next = bp->listing;
		bp->listing->prev = theLine;
		bp->listing = theLine;
		return;
	}

	/* test for 3 */
	l = bp->listing;
	while( l->next )
	{
		if( l->next->lineNumber > theLine->lineNumber )
		{
			theLine->next = l->next;
			theLine->prev = l;
			theLine->next->prev = theLine;
			theLine->prev->next = theLine;
			return;
		}
		l = l->next;
	}

	/* 4 */
	l->next = theLine;
	theLine->prev = l;
}


/* ******************************************************************************* */


char * catTokenToString( basic_token * t, char * toBuf, int bufsize )
{
	char tstr[32];

	if( !t ) return toBuf;

	if( t->token_id < kToken_User ) {
		strncat( toBuf, findTokenStringFromID( t->token_id ), bufsize );

	} else {
		switch( t->token_id )
		{
		case( kToken_Alpha ):
			strncat( toBuf, t->c_data, bufsize );
			break;

		case( kToken_Quote ):
			strncat( toBuf, "\"", bufsize );
			strncat( toBuf, t->c_data, bufsize );
			strncat( toBuf, "\"", bufsize );
			break;

		case( kToken_Number ):
			snprintf( tstr, 32, "%ld", t->l_data );
			strncat( toBuf, tstr, bufsize );
			break;

		case( kToken_Colon ):
			strncat( toBuf, ":", bufsize );
			break;

		case( kToken_Semicolon ):
			strncat( toBuf, ";", bufsize );
			break;

		case( kToken_Comma ):
			strncat( toBuf, ",", bufsize );
			break;

		case( kToken_User ):
		case( kToken_ERROR ):
		case( kToken_Space ):
		case( kToken_Newline ):
		case( kToken_EOS ):
			strncat( toBuf, "?", bufsize );
			break;
		}
	}

	return toBuf;
}

/* return a string suitible for LIST of the tokens */
char * stringizeLine( basic_line * l, char * toBuf, int bufsize )
{
	int prependSpace = 0;
	basic_token * tt;

	if( !l ) return toBuf;

	if( l->lineNumber != kNoLineNumber )
	{
		snprintf( toBuf, bufsize, "%ld ", l->lineNumber );
	} else {
		toBuf[0] = '\0';
	}
	
	tt = l->tokens;
	while( tt ) {
		if( prependSpace ) {
			strncat( toBuf, " ", bufsize );
		}
		prependSpace++;
		catTokenToString( tt, toBuf, bufsize );
		tt = tt->next;
	};

	if( l->continuation ) {
		strncat( toBuf, " : ", bufsize );
		stringizeLine( l->continuation, toBuf + strlen(toBuf), bufsize - strlen(toBuf) );
	}

	return toBuf;
}



/* ******************************************************************************* */


int tokenTypeFromString( char *c, int disableLut )
{
	int tidx = 0;

	if( !c ) return kToken_ERROR;

	if( *c >= '0' && *c <= '9')	return kToken_Number;
	if( *c == 10 )			return kToken_Newline;
	if( *c == ':' )			return kToken_Colon;
	if( *c == ';' )			return kToken_Semicolon;
	if( *c == '"' )			return kToken_Quote;
	if( *c == ',' )			return kToken_Comma;
	if( *c == '\0' )		return kToken_EOS;
	if( isspace( *c ) ) 		return kToken_Space;

	if( disableLut && isalnum( *c )) {
		return kToken_Alpha;
	}

	while( token_lut[tidx].cmd != NULL )
	{
		if( isSameToken( token_lut[tidx].cmd, c ))
		{
			return( token_lut[tidx].token );
		}
		tidx++;
	}

	return kToken_Alpha;
}

int bytesToNextToken( char * line )
{
	int startToken;
	int idx = 1;

	if( !line ) return 0;

	startToken = tokenTypeFromString( line, 0 );

#ifdef NEVER
	if( startToken == kToken_Alpha )
	{
		/* check for an alphanumeric label */
		while(    line[idx] != '\0' && line[idx] != '$'
		       && ( isalnum( line[idx] ) || line[idx] != '_' ))
		{
			idx++;
		}
		return idx;
	}
#endif

	/* check for something else */
	if( startToken >= kToken_User ) {
		while(     line[idx] != '\0' 
			&& tokenTypeFromString( line+idx, (startToken == kToken_Alpha) ) == startToken )
		{
			idx++;
		}
	}
	return idx;
}

/* tokenize a string into the proper location */
basic_line * tokenizeLine( char * line )
{
	char * pstr;
	char * tstr;
	char * lno;
	int thisToken;
	int comment = 0;
	basic_line * bl;
	basic_token * bt;
	basic_line * temp;

	if( !line ) return NULL;

	/* remove newline character at the end of the line */
	stringChomp( line );

	/* advance past initial whitespace */
	while( isspace( *line ) ) line++;

	/* check for empty string */
	if( strlen( line ) == 0 ) return NULL;

	/* okay! now, let's tokenize this mofo! */
	bl = newLine();

	/* now, check for a line number */
	lno = extractNumberFromString( line );
	if( strlen( lno )) {
		bl->lineNumber = atol( lno );
		line += strlen( lno );	/* advance past line number */
	}

	/* und overiteraten remainingtext, mit builden das tokenz */
	while( *line != '\0' )
	{
/*
		char * newToken = stringUntilDifferentToken( line );
		printf( "TOK [%s]\n", newToken );
		line += strlen( newToken );
*/
		tstr = NULL;
		pstr = NULL;
		thisToken = tokenTypeFromString( line, 0 );

		if( comment ) {
			while( isspace( *line ) ) line++;

			bt = newToken();
			bt->token_id = kToken_Alpha;
			bt->c_data = strdup( line );
			bl->tokens = appendToken( bl->tokens, bt );
			line += strlen( line );

		} else if( thisToken < kToken_User ) {
			/* it's a basic token */
			basic_tlut * lutval = findLutTokenFromID( thisToken );
			line += strlen( lutval->cmd );

			bt = newToken();
			bt->token_id = thisToken;
			bl->tokens = appendToken( bl->tokens, bt );

			/* check for REM statement */
			if( thisToken == 0x8f ) { comment = 1; }

		} else if( thisToken == kToken_Colon ) {
			/* COLON is a new statement. add it to the continuation value */
			line++; /* move past the colon */
			bl->continuation = tokenizeLine( line );
			if( bl->continuation ) {
				bl->continuation->continuationParent = bl;
			}

			/* fill in the line numbers */
			temp = bl->continuation;
			while( temp ) {
				temp->lineNumber = bl->lineNumber;
				temp = temp->continuation;
			}

			/* and force a completion of the line */
			line += strlen( line );

		} else if( thisToken == kToken_Space ) {
			/* eat it. */
			line++;

		} else if( thisToken == kToken_Quote ) {
			/* it's a quoted string */
			char * quotedText = extractQuotedFromString( line );
			if( quotedText ) {
				char * new_c_data = strdup( quotedText );
				line += strlen( quotedText ) + 2; /* +2 to account for quotes */

				bt = newToken();
				bt->token_id = thisToken;
				bt->c_data = new_c_data;
				bl->tokens = appendToken( bl->tokens, bt );
				free( quotedText );
			}

		} else if( thisToken == kToken_Number ) {
			char * theNumber = extractNumberFromString( line );
			if( theNumber )
			{
				line += strlen( theNumber );
				pstr = strdup( theNumber );

				bt = newToken();
				bt->token_id = thisToken;
				bt->l_data = atol( theNumber );
				bl->tokens = appendToken( bl->tokens, bt );

				free( theNumber );
			}
		
		} else {
			int sz = bytesToNextToken( line );
			char * label = (char *) malloc( sizeof( char ) * (sz+1) );

			strncpy( label, line, sz );
			label[sz] = '\0';

			bt = newToken();
			bt->token_id = thisToken;
			bt->c_data = label;
			bl->tokens = appendToken( bl->tokens, bt );

			line += sz;
		}


		/* display it */
		tstr = findTokenStringFromID( thisToken );

		if( pstr ) free( pstr );
	}

	return bl;
}

/* consume a string , and return the tokens
 * if the line has a line number, it will get inserted to the program listing
 */
basic_line * consumeString( basic_program * bp, char * inString )
{
	basic_line * bl;

	if( !bp || !inString ) return NULL;

	bl = tokenizeLine( inString );

	if( bl ) {
		if( bl->lineNumber >= 0 ) {
			removeLineNumber( bp, bl->lineNumber );
			if( bl->tokens ) insertLine( bp, bl );
		}
	}

	return bl;
}
