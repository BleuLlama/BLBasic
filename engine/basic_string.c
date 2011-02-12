/* basic_string
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

#include <stdio.h>
#include <stdlib.h>		/* malloc, etc */
#include <string.h>		/* strdup */
#include "basic_string.h"

/* newString
 *
 *	creates a new string, filled with "" [0]='\0'
 */
char * newString( char * content )
{
	char * ret;

	if( !content ) content = "";

	ret = strdup( content );
	return ret;
}


/* appendString
 *
 *	append a string to an existing string, realloc if necessary
 */
char * appendString( char * start, char * newBit )
{
	char * ret;

	/* nothing to add, just return */
	if( newBit == NULL ) return start;

	/* nothing to start with, start with "" */
	if( !start ) start = newString( "" );

	/* realloc our space for the new bit */
	ret = realloc( start, strlen( newBit ) + strlen( start ) + 2 );
	strcat( ret, newBit );
	return ret;
}


/* appendNumber
 *
 *	append a number to the end of a string, realloc if necessary
 */
char * appendNumber( char * start, long newBit )
{
	char buf[32];
	snprintf( buf, 32, "%ld", newBit );
	return appendString( start, buf );
}


/* stringChomp
 *
 *	remove newlines from the end of a string (if applicable)
 */
char * stringChomp( char * str )
{
	int len;

	/* check for valid strings */
	if( !str ) return NULL;
	len = strlen( str );

	if( len < 1 ) return str;

	/* remove newline stuffs */
	if( str[len-1] == '\n' )
	    str[len-1] = '\0';
	if( str[len-1] == '\r' )
	    str[len-1] = '\0';

	return str;
}


/* getLineFromUser
 *
 *	gets a line of text from the user
 *	returned line must be freed.
 */
char * getLineFromUser( void )
{
	char lnbuf[kInputBufferSize];
	fgets( lnbuf, kInputBufferSize, stdin );

	return( strdup( lnbuf ));
}
