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

#include <stdio.h>
#include <stdlib.h>	/* malloc */
#include <string.h>	/* strdup */
#include "basic_variable.h"

#ifndef __BASIC_VARIABLE_H__
#define __BASIC_VARIABLE_H__

#define kVar_Undefined  0
#define kVar_String     1
#define kVar_Number     2

typedef struct basic_variable {
        char * name;            /* the lookup name */
        int type;               /* see kVar_* above */
        long l_data;
        char * c_data;
        struct basic_variable * next;
} basic_variable;
#endif


/* ********************************************************************** */

/* getVariableType
 *
 * 	determine the type based on the name
 */
int getVariableType( char * name )
{
	if( !name ) return kVar_Undefined;
	if( strlen(name) == 0 ) return kVar_Undefined;
	if( name[strlen(name)-1] == '$' ) return kVar_String;
	return kVar_Number;
}

/* newVaraible
 *
 * 	allocates a new variable and sets its type based on the name
 * 	Name 	-> numerical variable
 * 	Name$	-> string variable
 */
basic_variable * newVariable( char * name )
{
	basic_variable * bv;
 
	if( !name ) return NULL;

	bv = (basic_variable *) malloc( sizeof( basic_variable ));
	bv->name = strdup( name );
	if( name[strlen(name)-1] == '$' ) {
		bv->type = kVar_String;
	} else {
		bv->type = kVar_Number;
	}
	bv->protect = 0;
	bv->l_data = 0;
	bv->c_data = strdup( "00" );
	bv->forStartLine = 0;
	return bv;
}


/* deleteVariables
 *
 * 	delete the entire list of variables
 */
basic_variable * deleteVariables( basic_variable * list, int force )
{
	basic_variable * newList = NULL;
	basic_variable * next;

	while( list ) {
		next = list->next;
		if( list->protect && !force ) {
			/* protect item, don't delete it. prepend it. */
			list->next = newList;
			newList = list;
		} else {
			/* delete it. */
			if( list->c_data ) free( list->c_data );
			if( list->name ) free( list->name );
			free( list );

		}
		list = next;
	}
	return newList;
}


/* clearVariableAux
 *
 * 	clears auxiliary data in the variables structure for runtime
 */
void clearVariableAux( basic_variable * list )
{
	while( list )
	{
		list->forStartLine = 0;
		list = list->next;
	}
}

/* ********************************************************************** */

/* dumpVariables
 *
 * 	print out the list of variables for debugging
 */
void dumpVariables( basic_variable * list )
{
	printf( "Variable List:\n" );
#ifdef DEBUGWORTHY
	while( list )
	{
		switch( list->type ) {
		case( kVar_String ): printf( "STR " ); break;
		case( kVar_Number ): printf( "NUM " ); break;
		case( kVar_Undefined ): printf( "UND " ); break;
		default: printf( "UNK " ); break;
		}

		printf( "%20s \"%s\" %ld\n",
			list->name?list->name:"UNK?", 
			list->c_data?list->c_data:"UNK?",
			list->l_data );

		list = list->next;
	}
#endif

	while( list ) {
		switch( list->type ) {
		case( kVar_String ):
			printf( "%16s : \"%s\" %s\n", 
				list->name?list->name:"ERR", 
				list->c_data?list->c_data:"ERR",
				list->protect?"(protected)":""
				);
			break;
		case( kVar_Number ):
			printf( "%16s : %ld %s\n", 
				list->name?list->name:"ERR", 
				list->l_data,
				list->protect?"(protected)":""
				);
			break;
	
		}
		list = list->next;
	}
}


/* ********************************************************************** */


/* findVariable
 *
 * 	find the specified variable in the list
 * 	returns NULL if not found
 */
basic_variable * findVariable( basic_variable * list, char * name )
{
	basic_variable * el = list;

	while( el ) {
		if( !strcmp( name, el->name )) return el;
		el = el->next;
	}
	return NULL;
}

/* protectVariable
 *
 *	find the variable and protect it.
 *	if it does not exist, create it and protect it.
 */
basic_variable * protectVariable( basic_variable * list, char * name, int protect )
{
	basic_variable * v;

	if( !name ) return list;

	v = findVariable( list, name );
	if( !v ) {
		list = setVariableSmart( list, name, "0" );
		v = findVariable( list, name );
	}

	if( v ) {
		v->protect = protect;
	}
	return list;
}

/* isProtectedVariable 
 *
 *	returns 1 if the variable is protected
 */
int isProtectedVariable( basic_variable * list, char * name )
{
	basic_variable * v;

	if( !name || !list ) return 0;

	v = findVariable( list, name );
	if( v ) {
		return v->protect;
	}
	return 0;
}


/* ********************************************************************** */

/* setVariableNumber
 *
 * 	find and set the variable.  If it doesn't exist yet, allocate it
 */
basic_variable * setVariableNumber( basic_variable * list, char * name, long value )
{
	basic_variable * tn;
	if( !name ) return list;

	/* look for the element */
	tn = findVariable( list, name );

	if( tn ) {
		/* if found, set the value, return list */
		if( !tn->protect ) {
			tn->l_data = value;
		}
	} else {
		/* if not found, allocate, add, set, return list */
		tn = newVariable( name );
		tn->l_data = value;
		tn->next = list;
		list = tn;
	}
	return list;
}


/* setVariableString
 *
 * 	find and set the variable. If it doesn't exist yet, allocate it
 */
basic_variable * setVariableString( basic_variable * list, char * name, char * value )
{
	basic_variable * tn;
	if( !name || !value ) return list;

	/* look for the element */
	tn = findVariable( list, name );

	if( tn ) {
		/* if found, set the value, return list */
		if( !tn->protect ) {
			if( tn->c_data ) free( tn->c_data );
			tn->c_data = strdup( value );
		}
	} else {
		/* if not found, allocate, add, set, return list */
		tn = newVariable( name );
		if( tn->c_data ) free( tn->c_data );
		tn->c_data = strdup( value );
		tn->next = list;
		list = tn;
	}
	return list;
}


/* setVariableSmart
 *
 *	Sets a string or number as appropriate
 */
basic_variable * setVariableSmart( basic_variable * list, char * name, char * value )
{
	int type;
	if( !name || !value ) return list;
	
	type = getVariableType( name );
	if( type == kVar_Number ) {
		return setVariableNumber( list, name, atol( value ));
	}
	return setVariableString( list, name, value );
}


/* getVariableNumber
 *
 * 	find and return the specified number
 */
long getVariableNumber( basic_variable * list, char * name )
{
	basic_variable * tn;
	if( !list || !name ) return 0;

	/* look for the element */
	tn = findVariable( list, name );

	if( tn ) {
		/* if found, return the number */
		return( tn->l_data );
	}

	/* if not found, return 0 */
	return 0;
}


/* getVariableString
 *
 * 	find and return the specified string
 */
char * getVariableString( basic_variable * list, char * name )
{
	basic_variable * tn;
	if( !list || !name ) return "";

	/* look for the element */
	tn = findVariable( list, name );

	if( tn ) {
		/* if found, return the string */
		return( tn->c_data );
	}

	/* if not found, return "" */
	return "";
}
