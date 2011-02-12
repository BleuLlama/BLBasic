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


#ifndef __basic_tokenizer_h__
#define __basic_tokenizer_h__

#include "basic_variable.h"



/* ******************************************************************************** */

/* The structure of the parsed in line is essentially:
 *
 * 	A list of LINEs 
 * 	each LINE has a list of COMMAND
 * 	each COMMAND is a list of TOKENs
 */

typedef struct basic_token {
	int token_id;
	long l_data;
	char * c_data;
	struct basic_token * next;
	struct basic_token * shunt;

	struct basic_token * nextData;
} basic_token;



typedef struct basic_line {
	long lineNumber;
	int temp;			/* used by renum */
	struct basic_token * tokens;
	struct basic_line * next;
	struct basic_line * prev;

	struct basic_line * continuation; 	/* tree added with ':' operator */
	struct basic_line * continuationParent;

	struct basic_token * nextData;
} basic_line;
#define kNoLineNumber	-1



#define kGosubLevels	256

typedef struct basic_program {
	int exitNow;
	struct basic_line * listing;

	int traceOn;
	basic_line * runningLine;
	long nextLineNumberToExecute;

	basic_line * gosubList[kGosubLevels];	/* list of lines that called the gosub last */
	int nGosubLevels;			/* starts at 0 - # of gosub calls so far */

	basic_variable * variables;

	basic_token * firstData;
	basic_token * currentData;
} basic_program;


/* ******************************************************************************** */

/* create new nodes */
basic_token * newToken( void );
basic_line * newLine( void );
basic_program * newProgram( void );

/* list manips */
basic_token * appendToken( basic_token * existingTokens, basic_token * theNewToken );

/* uncreate node trees */
void deleteTokens( basic_token * tokens );
void deleteLines( basic_line * lines );
void deleteProgram( basic_program * bp );

/* bookkeepping */
long countSteps( basic_line * lines );
long programSize( basic_program * bp );
void clearRuntime( basic_program * bp );

/* lineArgc
 * 	return the number of elements in the token list of this line
 */
int lineArgc( basic_line * bl );

/* argvToken
 * 	retrieves the token at the specified index
 */
basic_token * argvToken( basic_line * bl, int idx );

/* argvLong
 * 	retrieves the long data at the specified index
 */
long argvLong( basic_line * bl, int idx );

/* argvChar
 * 	retrieves the string data at the specified index
 */
char * argvChar( basic_line * bl, int idx );

/* argvTokenID
 * 	retrieves the string data at the specified index
 */
int argvTokenID( basic_line * bl, int idx );

/* argvMatch
 *	match the token list to a passed in string rep
 *	A = aplhanumeric (variable name)
 *	Q = Quoted	 (immediate string)
 *	0 = number       (immediate value) (zero)
 *	; = Semicolon
 *	, = comma
 *	- = dash
 *	E = require end of list
 */ 
int argvMatch( basic_token * tl, char ** matchList );


basic_line * findLineNumber( basic_program * bp, long lineNo );
basic_line * findLowestLineNumber( basic_program * bp, long lineNo );

void removeLineNumber( basic_program * bp, long lineNo );

/* return a string suitible for LIST of the tokens */
char * catTokenToString( basic_token * tt, char * toBuf, int bufSize );
char * stringizeLine( basic_line * l, char * toBuf, int bufsize );



/* tokenize a string into the proper location */
basic_line * tokenizeLine( char * line );

/* consume a string , and return the tokens
 * if the line has a line nummber, it will get inserted to the program listing
 */
basic_line * consumeString( basic_program * bp, char * inString );

#endif
