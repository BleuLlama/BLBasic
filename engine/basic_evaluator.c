/* basic_evaluator
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
#include <stdlib.h>		/* malloc, free */
#include <string.h>		/* strlen */
#include <ctype.h>		/* tolower */

#include <dirent.h>             /* for directory listing "ls" */

#include <sys/types.h>          /* for stat */
#include <sys/stat.h>           /* for directory listing (file sizes) */
#include <unistd.h>             /* for stat */

#include "basic_evaluator.h"
#include "basic_version.h"
#include "basic_string.h"
#include "basic_errors.h"

/* ********************************************************************** */


/* valueFromToken
 *	convert a token to a value 
 */
long valueFromToken( basic_token * bt, basic_variable * bv )
{
	long ret = 0;

	if( !bt ) return ret;

	if( bt->token_id == kToken_Number ) {
		ret = bt->l_data;
	} else if( bt->token_id == kToken_Alpha ) {
		if( bv ) ret = getVariableNumber( bv, bt->c_data );
	} else {
		ret = 0;
	}

	return ret;
}

/* evaluateStringExpression
 *
 * 	evaluate a stringical expressionometric whatsit
 */
void evaluateStringExpression( basic_program * bp, basic_token * bt )
{
	char * destVariable;
	char * destContent;

	if( !bp || !bt ) return;
	destVariable = bt->c_data;

	if( getVariableType( destVariable ) != kVar_String )
	{
		errorReport( kErrorParamType, 99 ); /* intenral error! */
		return;
	}

	/* advance to the = */
	bt = bt->next;
	if( !bt ) return;

	if( bt->token_id != 0xb2 ) /* = */
	{
		errorReport( kErrorParamType, 98 ); /* intenral error! */
		return;
	}

	/* okay. now we have expressions to work with... */
	bt = bt->next;

	destContent = (char *) malloc( sizeof( char ) * 2 );
	strcpy( destContent, "" );

	/* okay, we'll do this the simple way.  
 	 * the only things that get appended to strings are numbers,
 	 * other quoted text bits, and variable content */

	while( bt ) {
		if( bt->token_id == kToken_Quote ) {
			/* quoted - just append it */
			destContent = appendString( destContent, bt->c_data );
		}
		if( bt->token_id == kToken_Alpha ) {
			/* alpha - it's a variable */
			if( getVariableType( bt->c_data ) == kVar_String )
			{
				destContent = appendString( destContent, getVariableString( bp->variables, bt->c_data ));
			} else {
				destContent = appendNumber( destContent, getVariableNumber( bp->variables, bt->c_data ));
			}
			
		}
		if( bt->token_id == kToken_Number ) {
			destContent = appendNumber( destContent,bt->l_data );
		}

		bt = bt->next;
	}

	bp->variables = setVariableString( bp->variables, destVariable, destContent );
}

/* evaluateExpression
 *
 * 	evaluate a functional expressional thingy
 */
void evaluateExpression( basic_program * bp, basic_token * bt )
{
	int op = 0xb2;
	char * destVariable;
	long value = 0;

	if( !bp || !bt ) return;
	
	/* set up the destination variable name */
	/* from here, assume for now that it's numerical */
	destVariable = bt->c_data;

	if( getVariableType( destVariable ) == kVar_String )
	{
		evaluateStringExpression( bp, bt );
		return;
	}
	
	if( !bt->next ) return;
	bt = bt->next;	/* should be pointing at an '=' */
	if( !bt->next ) return;
	bt = bt->next;	/* should be pointing at a value or something */

	while( bt ) {
		/* aa +
 		 * ab -
 		 * ac *
 		 * AD /
 		 * AE ^
 		 * b2 =
 		 */
		if( (bt->token_id >= 0xaa && bt->token_id <= 0xae) || bt->token_id == 0xb2) {
			/* store aside our operation */
			op = bt->token_id;

		} else if( bt->token_id == kToken_Number ) {
			long lv = bt->l_data;

			switch( op ) {
			case( 0xb2 ): 	value  = lv;	break;
			case( 0xaa ): 	value += lv;	break;
			case( 0xab ): 	value -= lv;	break;
			case( 0xac ): 	value *= lv;	break;
			case( 0xae ): 	value ^= lv;	break;
			case( 0xad ): 	
				if( lv == 0 ) {
					errorReport( kErrorDBZero, 100 );
					return;
				}
				value /= lv;
				break;
			}

		} else if( bt->token_id == kToken_Alpha ) {
			long lv = getVariableNumber( bp->variables, bt->c_data );

			switch( op ) {
			case( 0xb2 ): 	value  = lv;	break;
			case( 0xaa ): 	value += lv;	break;
			case( 0xab ): 	value -= lv;	break;
			case( 0xac ): 	value *= lv;	break;
			case( 0xae ): 	value ^= lv;	break;
			case( 0xad ):
				if( lv == 0 ) {
					errorReport( kErrorDBZero, 101 );
					return;
				}
				value /= lv;
				break;
			}
		}

		bt = bt->next;
	}
	
	bp->variables = setVariableNumber( bp->variables, destVariable, value );
}

/* evaluateLine
 *
 * 	the call that triggers evaluation of the basic perform operation
 */
void evaluateLine( basic_program * bp, basic_line * bl )
{
	int idx = 0;

	if( !bl || !bl->tokens ) {
		return;
	}


	/* check for keyword */
	while( token_lut[idx].cmd != NULL )
	{
		if( token_lut[idx].token == bl->tokens->token_id )
		{
			/* printf( "%s operation.\n", token_lut[idx].cmd ); */
			if( token_lut[idx].perform )
			{
				token_lut[idx].perform( bp, bl );
			}
			return;
		}
		idx++;
	}

	/* okay. check for variable... */

	if( bl->tokens->token_id == kToken_Alpha )
	{
		evaluateExpression( bp, bl->tokens );
	}
}

/*
 * the structure of all of these entry calls is:
 * 	basic_program * bp		the program that this is assocated with
 * 	basic_line * bl			the line that triggered calling this entry point
 */



/* stopProgram
 *
 *	halt execution of the program
 */
void stopProgram( basic_program * bp )
{
	clearRuntime( bp );
}


/* run_poll
 *
 *	update for running the program
 */
int run_poll( basic_program * bp )
{
	char tl[512];

	long thisLineNumber = bp->nextLineNumberToExecute;
	if( !bp || !bp->runningLine ) return 0;

	if( bp->traceOn )
	{
		printf( ": %s\n", stringizeLine( bp->runningLine, tl, 512 ));
	}

	/* perform the line! */
	bp->nextLineNumberToExecute = -1;
	evaluateLine( bp, bp->runningLine );

	/* now, this can change the nextLineNumberToExecute */
	if( bp->nextLineNumberToExecute >= 0 ) {
		bp->runningLine = findLineNumber( bp, bp->nextLineNumberToExecute );
	} else {
		/* or we continue on our way! */
		if( bp->runningLine->continuation ) {
			/* do the continuation */
			bp->runningLine = bp->runningLine->continuation;

		} else {
			/* if we're done with continuations... back out of it */
			while( bp->runningLine->continuationParent )
			{
				bp->runningLine = bp->runningLine->continuationParent;
			}

			/* and advance to the next line */
			bp->runningLine = bp->runningLine->next;
		}
	}

	if( bp->runningLine ) {
		/* there's something more to do... */
		bp->nextLineNumberToExecute = bp->runningLine->lineNumber;
	} else {
		printf( "\nRun stopped after line %ld.\n", thisLineNumber );
		stopProgram( bp );
	}

	if( bp->runningLine ) return 1;
	return 0;
}


/* ********************************************************************** */

/* new_actual
 *
 * 	do the actual work of NEW
 */
void new_actual( basic_program * bp )
{
	if( bp->listing )
	{
		deleteLines( bp->listing );
		bp->listing = NULL;
	}
	bp->runningLine = NULL;
	bp->nextLineNumberToExecute = -1;
	/* TODO: Free variables too */
}

/* getFilePathInDocuments
 *
 * 	get a full path for the specified file in the documents folder
 * 	NOTE: returns a thing to be freed
 */
char * getFilePathInDocuments( char * fn )
{
	char * ret;
	long fpl = 0;

	if( !fn ) return NULL;

	fpl = strlen( fn ) + strlen( kDocumentsDirectory ) + 2;
	ret = (char *)malloc( sizeof( char ) * fpl );
	snprintf( ret, fpl, "%s/%s", kDocumentsDirectory, fn );
	return ret;
}


/* load_actual
 *
 * 	the routine that does the actual loop/loading from a file
 *	returns 0 on success
 */
int load_actual( basic_program * bp, char * filename )
{
	FILE * fp;
	char buf[1024];

	fp = fopen( filename, "r" );
	if( !fp ) {
		errorReport( kErrorFileError, 100 );
		return -1;
	}

	new_actual( bp );

	while( fgets( buf, 1024, fp ))
	{
		basic_line * lll = consumeString( bp, buf );

                /* if it was not absorbed, it has a negative line number */
                if( lll && lll->lineNumber == kNoLineNumber ) {
			deleteLines( lll );
                }
	}

	fclose( fp );
	printf( "Program loaded from file %s\n", filename );
	return 0;
}


/* save_actual
 *
 * 	the routine that does the actual loop/saving to a file
 */
void save_actual( basic_program * bp, char * filename )
{
	FILE * fp;

	char buf[1024];
	basic_line * ll;

	if( !bp || !bp->listing );

	fp = fopen( filename, "w" );
	if( !fp ) { errorReport( kErrorFileError, 103 ); return; }

	ll = bp->listing;
	while( ll ) {
		stringizeLine( ll, buf, 1024 );
		fprintf( fp, "%s\n", buf );
		ll = ll->next;
	}
	fclose( fp );
	printf( "Program saved to file %s\n", filename );
}


/* cmd_infosteps
 * 
 * 	display step info
 */
void cmd_infosteps( basic_program *bp, basic_line * bl )
{
	if( !bp )
	{
		printf( "No program loaded.\n" );
		return;
	} else {
		printf( "%ld steps used\n", programSize( bp ));
	}
	printf( "\n" );
}


/* rebuild_labels
 *
 * 	rebuild the label variables
 */
void rebuild_labels( basic_program * bp )
{
	basic_line * bl;
	basic_token * tt;

	if( !bp || !bp->listing ) return;

	bl = bp->listing;

	while( bl ) {
		tt = bl->tokens;
		/* look for GOSUB 0x8d  and GOTO 0x89 */
		if( tt )
		{
			if( tt->token_id == 0x0400 ) /* LABEL */
			{
				tt = tt->next;
				if( tt ) {
					if( tt->c_data ) 
					{
						/* first, unprotect the variable */
						bp->variables = protectVariable( bp->variables, tt->c_data, 0 );

						/* store the value */
						bp->variables = setVariableNumber( bp->variables, tt->c_data, bl->lineNumber );

						/* protect the value */
						bp->variables = protectVariable( bp->variables, tt->c_data, 1 );
					}
				}
			}
		}


		bl = bl->next;
	}
}


/* restore_actual
 *
 *	perform the DATA list rebuild, and setup data pointers
 */
void restore_actual( basic_program * bp, long line )
{
	basic_line * bl = NULL;
	basic_token * bt = NULL;
	basic_token * last_data_bt = NULL;

	if( !bp ) return;

	bp->firstData = NULL;
	bp->currentData = NULL;

	/* Rebuild the list */
	bl = bp->listing;
	while( bl ) {
		bt = bl->tokens;
		if( bt->token_id == 0x83 ) {
			while( bt ) {
				bt = bt->next;	/* advance past DATA or , token */
				if( bt ) {
					if( last_data_bt ) {
						last_data_bt->nextData = bt;
					}
					last_data_bt = bt;

					if( !bl->nextData ) {
						bl->nextData = bt;
					}

					bt = bt->next;
				}
			}
		}


		/* if there's no head pointer, set it up */
		if( !bp->firstData ) {
			bp->firstData = bl->nextData;
		}
		bl = bl->next;
	}


	/* Set up the currentData pointer to the appropriate place */

	/* 0 is a special item meaning the first thing on the list */
	if( line == 0 ) {
		bp->currentData = bp->firstData;
		return;
	}

	/* go to the selected place */
	bl = findLineNumber( bp, line );
	if( !bl ) 
	{
		/* line number doesn't exist */
		errorReport( kErrorNoLine, 112 );
		stopProgram( bp );
	}

	/* set the data pointer */
	while( bl ) {
		if( bl->nextData ) {
			bp->currentData = bl->nextData;
			return;
		}
		bl = bl->next;
	}
}


/* run_actual
 *
 *	start running a program on the specified line (or the next one)
 */
void run_actual( basic_program * bp, long startline )
{
	basic_line * r;

	if( !bp ) return;

	/* rebuild the labels */
	rebuild_labels( bp );

	/* set up the DATA list */
	restore_actual( bp, 0 );

	/* clear for-notes */
	clearVariableAux( bp->variables );

	/* find the actual line */
	r = findLowestLineNumber( bp, startline );

	/* and start running! */
	bp->runningLine = r;
	if( bp->runningLine ) {
		bp->nextLineNumberToExecute = bp->runningLine->lineNumber;
	}
}


/* goto_actual
 *
 *	the actual stuff necessary to do a goto call
 */
void goto_actual( basic_program * bp, long newLine )
{
	bp->nextLineNumberToExecute = newLine;

	if( !findLineNumber( bp, bp->nextLineNumberToExecute ))
	{
		errorReport( kErrorNoLine, 112 );
		stopProgram( bp );
	}
}


/* gosub_actual
 *
 *	the actual stuff necessary to do a gosub call
 */
void gosub_actual( basic_program * bp, long newLine )
{
	bp->nextLineNumberToExecute = newLine;

	/* make sure we haven't reached max depth for GOSUBs */
	if( bp->nGosubLevels +1 >= kGosubLevels ) {
		errorReport( kErrorGosub, 114 );
		stopProgram( bp );
		return;
	}

	/* our current line onto the list  */
	bp->gosubList[bp->nGosubLevels] = bp->runningLine;
	bp->nGosubLevels++;

	/* and jump to the requested line */
	if( !findLineNumber( bp, bp->nextLineNumberToExecute ))
	{
		errorReport( kErrorNoLine, 115 );
		stopProgram( bp );
	}
}




/* ********************************************************************** */

/* cmd_load
 *
 * 	load a file in to memory
 * 	clear out the old one before loading
 */
void cmd_load( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TQE", "TQ,0E", NULL };
	int pm;
	int autorun = 0;

	char * fn;
	char * fullpath;
	
	if( !bp || !bl ) return;

	if( bp->runningLine ) {
		errorReport( kErrorRunning, 109 );
		return;
	}

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* TQE */
		fn = argvChar( bl, 1 );
		break;
	case( 1 ): /* TQ,0E */
		if( argvLong( bl, 3 ) != 1 ) {
			errorReport( kErrorParams, 109 );
			return;
		}
		fn = argvChar( bl, 1 );
		autorun = 1;
		break;

	default:
		errorReport( pm, 109 );
		return;
	}


	fullpath = getFilePathInDocuments( fn );
	if( !load_actual( bp, fullpath ) ) {
		cmd_infosteps( bp, NULL );

		if( autorun ) {
			run_actual( bp, 0 );
		}
		
	}
	free( fullpath );
}



/* cmd_save
 *
 * 	save out the current program to a file
 */
void cmd_save( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TQE", NULL };
	int pm;

	char * fn;
	char * fullpath;
	
	if( !bp || !bl ) return;

	if( bp->runningLine ) {
		errorReport( kErrorRunning, 109 );
		return;
	}

	if( !bp->listing ) {
		errorReport( kErrorProgramEmpty, 104 );
		return;
	}


	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* TQE */
		fn = argvChar( bl, 1 );
		break;

	default:
		errorReport( pm, 105 );
		return;
	}


	if( !fn )
	{
		errorReport( kErrorParamType, 106 );
		return;
	}

	fullpath = getFilePathInDocuments( fn );
	save_actual( bp, fullpath );
	free( fullpath );
}


/* ********************************************************************** */



/* cmd_new
 *
 * 	clear the program memory
 */
void cmd_new( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 105 );
		return;
	}

	new_actual( bp );
}


/* renumberParameters
 *	renumber goto and gosubs
 */
void renumberParameters( basic_program * bp, long oldNumber, long newNumber, int temp )
{
	basic_line * bl;
	basic_token * tt;

	if( !bp ) return;
	if( oldNumber == newNumber ) return;

	bl = bp->listing;

	while( bl ) {
		/* TODO: NOTE:
			Need to handle continuations
		*/

		tt = bl->tokens;
		/* look for GOSUB 0x8d  and GOTO 0x89 */
		if( tt )
		{
			if( tt->token_id == 0x8d /* GOSUB */ 
			 || tt->token_id == 0x89 /* GOTO */ )
			{
				tt = tt->next;
				if( tt ) {
					if( tt->l_data == oldNumber && bl->temp != temp )
					{
						bl->temp = temp;
						tt->l_data = newNumber;
					}
				}
			}

			/* TODO: add support for "ON" */
		}

		bl = bl->next;
	}
}

/* cmd_renum
 *
 * 	renumber all lines
 */
void cmd_renum( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", "T0E", "T00E", "T0,0E", NULL };
	int pm;

	static int renumTemp = 3;

	basic_line * cl;

	long count = 0;
	long start = 100;
	long step = 10;
	long newLineNo;
	long oldLineNo;

	if( !bp || !bp->listing ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* TE */
		start = 100;
		step = 10;
		break;

	case( 1 ): /* T0E */
		start = argvLong( bl, 1 );
		break;

	case( 2 ): /* T00E */
		start = argvLong( bl, 1 );
		step = argvLong( bl, 2 );
		break;

	case( 3 ): /* T0,0E */
		start = argvLong( bl, 1 );
		step = argvLong( bl, 3 );
		break;

	default:
		errorReport( pm, 109 );
		return;
	}

	/* make sure our numbers make sense */
	if( start <= 0 || step < 1 ) {
		errorReport( kErrorParams, 108 );
		return;
	}

	renumTemp++;	/* so that we know which have been renumbered */

	newLineNo = start;
	cl = bp->listing;
	while( cl ) {
		oldLineNo = cl->lineNumber;
		cl->lineNumber = newLineNo;
		
		renumberParameters( bp, oldLineNo, newLineNo, renumTemp );

		count++;

		newLineNo += step;
		cl = cl->next;
	}
	printf( "%ld lines renumbered.\n", count );
}

/* cmd_list
 *
 * 	display a program listing
 */
void cmd_list( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", "T0E", "T0-E", "T-0E", "T00E", "T0-0E", NULL };
	int pm;

	long min = -1;
	long max = -1;
	int showThis = 1;

	char buf[1024];
	basic_line * ll;

	if( !bp || !bp->listing );


	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* TE */
		min = max = -1;
		break;

	case( 1 ): /* T0E */
		min = max = argvLong( bl, 1 );
		break;

	case( 2 ): /* T0-E */
		min = argvLong( bl, 1 );
		break;

	case( 3 ): /* T-0E */
		max = argvLong( bl, 2 );
		break;

	case( 4 ): /* T00E */
		min = argvLong( bl, 1 );
		max = argvLong( bl, 2 );
		break;

	case( 5 ): /* T0-0E */
		min = argvLong( bl, 1 );
		max = argvLong( bl, 3 );
		break;

	default:
		errorReport( pm, 109 );
		return;
	}

	ll = bp->listing;
	while( ll ) {
		showThis = 1;
		if( min > 0 ) {
			if( ll->lineNumber < min ) showThis = 0;
		}
		if( max > 0 ) {
			if( ll->lineNumber > max ) showThis = 0;
		}

		if( showThis ) {
			stringizeLine( ll, buf, 1024 );
			printf( "%s\n", buf );
		}

		ll = ll->next;
	}
	printf( "\n" );
}


/* cmd_head
 *
 *	print out the first 5 lines of a program to see if we want to load it
 */
void cmd_head( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	char buf[1024];
	int lineCounter = 0;
	FILE * fp;
	char * fn;
	char * fullpath;

	/* determine the file to check. */
	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		fn = argvChar( bl, 1 );
		break;
	default:
		errorReport( pm, 201 );
		return;
	}


	if( !fn )
	{
		errorReport( kErrorParamType, 202 );
		return;
	}

	/* get the full path in the documents folder */
	fullpath = getFilePathInDocuments( fn );
	fp = fopen( fullpath, "r" );

	/* make sure it is readable */
	if( !fp ) {
		errorReport( kErrorFileNotFound, 203 );
		free( fullpath );
		return;
	}
	
	/* print out the first 5 lines. */
	printf( "%s:\n", fn );
	while( fgets( buf, 1024, fp ) && lineCounter < 5 )
	{
		printf( "%02d: %s", lineCounter, buf );
		lineCounter++;
	}
	fclose( fp );
	if( lineCounter >= 5 ) printf( "..." );
	printf( "\n" );
	
	free( fullpath );
}


/* cmd_files
 *
 *      listing of the available documents
 */
void cmd_files( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	char * directory = kDocumentsDirectory;
	DIR *dfp;
	struct stat st;
	struct dirent *de;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 201 );
		return;
	}

	/* do the listing... */
	dfp = opendir( directory );

	if( !dfp ) {
		errorReport( kErrorDirectory, 201 );
		return;
	}

	while( (de = readdir( dfp )) )
	{
		int strsz = strlen( directory ) + strlen( de->d_name ) + 5;
		char * fullpath = (char *) malloc( sizeof( char ) * strsz );

		snprintf( fullpath, strsz, "%s/%s", directory, de->d_name );

		stat( fullpath, &st );
		if( st.st_mode & S_IFREG )
		{
#ifdef NEVER
			/* sinclair style */
			printf( " %-15s : %ld bytes\n", de->d_name, (long) st.st_size );
#endif

			/* commodore 64 style */
			printf( " %-8ld \"%s\"\n", (long)st.st_size, de->d_name );
		}
	}
	closedir( dfp );
}



/* cmd_help
 *
 * 	print out a list of available commands
 */
void cmd_help( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;
	int pos = 0;

	
	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		break;

	default:
		errorReport( pm, 201 );
		return;
	}

	while( token_lut[pos].cmd != NULL )
	{
		if( token_lut[pos].helpText ) {
			printf( "%10s .. %s\n", token_lut[pos].cmd, token_lut[pos].helpText );
		}
		pos++;
	}

}



/* cmd_info
 * 
 *	Display statistics about the system
 */
void cmd_info( basic_program * bp, basic_line * bl )
{
	printf( "\n" );
        printf( "BleuLlama BASIC version %s (%s)\n", BASIC_VERSION, BASIC_BUILDNAME );
        printf( "Copyright 2011 by Scott Lawrence\n" );

	cmd_infosteps( bp, bl );
}

void cmd_bye( basic_program * bp, basic_line * bl )
{
	printf( "Exiting...\n" );
	if( bp ) bp->exitNow = 1;
}


/* ********************************************************************** */


/* cmd_end
 *
 * 	terminate program running
 */
void cmd_end( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm <0 ) {
		errorReport( pm, 110 );
	}

	stopProgram( bp );
}


/* cmd_run
 *
 *	start execution of the program
 */
void cmd_run( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", "T0E", NULL };
	int pm;

	long startLine = 0;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		startLine = 0;
		break;

	case( 1 ):
		startLine = argvLong( bl, 1 );
		break;

	default:
		errorReport( pm, 110 );
		break;
	}

	/* startLine contains the lowest line number to start on */
	run_actual( bp, startLine );
}


/* cmd_goto
 *
 *	move execution elsewhere
 */
void cmd_goto( basic_program * bp, basic_line * bl )
{
	char * params[] = { "T0E", "TAE", NULL };
	int pm;
	long newLine;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		newLine = argvLong( bl, 1 );
		break;

	case( 1 ):
		newLine = getVariableNumber( bp->variables, argvChar( bl, 1 ));
		break;

	default:
		errorReport( kErrorParams, 111 );
		stopProgram( bp );
		return;
	} 

	goto_actual( bp, newLine );
}


/* cmd_gosub
 *
 *	call a subroutine
 */
void cmd_gosub( basic_program * bp, basic_line * bl )
{
	char * params[] = { "T0E", "TAE", NULL };
	int pm;
	long newLine;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		newLine = argvLong( bl, 1 );
		break;

	case( 1 ):
		newLine = getVariableNumber( bp->variables, argvChar( bl, 1 ));
		break;

	default:
		errorReport( pm, 113 );
		stopProgram( bp );
		return;
	} 

	gosub_actual( bp, newLine );
}


/* cmd_on
 *
 * 	ON x GOTO 1,2,3
 * 	ON x GOSUB 1,2,3
 */
void cmd_on( basic_program * bp, basic_line * bl )
{
	basic_token * toList;
	char * params[] = { "TAT", "TA0", NULL };
	int pm;
	int tosub; /* goto or gosub */
	long userIdx;
	long toLoc = 0;


	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		/* ON x GOTO Y,Z,A */
		/* ON x GOSUB Y,Z,A */
		userIdx = valueFromToken( argvToken( bl, 1 ), bp->variables );

		tosub = argvTokenID( bl, 2 );

		if( tosub == 0x89 ) {
			/* GOTO */
		} else if( tosub == 0x8d ){
			/* GOSUB */
		} else {
			/* NOPE! */
                	errorReport( kErrorParams, 140 );
			return;
		}
		toList = argvToken( bl, 3 );

		
		break;

	case( 1 ):
		/*  ON x Y,Z,A */
		userIdx = valueFromToken( argvToken( bl, 1 ), bp->variables );
		tosub = 0x89;	/* force GOTO */
		toList = argvToken( bl, 2 );
		break;

        default:
		/* ERROR */
                errorReport( pm, 140 );
                stopProgram( bp );
                return;
        }

	userIdx -= 1;

	while( toList ) {
		/* where we're going to for this element */
		toLoc = valueFromToken( toList, bp->variables );

		if( userIdx <= 0 ) {
			break;
		}

		userIdx--;

		/* advance to the next one - ignore the , tokens. */
		/* don't even bother checking them. */
		toList = toList->next;
		if( toList ) toList = toList->next;
	}

	if( toList == NULL || userIdx != 0 ) {
		/* past the end or before the beginning */
		/*
                errorReport( kErrorNoOn, 140 );
                stopProgram( bp );
		*/
		/* just bail out... it's not an error */
		return;
	}

	if( tosub == 0x89 ) { /* goto */
		goto_actual( bp, toLoc );
	} else if( tosub == 0x8d ){ /* gosub */
		gosub_actual( bp, toLoc );
	}
}


/* cmd_return
 *
 *	resume from a gosub
 */
void cmd_return( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp || !bl ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 116 );
                stopProgram( bp );
		return;
	}

	/* make sure we have someplace to go */
	if( bp->nGosubLevels == 0 ) {
                errorReport( kErrorReturn, 117 );
                stopProgram( bp );
                return;
	}

	/* restore our location */
	bp->nGosubLevels --;
	bp->runningLine = bp->gosubList[ bp->nGosubLevels];
	bp->nextLineNumberToExecute = -1;		/* just in case */
}


/* cmd_for
 *
 * 	iterate through a variable
 *
 * 	FOR VAR = VALUE TO VALUE 
 * 	FOR VAR = VALUE TO VALUE STEP VALUE
 * 	where VALUE is an immediate number or variable
 */
void cmd_for( basic_program * bp, basic_line * bl )
{
	basic_variable * bv;
	int argc;

	char * bvar;
	long curr = 0;
	long start = 0;
	long end = 0;
	long step = 1;

	/* TO 0xa4	STEP 0xa9 */

	/*                   01234567 */
	char * params[] = { "TA=ATA", 
			    "TA=AT0",
			    "TA=0TA",
			    "TA=0T0",
	/*                       ^TO     0xA4 */
	/*                         ^STEP 0xA9 */
				NULL };
	int pm;

	if( !bp || !bl ) return;

	argc = lineArgc( bl );
	if( argc != 6 && argc != 8 ) {
		errorReport( kErrorParams, 115 );
		return;
	}

	pm = argvMatch( bl->tokens, params );
	switch( pm ){
	case( 0 ):
	case( 1 ):
	case( 2 ):
	case( 3 ):
		/* 0: FOR 
 		 * 1:  (var)
 		 * 2: =
 		 * 3:  (start)
 		 * 4: TO
 		 * 5:  (end)
 		 * 6: STEP	(optional)
 		 * 7:  (step)	(optional)
 		 */
		bvar = argvChar( bl, 1 );

		start = valueFromToken( argvToken( bl, 3), bp->variables );

		if( argvTokenID(bl, 4) != 0xA4 ) {
			errorReport( kErrorParams, 115 );
			return;
		}
		end = valueFromToken( argvToken( bl, 5), bp->variables );

		if( argc == 8 ) {
			if( argvTokenID( bl, 6 ) != 0xa9 ) {
				errorReport( kErrorParams, 115 );
				return;
			}

			step = valueFromToken( argvToken( bl, 7 ), bp->variables );
		}
		break;

	case( 4 ):
	default:
                errorReport( pm, 117 );
                stopProgram( bp );
	}

	
	/* set the variable->forStartLine to this line */

	/* check to see if we're doing this for the first time through */
	bv = findVariable( bp->variables, bvar );
	if( !bv )
	{
		/* it doesn't exist. make it first. */
		bp->variables = setVariableNumber( bp->variables, bvar, start );
		/* we're obviously in the first time through now, but re=find it anyway */
		bv = findVariable( bp->variables, bvar );
	}

	/* okay, now we check bv */
	if( bv->forStartLine ) {
		/* we've been through here, adjust things */
		curr = getVariableNumber( bp->variables, bvar );
		curr += step;
		bp->variables = setVariableNumber( bp->variables, bvar, curr );
	} else {
		/* start it! */
		bp->variables = setVariableNumber( bp->variables, bvar, start );
		bv->forStartLine = bl->lineNumber;
	}

	if( curr == end ) {
		/* end */
		bv->forStartLine = 0;

		/* we fall out of the loop at the NEXT */
	}
}


/* cmd_next
 *
 * 	end of a FOR block
 */
void cmd_next( basic_program * bp, basic_line * bl )
{
	basic_variable * bv;
	char * bvar;
	char * params[] = { "TAE", NULL };
	int pm;

	if( !bp || !bl ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 116 );
                stopProgram( bp );
		return;
	}

	/* okay, now pull the variable name, and set the */
	bvar = argvChar( bl, 1 );
	bv = findVariable( bp->variables, bvar );
	if( !bv ) {
		errorReport( kErrorNext, 117 );
		stopProgram( bp );
		return;
	}

	if( bv->forStartLine != 0 ) {
		goto_actual( bp, bv->forStartLine );
	}
}


/* cmd_if
 *
 *	conditionally do stuff
 * 		IF v = w THEN       x		-- TI
 * 		IF v = w THEN GOTO  x		-- Sinclair
 * 		IF v = w THEN GOSUB x
 *              0  1 2 3 4    5     6
 *
 * ATARI basic seems to have no ELSE
 * TI basic only allows GOTOs
 */
void cmd_if( basic_program * bp, basic_line * bl )
{
	char * params[] = {
			"TAT0T0E", 	/* IF a ?= 30 THEN 140 */
			"TAT0TAE", 	/* IF a ?= 30 THEN foobar */
			"TATAT0E", 	/* IF a ?= b  THEN 140 */
			"TATATAE", 	/* IF a ?= b  THEN foobar */

			"TA=0T0E", 	/* IF a = 30 THEN 140 */
			"TA=0TAE", 	/* IF a = 30 THEN foobar */
			"TA=AT0E", 	/* IF a = b  THEN 140 */
			"TA=ATAE", 	/* IF a = b  THEN foobar */

			"TAT0TT0E", 	/* IF a ?= 30 THEN GOTO/GOSUB 140 */
			"TAT0TTAE", 	/* IF a ?= 30 THEN GOTO/GOSUB foobar */
			"TATATT0E", 	/* IF a ?= b  THEN GOTO/GOSUB 140 */
			"TATATTAE", 	/* IF a ?= b  THEN GOTO/GOSUB foobar */

			"TA=0TT0E", 	/* IF a = 30 THEN GOTO/GOSUB 140 */
			"TA=0TTAE", 	/* IF a = 30 THEN GOTO/GOSUB foobar */
			"TA=ATT0E", 	/* IF a = b  THEN GOTO/GOSUB 140 */
			"TA=ATTAE", 	/* IF a = b  THEN GOTO/GOSUB foobar */

/*			 ^ ^-comparison
			 IF
*/
			    NULL };
	int pm;

	long testValue = 0;
	int comparisonTokenID = 0;
	long compareValue = 0;
	int isThen = 0;
	int isGoto = 1;
	long destLine = 0;

	int trigger = 0;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):	/* IF a ?= 30 THEN 140 */
	case( 1 ):	/* IF a ?= 30 THEN label */
	case( 2 ):	/* IF a ?= b  THEN 140 */
	case( 3 ):	/* IF a ?= b  THEN label */
	case( 4 ):	/* IF a = 30 THEN 140 */
	case( 5 ):	/* IF a = 30 THEN label */
	case( 6 ):	/* IF a = b  THEN 140 */
	case( 7 ):	/* IF a = b  THEN label */
		testValue = valueFromToken( argvToken( bl, 1 ), bp->variables );
		comparisonTokenID = argvTokenID( bl, 2 );
		compareValue = valueFromToken( argvToken( bl, 3 ), bp->variables );
		if( argvTokenID( bl, 4 ) == 0xa7 ) isThen = 1;
		isGoto = 1;
		destLine = valueFromToken( argvToken( bl, 5 ), bp->variables );
		break;

	case( 8 ):	/* IF a ?= 30 THEN GOTO/GOSUB 140 */
	case( 9 ):	/* IF a ?= 30 THEN GOTO/GOSUB label */
	case( 10 ):	/* IF a ?= b  THEN GOTO/GOSUB 140 */
	case( 11 ):	/* IF a ?= b  THEN GOTO/GOSUB label */
	case( 12 ):	/* IF a = 30 THEN GOTO/GOSUB 140 */
	case( 13 ):	/* IF a = 30 THEN GOTO/GOSUB label */
	case( 14 ):	/* IF a = b  THEN GOTO/GOSUB 140 */
	case( 15 ):	/* IF a = b  THEN GOTO/GOSUB label */
		testValue = valueFromToken( argvToken( bl, 1 ), bp->variables );
		comparisonTokenID = argvTokenID( bl, 2 );
		compareValue = valueFromToken( argvToken( bl, 3 ), bp->variables );
		if( argvTokenID( bl, 4 ) == 0xa7 ) isThen = 1;
		if( argvTokenID( bl, 5 ) == 0x8d ) isGoto = 0; /* gosub is 8d goto is 89 */
		destLine = valueFromToken( argvToken( bl, 6 ), bp->variables );
		break;

	default:
		errorReport( pm, 116 );
		return;
	}

	if( !isThen ) {
		errorReport( kErrorParams, 117 );
		return;
	}

	trigger = 0;
	switch( comparisonTokenID ) {
	case( 0x0507 ):  if( testValue <= compareValue ) trigger = 1; break;
	case( 0x0508 ):  if( testValue >= compareValue ) trigger = 1; break;
	case( 0x0509 ):  if( testValue != compareValue ) trigger = 1; break;

	case( 0x00b1 ):  if( testValue > compareValue ) trigger = 1; break;
	case( 0x00b2 ):  if( testValue == compareValue ) trigger = 1; break;
	case( 0x00b3 ):  if( testValue < compareValue ) trigger = 1; break; 
	default:
		errorReport( kErrorParams, 118 );
		return;
	}

	if( !trigger ) return;

	if( isGoto ) {
		goto_actual( bp, destLine );
	} else {
		gosub_actual( bp, destLine );
	}
}


/* cmd_tron
 *
 *	(debug and) trace on
 */
void cmd_tron( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 116 );
                stopProgram( bp );
		return;
	}

	bp->traceOn = 1;
	printf( "Trace on.\n" );
}


/* cmd_troff
 *
 *	(debug and) trace off
 */
void cmd_troff( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm != 0 ) {
		errorReport( pm, 116 );
                stopProgram( bp );
		return;
	}

	bp->traceOn = 0;
	printf( "Trace off.\n" );
}


/* ********************************************************************** */

/* cmd_print
 *
 *	print stuff to the console
 */
void cmd_print( basic_program * bp, basic_line * bl )
{
	basic_token * bt;
	int addNewline = 1;
	int vt;

	if( !bp || !bl ) return;

	bt = bl->tokens->next;

	/* okay.. let's walk down the token list */
	while( bt ) {
		if( bt->token_id == kToken_Semicolon ) {
			addNewline = 0;

		} else if( bt->token_id == kToken_Comma ) {
			printf( "\t" );
			addNewline = 1;

		} else if( bt->token_id == kToken_Number ) {
		    	printf( "%ld", bt->l_data );
			addNewline = 1;

		} else if( bt->token_id == kToken_Quote ) {
			if( bt->c_data ) printf( "%s", bt->c_data );
			addNewline = 1;

		} else if( bt->c_data) {
			/* probably a variable or expression */
			vt = getVariableType( bt->c_data );
			if( vt == kVar_String ) printf( "%s", getVariableString( bp->variables, bt->c_data ));
			else if( vt == kVar_Number ) printf( "%ld", getVariableNumber( bp->variables, bt->c_data ));
			else printf( " ?? " );
			
			addNewline = 1;
		} else {
			printf( " ?%04x? ", bt->token_id );
			addNewline = 1;
		}
		bt = bt->next;
	}

	if( addNewline ) {
		printf( "\n" );
	}
	fflush( stdout );
}


/* cmd_input
 *
 *	prompt the user for input to a variable
    INPUT A$			Apple	C64	Casio	ZX81	TI		\n? ___
    INPUT "Enter Info",A	Apple		Casio				\nEnter Info? ___
    INPUT "Enter Info":A					TI		\nEnter Info? ___
    INPUT A,B,C			Apple						\N? ___
	NOTE: TI does not have a space between ? and ___ (user input)

	OUR IMPLEMENTATION:

	INPUT A
	INPUT A$
	INPUT "FOO";A
	INPUT "FOO";A$

 */
void cmd_input( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TAE", "TQ;AE", NULL };
	int pm;

	char * userPrompt = "";
	char * userInput = NULL;
	char * variableName = "UNK";

	if( !bp || !bl ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		variableName = argvChar( bl, 1 );
		break;

	case( 1 ):
		userPrompt = argvChar( bl, 1 );
		variableName = argvChar( bl, 3 );
		break;

	default:
		errorReport( pm, 301 );
		return;
	}

	/* now, display the prompt to the user. */
	printf( "%s? ", userPrompt ); fflush( stdout );
	userInput = getLineFromUser();
	stringChomp( userInput );

	/* store it */
	bp->variables = setVariableSmart( bp->variables, variableName, userInput );

	/* clean up */
	free( userInput );
}

/* ********************************************************************** */
/* Data */



/* cmd_restore
 *
 *	restore the data pointer to a specific line
 */
void cmd_restore( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", "T0E", "TAE", NULL };
	int pm;

	if( !bp ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* RESTORE */
		restore_actual( bp, 0 );
		break;

	case( 1 ): /* RESTORE 430 */
	case( 2 ): /* RESTORE foo */
		restore_actual( bp, valueFromToken( argvToken( bl, 1 ), bp->variables ));
		break;

	default:
		errorReport( pm, 400 );
		return;
	}
}


/* cmd_read
 *
 *	read the next data item to the supplied variable
 */
void cmd_read( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TAE", NULL };
	int pm;

	if( !bp ) return;

	if( !bp->firstData ) {
		/* assume we've got a program loaded and haven't runit yet.
		   set up the data list for immediate mode */
		restore_actual( bp, 0 );
	}

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ): /* READ foo    READ foo$ */
		if( bp->currentData ) {
			/* store it */
			char * dval = argvChar( bl, 1 );

			if( getVariableType( dval ) == kVar_String ) {
				bp->variables = setVariableString(
							bp->variables,
							argvChar( bl, 1 ),
							bp->currentData->c_data );
			} else {
				bp->variables = setVariableNumber(
							bp->variables,
							argvChar( bl, 1 ),
							bp->currentData->l_data );
			}

			/* advance to the next data token */
			bp->currentData = bp->currentData->nextData;
		} else {
			errorReport( kErrorDataRead, 400 );
		}

		break;
	default:
		errorReport( pm, 400 );
		return;
	}
}


void cmd_data( basic_program * bp, basic_line * bl )
{
	/* I think we'll just assume that it's set up properly.
	   It will never be outright "run" anyway. */

	/* do nothing, but do it a lot. */
}


/* ********************************************************************** */
/* Variables */

/* cmd_let
 *
 * 	handle an assignment 
 */
void cmd_let( basic_program * bp, basic_line * bl )
{
	basic_token * bt;
	if( !bp || !bl ) return;

	bt = bl->tokens;

	/* advance past the "LET" keyword -- it's surperfluous */
	if( !bt || !bt->next ) return;
	bt = bt->next;

	/* and evaluate it */
	evaluateExpression( bp, bt );
}


/* cmd_vac
 * 
 * 	clear all variables
 */
void cmd_vac( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", "TAE", NULL };
	int pm;

	if( !bp || !bp->variables ) return;

	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		bp->variables = deleteVariables( bp->variables, 0 );
		break;

	case( 1 ):
		if( !strcmp( argvChar( bl, 1 ), "ALL" )){
			bp->variables = deleteVariables( bp->variables, 1 );
		} else {
			errorReport( kErrorParams, 400 );
		}
		break;

	default:
		errorReport( pm, 400 );
		return;
	}
}


/* cmd_variables
 * 
 * 	dump a list of all variables
 */
void cmd_variables( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TE", NULL };
	int pm;

	if( !bp || !bp->variables ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm < 0 ) {
		errorReport( pm, 400 );
		return;
	}
	
	dumpVariables( bp->variables );
}


/* cmd_swap
 *
 *	swap two variables
 */
void cmd_swap( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TAAE", "TA,AE", NULL };
	int pm;
	char * v1;
	char * v2;
	int t1,t2;

	/* first, get the parameters */
	pm = argvMatch( bl->tokens, params );
	switch( pm ) {
	case( 0 ):
		v1 = argvChar( bl, 1 );
		v2 = argvChar( bl, 2 );
		break;
	case( 1 ):
		v1 = argvChar( bl, 1 );
		v2 = argvChar( bl, 3 );
		break;
	default:
		errorReport( pm, 400 );
		return;
	}
	
	/* next, check their types */
	t1 = getVariableType( v1 );
	t2 = getVariableType( v2 );
	if( t1 != t2 && t1 != kVar_Undefined ) {
		errorReport( kErrorParamType, 401 );
		return;
	}

	/* check their protection */
	if( isProtectedVariable( bp->variables, v1 ) || isProtectedVariable( bp->variables, v2 ))
	{
		errorReport( kErrorNonsense, 404 );
		return;
	}

	/* finally... swap */
	if( t1 == kVar_String ) {
		char * cv1 = strdup( getVariableString( bp->variables, v1 ));
		char * cv2 = strdup( getVariableString( bp->variables, v2 ));

		bp->variables = setVariableString( bp->variables, v1, cv2 );
		bp->variables = setVariableString( bp->variables, v2, cv1 );

		free( cv1 );
		free( cv2 );
	}
	if( t1 == kVar_Number ) {
		long lv1 = getVariableNumber( bp->variables, v1 );
		long lv2 = getVariableNumber( bp->variables, v2 );

		bp->variables = setVariableNumber( bp->variables, v1, lv2 );
		bp->variables = setVariableNumber( bp->variables, v2, lv1 );
	}
}


/* cmd_protect
 *
 *	protects a variable (readonly)
 */
void cmd_protect( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TAE", NULL };
	int pm;

	if( !bp || !bl ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm < 0 ) {
		errorReport( pm, 400 );
		return;
	}

	bp->variables = protectVariable( bp->variables, argvChar( bl, 1 ), 1 );
}


/* cmd_unprotect
 *
 *	unprotects a variable (read-write)
 */
void cmd_unprotect( basic_program * bp, basic_line * bl )
{
	char * params[] = { "TAE", NULL };
	int pm;

	if( !bp || !bl ) return;

	pm = argvMatch( bl->tokens, params );
	if( pm < 0 ) {
		errorReport( pm, 400 );
		return;
	}

	bp->variables = protectVariable( bp->variables, argvChar( bl, 1 ), 0 );
}



/* ********************************************************************** */
/* Token Lookup Table */


/* Bare minimum commands/operators:
 * LOAD, SAVE, DATA, READ
 * RUN, END, IF, THEN, FOR, NEXT, PRINT, INPUT, :
 * +, -, *, /, ^, AND, OR, <, <=, =, >=, >
 * PEEK, POKE
 *
 * although ':' is a tokenizer thing.
 */

basic_tlut token_lut[] = {
	/* NOTE: Based on CBM BASIC */
	/* 80-A2 Action Keywords
 	 * B4-CA function keywords
 	 * AA-B3 are BASIC operators
 	 * F0-ff are my additions
 	 */
	/* Action keywords */
	{ "END",	0x80, cmd_end,		"Terminate a running program" },
	{ "FOR",	0x81, cmd_for,		"Iterate via a variables" },
	{ "NEXT",	0x82, cmd_next,		"End of a \"for\" iterative block" },
	{ "DATA", 	0x83, cmd_data,		"Comma separated list of data items" },
	{ "INPUT#",	0x84 },
	{ "INPUT",	0x85, cmd_input,	"Input from the user to a variable" },
	{ "DIM",	0x86 },
	{ "READ",	0x87, cmd_read,		"Read the next item from the data list" },

	{ "LET",	0x88, cmd_let,		"Assign a variable a value" },
	{ "GOTO",	0x89, cmd_goto,		"Continue on the specified line" },
	{ "RUN",	0x8a, cmd_run,		"Run the loaded program" },
	{ "IF",		0x8b, cmd_if,		"Conditionally goto a new line" },
	{ "RESTORE",	0x8c, cmd_restore,	"Adjust the DATA-READ list location" },
	{ "GOSUB",	0x8d, cmd_gosub,	"Call a subroutine" },
	{ "RETURN",	0x8e, cmd_return,	"Return from a subroutine" },
	{ "REM",	0x8f, NULL,		"REMark. (comment)" },

	{ "STOP",	0x90 },
	{ "ON",		0x91, 	cmd_on,		"Switch for GOTO and GOSUB" },
	{ "WAIT",	0x92 },
	{ "LOAD",	0x93, 	cmd_load,	"Load the specified \"program\"" },
	{ "SAVE",	0x94, 	cmd_save,	"Save the program to disk" },
	{ "VERIFY",	0x95 },
	{ "DEF",	0x96 },
	{ "POKE",	0x97 },

	{ "PRINT#",	0x98 },
	{ "PRINT",	0x99, 	cmd_print,	"Print out an expression" },
	{ "CONT",	0x9a },
	{ "LIST",	0x9b,	cmd_list,	"List the current program" },
	{ "CLR",	0x9c },
	{ "CMD",	0x9d },
	{ "SYS",	0x9e },
	{ "OPEN",	0x9f },
	{ "CLOSE",	0xa0 },
	{ "GET",	0xa1 },
	{ "NEW",	0xa2, 	cmd_new,	"Erase program ram" },

	/* Misc functions */
	{ "TAB(",	0xa3 },
	{ "TO",		0xa4 },
	{ "FN",		0xa5 },
	{ "SPC(",	0xa6 },
	{ "THEN",	0xa7 },
	{ "NOT",	0xa8 },
	{ "STEP",	0xa9 },

	/* operators */
	{ "+",		0xaa },
	{ "-",		0xab },
	{ "*",		0xac },
	{ "/",		0xad },
	{ "^",		0xae },
	{ "AND",	0xaf },
	{ "OR",		0xb0 },

	{ "<=", 	0x0507 },	/* less than or equal */
	{ ">=", 	0x0508 },	/* greater than or equal */
	{ "<>", 	0x0509 },	/* not equal */

	{ ">",		0xb1 },		/* greater than */
	{ "=",		0xb2 },		/* equal */
	{ "<",		0xb3 },		/* less than */

	/* functions */
	{ "SGN",	0xb4 },
	{ "INT",	0xb5 },
	{ "ABS",	0xb6 },
	{ "USR",	0xb7 },

	{ "FRE",	0xb8 },
	{ "POS",	0xb9 },
	{ "SQR",	0xba },
	{ "RND",	0xbb },
	{ "LOG",	0xbc },
	{ "EXP",	0xbd },
	{ "COS",	0xbe },
	{ "SIN",	0xbf },

	{ "TAN",	0xc0 },
	{ "ATN",	0xc1 },
	{ "PEEK",	0xc2 },
	{ "LEN",	0xc3 },
	{ "STR$",	0xc4 },
	{ "VAL",	0xc5 },
	{ "ASC",	0xc6 },
	{ "CHR$",	0xc7 },

	{ "LEFT$",	0xc8 },
	{ "RIGHT$",	0xc9 },
	{ "MID$",	0xca },

	/* file and disk operations */
	{ "BANK",	0xe1 },		/* select program bank PARAM */

	/* misc */
	{ "PI",		0xff },


	/* my turtle graphics */
	{ "CLS",	0x0100 }, 	/* clear screen */
	{ "COLOR",	0x0101 },	/* set color to PARAM */
		/* Commodore 64 colors:
 		 * 1: black	000
 		 * 2: white	fff
 		 * 3: red	f00
 		 * 4: cyan	0ff
 		 * 5: purple	f0f
 		 * 6: green	0f0
 		 * 7: blue	00f
 		 * 8: yellow	ff0
 		 *
 		 * TI 99/4A colors:
 		 * 1: transparent
 		 * 2: black		000
 		 * 3: medium green	080
 		 * 4: light green	8f8
 		 * 5: dark blue		008
 		 * 6: light blue	88f
 		 * 7: dark red		800
 		 * 8: cyan		0ff
 		 * 9: medium red	f00
 		 * 10: light red	f88
 		 * 11: dark yellow	880
 		 * 12: light yellow	ff0
 		 * 13: dark green	0f0
 		 * 14: magenta		f0f
 		 * 15: gray		888
 		 * 16: white		fff
 		 */
	{ "PD",		0x0102 },	/* pen down (start drawing) */
	{ "PU",		0x0103 },	/* pen up (stop drawing) */
	{ "FD",		0x0104 },	/* forward PARAM px */
	{ "RT",		0x0105 },	/* right rotate PARAM degrees */
	{ "LT",		0x0106 },	/* left rotate PARAM degrees */
	{ "MT", 	0x0107 },	/* move to PARAMX,PARAMY */

	/* floppy diskette commands */
	{ "FILES",	0x0201,	cmd_files,	"List of saved basic program files" },
	{ "CATALOG",	0x0202,	cmd_files,	"List of saved basic program files" },
	{ "HEAD",	0x0203,	cmd_head,	"Display 5 lines of the specified program" },

	/* user commands */
	{ "HELP",	0x0301,	cmd_help,	"Display a list of commands" },
	{ "INFO",	0x0302,	cmd_info,	"Print system information" },
	{ "BYE",	0x0303, cmd_bye,	"Quit out of the interpreter" },
	{ "RENUM",	0x0304, cmd_renum,	"Renumber the basic program lines" },

	{ "TRON",	0x0305, cmd_tron,	"Enable program run tracing" },
	{ "TROFF",	0x0306, cmd_troff,	"Disable program run tracing" },

	{ "VAC",	0x0307, cmd_vac,	"Erase all variables from memory" },
	{ "VARIABLES",	0x0308, cmd_variables,	"Displays all variables in memory" },
	{ "PROTECT",	0x0310, cmd_protect,	"Protects a variable (read-only)" },
	{ "UNPROTECT",	0x0311, cmd_unprotect,	"Unprotects a variable (read-write)" },

	{ "LABEL",	0x0400, NULL,		"Label the line for goto/gosubs" },
	{ "SWAP",	0x0401, cmd_swap,	"Swap two variables" },

	/* anything less than 0x1000 is string tokens with functions (above) */
	/* 0x42xx range is used for token detection of characters (num, space, etc) */

	{ NULL }
};


/*
 * isSameToken
 *	compare the haystack against possible tokens (case insensitive)
 *	returns 0 if it doesn't match
 *	returns 1 if it does match
 */
int isSameToken( char * master, char * haystack )
{
	if( !master || !haystack ) return 0;

	while( *master != '\0' )
	{
		if( tolower( *master ) != tolower( *haystack )) return 0;
		master++;
		haystack++;
	}

	return 1;
}



/* findLutTokenFromID
 *
 * 	find the token node that has the same ID
 *
 */
basic_tlut * findLutTokenFromID( int token )
{
	int tidx = 0;

	while( token_lut[tidx].cmd != NULL )
	{
		if( token_lut[tidx].token == token ) 
		{
			return &token_lut[tidx];
		}
		tidx++;
	}
	return NULL;
}


/* findTokenStringFromID
 *
 *  get the string representation of the passed in token
 */
char * findTokenStringFromID( int token )
{
	basic_tlut * t = findLutTokenFromID( token );

	if( t ) {
		return t->cmd;
	}

	if( token == 10 ) 		return "EOL";
	switch( token ) {
	case( kToken_User ):		return "USER";
	case( kToken_ERROR ):		return "ERR0";
	case( kToken_Alpha ):		return "A";
	case( kToken_Space ):		return "SPC";
	case( kToken_Number ):		return "1";
	case( kToken_Newline ):		return "CR";
	case( kToken_Colon ):		return ":";
	case( kToken_Comma ):		return ",";
	case( kToken_Semicolon ):	return ";";
	case( kToken_Quote ):		return "\"";
	case( kToken_EOS ):		return "0";
	}

	return "UNK?";
}

