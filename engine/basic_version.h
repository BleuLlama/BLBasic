/* BleuLlama BASIC
 *
 *   A BASIC interpreter
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


/* ************************************************************ 
 * Version history
 */

#define BASIC_VERSION 		"0.15"
#define BASIC_BUILDNAME		"Intersections"
/*
 * 0.15 2011-Feb-11 "Intersections"
 *	Statements: DATA, READ, RESTORE, SWAP, IF/THEN
 *	IF/THEN only allows for integer compares, not strings
 *
 * 0.14 2011-Feb-10 "Voice"
 *	Babylon 5 (Voice Of The Resistance)
 *	Statements: ON, FOR-NEXT-(STEP)
 *	ON implemented. (ON _ GOTO _,_  ON _ GOSUB _,_ )
 *	LINE CONTINUATIONS!!! :
 *
 * 0.13 2011-Feb-9 "Spark Plug"
 *	Aah Megamisama
 *	Statements: PROTECT, UNPROTECT, VAC ALL, LABEL
 *	LS renamed to FILES (Amiga BASIC) 
 *	CATALOG is the same as FILES (Atari BASIC)
 *	argMatch created to lex parse
 * 	added variable protection
 *	LOAD "filename",1   to automatically run it
 *	comma in PRINT injects a tab. (AmigaBASIC)
 *
 * 0.12 2011-Feb-8 "Take out the Vorlon"
 *	Babylon 5
 *	Statements: HEAD, INPUT
 *	file rearrangement, smarter value storage a,a$ -> auto
 *
 * 0.11 2011-Feb-7 "Pizza Stabs"
 * 	Was listening to ????
 * 	fixed the memory crash bug in the tokenizer
 * 	Statements: VAC, VARIABLES
 * 	String evaluator
 *
 * 0.10 2011-Feb-6 "Superb Owl"
 *	SuperBowl Sunday
 * 	Variable engine added
 * 	PRINT command first version.
 * 	RENUM fixed.  LIST now does -NUM NUM- support
 * 	basic evaluation kinda working
 *
 * 0.09 2011-Feb-5 "Delenn"
 *	Babylon 5 rewatching
 * 	Core backbone is complete
 * 	argc/argv for token lists implemented
 * 	LIST now supports TI-style line number listing LIST, LIST A, LIST A-B
 * 	Runtime execution basics
 * 	Statements:  RUN, END, GOTO, RENUM, GOSUB, RETURN, TRON, TROFF
 * 	readme.txt added
 *
 * 0.08 2011-Feb-2 "Valen"
 *	Babylon 5 rewatching
 * 	Rewrite started 
 * 	new core (linked lists), tokenizer, stringizer, structures
 * 	Statements: bye, rem, ls, load, save, list info
 *
 * 0.07 2011-Feb-2
 *	Changed "quit" to Atari 8Bit "bye"
 *
 * 0.06 2011-Feb-1
 *	Proper line number insertion/removal
 *	Statements: help, hex
 *	Fixed: rem, : ; tokens
 *	moved the globals into a handle
 *
 * 0.05 2011-Jan-31
 * 	Tokenizer
 *	Statements: load, save, list, new, ls, info, quit
 *
 * 0.00-0.04
 *	The first version, with tokenized bytecode rather than linked list
 *	Scrapped, and mostly rewritten as this version.
 */
