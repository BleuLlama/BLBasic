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

#ifndef __basic_evaluator_h__
#define __basic_evaluator_h__

#include "basic_tokenizer.h"

#define kDocumentsDirectory     "./programs"    /* directory where we store programs */

/* ********************************************************************** */ 

typedef struct basic_tlut {
	char * cmd;		/* command text to match/generate */
	int token;		/* internal token ID number */
	void (*perform)( basic_program * bp, basic_line * bl );
	char * helpText;	/* text to display for this command */
} basic_tlut;

extern basic_tlut token_lut[]; /* the actual table */

/* ********************************************************************** */ 

/*                             <0x1000     T  */
#define kToken_User             0x1000  /*   anything above this is special use */
#define kToken_ERROR            0x4200  /*    nothing */
#define kToken_Alpha            0x4201  /* V  anything printable */
#define kToken_Space            0x4202  /*    [ \n\t\r] */
#define kToken_Number           0x4203  /* 0  [0123456789] */
#define kToken_Newline          0x4204  /*    \n */
#define kToken_Colon            0x4205  /* :  : */
#define kToken_Semicolon        0x4206  /* ;  ; */
#define kToken_Quote            0x4207  /* Q  " */
#define kToken_Comma		0x4208	/* , 	*/
#define kToken_EOS              0x5000  /* E  \0 */
/* SPECIALS: */
/*             (                           (     */
/*             )                           )     */
#define kToken_Dash		0x00ab	/* - 	 */
#define kToken_Equals		0x00b2	/* =     */

/* ********************************************************************** */

int isSameToken( char * master, char * haystack );
basic_tlut * findLutTokenFromID( int token );
char * findTokenStringFromID( int token );

/* ********************************************************************** */ 

void evaluateLine( basic_program * bp, basic_line * bl );
int run_poll( basic_program * bp );

void runProgram( basic_program * bp, long startline );
void stopProgram( basic_program * bp );


/* ********************************************************************** */ 

void cmd_new( basic_program * bp, basic_line * bl );
void cmd_info( basic_program * bp, basic_line * bl );
void cmd_infosteps( basic_program *bp, basic_line * bl );

#endif
