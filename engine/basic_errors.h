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


#define kErrorNone		0
#define kErrorMalloc		-1
#define kErrorParams		-2	/* not enough params */
#define kErrorParamType		-3	/* a parameter was of incorrect type */
#define kErrorFileNotFound	-4	/* file not found */
#define kErrorFileError		-5	/* other file error */
#define kErrorData		-6	/* data stream error */
#define kErrorProgramEmpty	-7	/* program is empty */
#define kErrorNoLine		-8	/* line number doesn't exist */
#define kErrorNonsense		-9	/* nonsense in BASIC */
#define kErrorGosub		-10	/* GOSUB max depth reached */
#define kErrorReturn		-11	/* RETURN when there's no place to return to*/
#define kErrorDirectory		-12	/* unable to read directory */
#define kErrorRunning		-13	/* not a runnable command */
#define kErrorNoOn		-14	/* ON target list exhausted */
#define kErrorNext		-15	/* NEXT error */
#define kErrorDataRead		-16	/* out of DATA error */
#define kErrorDBZero		-17	/* divide by zero */

void errorReportAdditional( int code, int codeMinor, char * additional );
void errorReport( int code, int codeMinor );
