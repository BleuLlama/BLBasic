/* basic_errors
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
#include "basic_errors.h"

void errorReportAdditional( int code, int codeMinor, char * additional )
{
	char * msg = "Unknown.";

	switch( code )
	{
	case( kErrorNone ):		msg = "No error.";			break;
	case( kErrorMalloc ):		msg = "Could not allocate memory."; 	break;
	case( kErrorParams ):		msg = "Parameter list incorrect.";	break;
	case( kErrorParamType ):	msg = "A parameter type is incorrect.";	break;
	case( kErrorFileNotFound ):	msg = "File not found.";		break;
	case( kErrorFileError ):	msg = "File IO error.";			break;
	case( kErrorData ):		msg = "Data stream error.";		break;
	case( kErrorProgramEmpty ):	msg = "No program data.";		break;
	case( kErrorNoLine ):		msg = "Unavailable line number.";	break;
	case( kErrorNonsense ):		msg = "Nonsense in BASIC.";		break;
	case( kErrorGosub ):		msg = "GOSUB max depth reached.";	break;
	case( kErrorReturn ):		msg = "RETURN to nowhere.";		break;
	case( kErrorDirectory ):	msg = "Directory is unreadable.";	break;
	case( kErrorRunning ):		msg = "Not a runnable command.";	break;
	case( kErrorNoOn ):		msg = "ON target address not found.";	break;
	case( kErrorNext ):		msg = "For-Next error.";		break;
	case( kErrorDataRead ):		msg = "Out of DATA error.";		break;
	case( kErrorDBZero ):		msg = "Divide by zero error.";		break;

	default:
		break;
	}

	fprintf( stderr, "ERROR %d-%d: %s", -code, codeMinor, msg );
	if( additional )
	{
		fprintf( stderr, "(%s)", additional );
	}
	fprintf( stderr, "\n" );
}

void errorReport( int code, int codeMinor )
{
	errorReportAdditional( code, codeMinor, NULL );
}
