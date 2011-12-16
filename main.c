/* BleuLlama BASIC
 *
 *   A BASIC interpreter
 *   Copyright (c) 2011 Scott Lawrence
 */

/* TODO:
	REM command needs to store to end of line
	after immediate evaluate() here, we need to dispose of the tree.
	    deleteLines( bl ); ?
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

/* Reference:
 *	http://www.devili.iki.fi/Computers/Commodore/C64/Programmers_Reference/Chapter_1/page_008.html
 * 	http://www.commodore.ca/manuals/funet/cbm/programming/cbm-basic-tokens.txt
 *	http://www.c64-wiki.com/index.php/C64-Commands
 */

#include <stdio.h>
#include <stdlib.h>		/* exit */
#include <signal.h>		/* signal catching */
#include <string.h>		/* strcmp */
#include <sys/stat.h>		/* mkdir on OS X */
#include <unistd.h>		/* mkdir on MinGW */
#include <time.h>		/* for nanosleep */

#include "basic_version.h"
#include "basic_errors.h"
#include "basic_tokenizer.h"
#include "basic_evaluator.h"
#include "basic_string.h"

/* because windows is braindead */
#if defined(_WIN32) || defined( __MINGW32__ )
#ifndef SIGHUP
#define SIGHUP 		-9
#endif
#endif


/************************************************************************/ 
/*  misc info */


void startInfo( basic_program * bp )
{
	cmd_info( bp, NULL );
	printf( "Ready.\n" );
}


/************************************************************************/ 
/*  Command line, interactive stuff */

int breakCount = 0;

/* signalBreak
 *
 *	our unix signal handler
 */
void signalBreak( int sig )
{
	if( sig == SIGHUP || sig == SIGINT ) {
		printf( "BREAK %d/5!\n", breakCount+1 );
		breakCount++;
		if( breakCount >= 5 ) {
			exit( 0 );
		}
	}
}


/* main
 *
 *	do some main stuff.
 */
int main( int argc, char ** argv )
{
	basic_program * bp;
	basic_line * bl;
	char lnbuf[kInputBufferSize];

	/* set up our ctrl-c catchers */
	(void) signal( SIGINT, signalBreak );
	(void) signal( SIGHUP, signalBreak );

	/* make the documents directory, if we don't already have it */
#if defined(_WIN32) || defined( __MINGW32__ )
	mkdir( kDocumentsDirectory );
#else
	mkdir( kDocumentsDirectory, 0755 );
#endif

	/* set up our program space */
	bp = newProgram();
	if( !bp ) {
		errorReport( kErrorMalloc, 1 );
		return 0;
	}

	/* dump out version info and stuff */
	startInfo( bp );

	/* and loop in our minishell! */
	do {
		/* print our prompt, get user input */
		fprintf( stdout, ">: " ); fflush( stdout );
		fgets( lnbuf, kInputBufferSize, stdin);
		
		/* tokenize and absorb the string into our listing */
		bl = consumeString( bp, lnbuf );

		/* if it was not absorbed, it has a negative line number */
		if( bl ) {
			if( bl->lineNumber == kNoLineNumber ) {
				/* so immediately evaluate it. */
				evaluateLine( bp, bl );
				/* and any continuations it might have. */
				while( bl->continuation ) {
					bl = bl->continuation;
					evaluateLine( bp, bl );
				}
			} else {
				/* let's print out the line */
				/* and reuse lnbuf */
				if( bp->traceOn ) {
					stringizeLine( bl, lnbuf, 1024 );
					printf( "%s\n", lnbuf );
				}

				/* HACK. We should do this:
				deleteLines( bl );
				*/
			}
		}

		/* reset our break counter */
		breakCount = 0;

		/* and run the program, if we should... */
		while( run_poll( bp ) && !breakCount )
		{
			/* for now...  */
			run_poll( bp );

			/* if we're doing a trace, display the line */
			if( bp->traceOn ) {
				dumpVariables( bp->variables );
			}

			/* let's not kill the system, and give it a retroey speed */
			usleep( 1 * 1000 );
		}

		/* also, dump the variables each line entered */
		if( bp->traceOn ) {
			dumpVariables( bp->variables );
		}

		/* force the run poll to not continue */
		/* HACK for now. */
		/* need to figure this out. */
		bp->runningLine = NULL;
		bp->nextLineNumberToExecute = -1;
		
	} while( !bp->exitNow );

	return 0;
}
