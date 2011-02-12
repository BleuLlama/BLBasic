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

char *program[] = {
	"10 print \"Hello, World!\"",
	"20 for a = 0 to 10",
	"30 print a",
	"40 next a",
	"50 end"
};


/* main
 *
 *	do some main stuff.
 */
int main( int argc, char ** argv )
{
	basic_program * bp;
	basic_line * bl;


	printf( "0\n" );
	/* set up our program space */
	bp = newProgram();
	if( !bp ) {
		errorReport( kErrorMalloc, 1 );
		return 0;
	}

	/* display version info */
	cmd_info( bp, NULL );

	bl = consumeString( bp, program[0] );
	bl = consumeString( bp, program[1] );
	bl = consumeString( bp, program[2] );
	bl = consumeString( bp, program[3] );
	bl = consumeString( bp, program[4] );

	/* and run the program, if we should... */
	printf( "Running program\n" );
	runProgram( bp, 0 );
	while( run_poll( bp ) );

	/* just show access of variables */
	printf( "Variable 'a' is %ld\n", getVariableNumber( bp->variables, "a" ));

	deleteProgram( bp );

	return 0;
}
