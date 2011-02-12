/* basic_variable
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

#ifndef __BASIC_VARIABLE_H__
#define __BASIC_VARIABLE_H__

#define kVar_Undefined	0
#define kVar_String	1
#define kVar_Number	2

typedef struct basic_variable {
        char * name;            /* the lookup name */
	int protect;		/* 1 if not deletable, readonly */
        int type;               /* see kVar_* above */
        long l_data;
        char * c_data;
        struct basic_variable * next;

	long forStartLine;	/* for using the variable in a FOR */
} basic_variable;


int getVariableType( char * name );
basic_variable * newVariable( char * name );
basic_variable * deleteVariables( basic_variable * list, int force );
void clearVariableAux( basic_variable * list );

void dumpVariables( basic_variable * list );

basic_variable * findVariable( basic_variable * list, char * name );
basic_variable * protectVariable( basic_variable * list, char * name, int protect );
int isProtectedVariable( basic_variable * list, char * name );

basic_variable * setVariableNumber( basic_variable * list, char * name, long value );
basic_variable * setVariableString( basic_variable * list, char * name, char * value );

/* this next one picks the right one, and does atol() if necessary */
basic_variable * setVariableSmart( basic_variable * list, char * name, char * value );

long getVariableNumber( basic_variable * list, char * name );
char * getVariableString( basic_variable * list, char * name );

#endif
