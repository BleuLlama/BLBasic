----------------------------------------
                         BleuLlama Basic
                                   vx.xx
                             2011-xxx-xx

                          Scott Lawrence
                                 @yorgle
                  http://umlautllama.com


----------------------------------------
                                Overview

BleuLlama Basic (blbasic) is an implementation of the BASIC language, with a few 
specific goals.

1) Load, Run, and Save programs that are similar to existing BASIC
   programs for traditional classic systems.  In particular:

	- Commodore 64
	- Tandy/Casio/Sharp Pocket Computers
	- Texas Instruments TI 99/4A
	- Atari 400/800

   Minus of course, the platform specific stuff (expecting certain
   values at various PEEK locations, or device-specific behaviors.)

2) Portable, and easy to port to new host systems.

3) Open, unencumbered license.  (MIT License)


----------------------------------------
                                   Modes

Like most implementations of BASIC, there are different modes that
are imposed when using the interface.  The user does not switch
between them like one does on pocket computers, but rather their
operations happen based on the input the user provides for each
line entered.

-- Immediate Mode --

This is encountered when the user types a command at the prompt.
For example:
	>: LIST [RETURN]
	>: PRINT A [RETURN]
	>: A = A + 2 [RETURN]


-- Edit Program Mode --

This is encountered when the user puts a line number before a
command.  For example:

	>: 10 PRINT A [RETURN]
	>: 20 A = A + 2 [RETURN]

The user can also replace lines with new ones:

	>: 10 PRINT A [RETURN]
	>: 10 PRINT B [RETURN]

The user can also eliminate lines from their program by entering a
line number with no commands:

	>: 10 [RETURN]

Unlike many implementations of BASIC, just about all of the
traditionally "immediate" calls can be made in Edit Program mode.
That is, the user can actually specify a line such as:

	>: 10 LIST [RETURN]

Which is completely valid.  Often these things are flagged out as
being invalid, but by allowing such things, it simplifies the engine
quite a bit, and adds flexibility to the user.


----------------------------------------
                             Usage Notes

The line is scanned along its length to look for "tokens".  This
means that as long as you observe what tokens/keywords you are 
entering, you can eliminate whitespace.

	>: 10PRINT"HELLO"

expands to:	10 PRINT "HELLO"

	>: 10PRINTSINA+3

expands to:	10 PRINT SIN A + 3

However, if you mistype an expected token, it will likely be 
assumed to be a variable.  (Also note that variable names are not
limited to just one or two characters.)

	>: 10PRINTSONA+3

expands to:	10 PRINT SONA + 3

If you start a string, it will continue through known keywords until
it hits whitespace or a non-alphanumeric character.

	>:10PRINTOSIN+3

expands to:	10 PRINT OSIN + 3


No limits have been set otherwise on the internal implementation
to make it run on classic systems.  For example, the program is not
stored specifically as a flat bytecode array, but rather as modern
linked lists.

Since all function calls (where applicable) pass a handle to a
container which holds the program and variables, you can run multiple
basic interpreters in the same codespace/thread.

Likewise, since all methods are called explicitly, they can be
wrapped with the appropriate semaphores and run thread-safe.

The interface in this package is a demonstrative interface.  It is
a simple command shell, but it is easily expandable/replaceable
with another interface specific to the system it is being ported
to.  That said, it is a fully functional BASIC shell.


----------------------------------------
                          Language Notes

Lines can have multiple statements on each list.  A colon ":" should be
used to separate statements.  For example:


	10 w$ = "World"
	20 PRINT : PRINT "Hello";
	30 PRINT ", "; 
	40 PRINT w$; : PRINT "!"

displays:

	Hello, World!

Entire routines can also be put on a single line

	100 a = a - 100 : RETURN



On some classic systems, line numbers had to be between 1 and 9999,
or 1 and 32767.  In this implementation, line numbers must be between
1 and 2147483647.


----------------------------------------
                            Command List

Most commands can be run in run mode by just typing them in at the
user prompt, as well as in program mode.  Where applicable, examples
of usage have also been provided here.


-- File and Listing commands --

LOAD "FILENAME.BAS"
	Loads the specified filename from the "programs" folder.
LOAD "FILENAME.BAS",1
	Loads the specified filename from the "programs" folder
	and then start running it.

SAVE "FILENAME.BAS"
	Saves the program in memory to the "programs" folder.

LIST
	Prints to the screen the program in memory.
LIST LINE
	Prints to the screen the specific line specified.
LIST MIN-
	lists all lines starting at and including MIN
LIST -MAX
	Lists all lines up to and including MAX
LIST MIN MAX
LIST MIN-MAX
	Prints to the screen the inclusive range specified.

FILES
CATALOG
	Prints a list of files available in the "programs" folder.

NEW
	Erases the program in memory, and clears variables.

RENUM
	renumber the program from 100, increments of 10
RENUM START
	renumber the program from START, increments of 10
RENUM START STEP
	renumber the program from START, increments of STEP


-- System commands --

BYE
	Quit out of the interpreter back to your command shell.

HELP
	Display a list of commands and keywords known.

INFO
	Prints out version information, and number of "steps" used.

VARIABLES
	Prints out a list of all defined variables and their contents.

VAC ALL
	Clear all variables in memory.
VAC
	Clear only unprotected variables.

PROTECT var
PROTECT var$
	Protects a variable from writing. (make it read-only)

UNPROTECT var
UNPROTECT var$
	Unprotects a variable for writing. (make it read-write)

SWAP var,var
SWAP var$,var$
	Swap two variables in-place


-- Commands --

RUN
	Start automatic program execution

TRON
	Turn TRace ON

TROFF
	Turn TRace OFF


GOTO LINENO
	continues runtime operation from the specified line number

GOSUB LINENO
	calls a subroutine at line LINENO
	Max depth of 256 levels.
	NOTE: Casio-Tandy=8 levels. C64=24 levels, miSoft=128 levels

ON a GOTO 10,20,var,var
ON a 10,20,var,var
ON a GOSUB 10,20,var,var
	Checks the value of 'a', and picks the corresponding value
	in the list, (first item is 1) and GOTOs it or GOSUBs it.
	If "GOTO" or "GOSUB" is omitted, it is implied to be a "GOTO" list.
	Both the switcher and the list can contain explicit numbers or
	variables or labels.  It the variable is <1 or >number of things on 
	the list, execution just continues on the next line.

IF a <c> b THEN c
IF a <c> b THEN GOTO c
IF a <c> b THEN GOSUB c
	Compares a with b.  If the comparison <c> is true, then the
	clause is executed with either a GOTO or GOSUB.  If GOTO is
	omitted, it is implied.  the comparisons that are valid are:
	=	is equal to		<>	is not equal to
	< 	is less than		<=	is less or equal to
	> 	is greater than		>=	is greater or equal to

FOR a = b TO c
FOR a = b TO c STEP d
NEXT a
	Iterative FOR-NEXT loop.  b, c, d can be immediate numbers, or
	variables containing the desired numbers. 'a' will be managed 
	by the foor loop mechanism.
	NOTE: due to current implementations:
		d must be positive
		b must be less than c
		the for loop will ALWAYS exceute through once.

LABEL name
	Creates a label to be used with GOTO or GOSUB.  They
	internally create protected variables, and are regenerated
	at runtime.
	Note, that if you change or remove the labels between runs,
	there might be ghosts of the old labels in the variables
	list.

RETURN
	return from a subroutine

PRINT 
PRINT 42
PRINT "hello"
PRINT "hello " ; yourName$
	Prints a list of things to the screen. All things on the list
	will be printed joined together.  If a semicolon is put at the 
	end of the list, no newline will be printed out.  Subsequent
	PRINT or other output will appear on that line.

INPUT a
INPUT a$
INPUT "Prompt " ; a$
INPUT "Prompt ";a
	Prompts the user for input, which will be stored into the 
	variable specified.  All INPUT statements will print a "? ",
	after the user specified prompt if one is provided.

DATA 1,2,3
DATA "a","b","c",d
	Store a list of data to be read out later.

READ a
READ a$
	Read from the current data item into variable 'a' or 'a$'.
	This will also advance to the next data item.

RESTORE
	Reset the data item list to the first data item in the program.
RESTORE 100
RESTORE x
	Reset the data item list to the next item on the specified line.
	'x' can be a variable containing a line number, or a label.


----------------------------------------
                            Known Issues

1) Order of operations is not implemented yet.  Operations will happen
   in the order they're on the line, rather than:
	Functions (SIN, COS, TAN, etc)
	Powers
	Multiplication and Division
	Addition and Subtraction

2) Arrays are not implemented yet.

3) RENUM command is faulty. (currently disabled)

----------------------------------------
                                  Errors

INCOMPLETE, INCORRECT
THESE NEED TO BE REVISITED.

	100 	LOAD couldn't open file for reading
	101	LOAD parameters are invalid
	102	LOAD filename is invalid
	103	SAVE couldn't open file for saving
	104	SAVE couldn't save with no program in memory
	105	SAVE filename not specified
	106	SAVE filename is invalid

	107	RENUM invalid number of parameters
	108	RENUM numbers specified are invalid
	109	LIST invalid number of parameters
	110	RUN invalid number of parameters

	111	GOTO invalid number of parameters
	112	GOTO line number specified was not found
	113	GOSUB invalid number of parameters
	114	GOSUB max depth of gosubs has been reached
	115	GOSUB target line number was not found 

	116	RETURN invalid number of parameters
	117	RETURN no return value in stack to go to


----------------------------------------
                               Platforms

Tested platforms:

	OS X 10.6.6, GCC 4.2.1 (Mac)
	MinGW/MSYS (Windows)



----------------------------------------
                                 License

(MIT License)

Copyright (C) 2011 by Scott Lawrence

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
