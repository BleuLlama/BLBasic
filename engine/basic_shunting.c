/* basic_shunting
 *
 *   BleuLlama BASIC
 *   Copyright (c) 2011 Scott Lawrence
 */

/* a modification of the "Shunting Yard Algorithm" by Edsger Dijkstra, 
 * and implemented on the wikipedia page here:
 * http://en.wikipedia.org/wiki/Shunting_yard_algorithm
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

#include <string.h>
#include <stdio.h>

#ifndef bool
#define bool int
#endif
#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
 
/* operators
 * precedence   operators       associativity
 * 1            !               right to left
 * 2            * / %           left to right
 * 3            + -             left to right
 * 4            =                right to left
*/
int op_preced(const char c)
{
    switch(c)    {
        case '!':
            return 4;
        case '*':  case '/': case '%':
            return 3;
        case '+': case '-':
            return 2;
        case '=':
            return 1;
    }
    return 0;
}
 
bool op_left_assoc(const char c)
{
    switch(c)    {
        /* left to right */
        case '*': case '/': case '%': case '+': case '-':
            return true;
        /* right to left */
        case '=': case '!':
            return false;
    }
    return false;
}
 
unsigned int op_arg_count(const char c)
{
    switch(c)  {
        case '*': case '/': case '%': case '+': case '-': case '=':
            return 2;
        case '!':
            return 1;
	default:
 	    return c - 'A';
    }
    return 0;
}
 
#define is_operator(c)  (c == '+' || c == '-' || c == '/' || c == '*' || c == '!' || c == '%' || c == '=')
#define is_function(c)  (c >= 'A' && c <= 'Z')
#define is_ident(c)     ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z'))
 
bool shunting_yard(const char *input, char *output)
{
    const char *strpos = input, *strend = input + strlen(input);
    char c, *outpos = output;
 
    char stack[32];       /* operator stack */
    unsigned int sl = 0;  /* stack length */
    char     sc;          /* used for record stack element */
 
    while(strpos < strend)   {
        /* read one token from the input stream */
        c = *strpos;
        if(c != ' ')    {
            /* If the token is a number (identifier), then add it to the output queue. */
            if(is_ident(c))  {
                *outpos = c; ++outpos;
            }
            /* If the token is a function token, then push it onto the stack. */
            else if(is_function(c))   {
                stack[sl] = c;
                ++sl;
            }
            /* If the token is a function argument separator (e.g., a comma): */
            else if(c == ',')   {
                bool pe = false;
                while(sl > 0)   {
                    sc = stack[sl - 1];
                    if(sc == '(')  {
                        pe = true;
                        break;
                    }
                    else  {
                        /* Until the token at the top of the stack is a left parenthesis, */
                        /* pop operators off the stack onto the output queue. */
                        *outpos = sc; 
                        ++outpos;
                        sl--;
                    }
                }
                /* If no left parentheses are encountered, either the separator was misplaced */
                /* or parentheses were mismatched. */
                if(!pe)   {
                    printf("Error: separator or parentheses mismatched\n");
                    return false;
                }
            }
            /* If the token is an operator, op1, then: */
            else if(is_operator(c))  {
                while(sl > 0)    {
                    sc = stack[sl - 1];
                    /* While there is an operator token, o2, at the top of the stack */
                    /* op1 is left-associative and its precedence is less than or equal to that of op2, */
                    /* or op1 is right-associative and its precedence is less than that of op2, */
                    if(is_operator(sc) &&
                        ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) ||
                           (!op_left_assoc(c) && (op_preced(c) < op_preced(sc)))))   {
                        /* Pop o2 off the stack, onto the output queue; */
                        *outpos = sc; 
                        ++outpos;
                        sl--;
                    }
                    else   {
                        break;
                    }
                }
                /* push op1 onto the stack. */
                stack[sl] = c;
                ++sl;
            }
            /* If the token is a left parenthesis, then push it onto the stack. */
            else if(c == '(')   {
                stack[sl] = c;
                ++sl;
            }
            /* If the token is a right parenthesis: */
            else if(c == ')')    {
                bool pe = false;
                /* Until the token at the top of the stack is a left parenthesis, */
                /* pop operators off the stack onto the output queue */
                while(sl > 0)     {
                    sc = stack[sl - 1];
                    if(sc == '(')    {
                        pe = true;
                        break;
                    }
                    else  {
                        *outpos = sc; 
                        ++outpos;
                        sl--;
                    }
                }
                /* If the stack runs out without finding a left parenthesis, then there are mismatched parentheses. */
                if(!pe)  {
                    printf("Error: parentheses mismatched\n");
                    return false;
                }
                /* Pop the left parenthesis from the stack, but not onto the output queue. */
                sl--;
                /* If the token at the top of the stack is a function token, pop it onto the output queue. */
                if(sl > 0)   {
                    sc = stack[sl - 1];
                    if(is_function(sc))   {
                        *outpos = sc; 
                        ++outpos;
                        sl--;
                    }
                }
            }
            else  {
                printf("Unknown token %c\n", c);
                return false; /* Unknown token */
            }
        }
        ++strpos;
    }
    /* When there are no more tokens to read: */
    /* While there are still operator tokens in the stack: */
    while(sl > 0)  {
        sc = stack[sl - 1];
        if(sc == '(' || sc == ')')   {
            printf("Error: parentheses mismatched\n");
            return false;
        }
        *outpos = sc; 
        ++outpos;
        --sl;
    }
    *outpos = 0; /* Null terminator */
    return true;
}
 
bool execution_order(const char *input) {
    const char *strpos = input, *strend = input + strlen(input);
    char c, res[4];
    unsigned int sl = 0, sc, stack[32], rn = 0;

    printf("order: (arguments in reverse order)\n");
	/* While there are input tokens left */
    while(strpos < strend)  {
		/* Read the next token from input. */
        c = *strpos;
		/* If the token is a value or identifier */
        if(is_ident(c))    {
			/* Push it onto the stack. */
            stack[sl] = c;
            ++sl;
        }
		/* Otherwise, the token is an operator  (operator here includes both operators, and functions). */
        else if(is_operator(c) || is_function(c))    {
			unsigned int nargs = 0;
			sprintf(res, "_%02d", rn);
			printf("%s = ", res);
			++rn;
			/* It is known a priori that the operator takes n arguments. */
			nargs = op_arg_count(c);
			/* If there are fewer than n values on the stack */
			if(sl < nargs) {
				/* (Error) The user has not input sufficient values in the expression. */
				return false;
			}
			/* Else, Pop the top n values from the stack. */
			/* Evaluate the operator, with the values as arguments. */
			if(is_function(c)) {
				printf("%c(", c);
				while(nargs > 0)	{
					sc = stack[sl - 1];
					sl--;
					if(nargs > 1)	{
						printf("%s, ", (char*)&sc);
					}
					else {
						printf("%s)\n", (char*)&sc);
					}
					--nargs;
				}
			}
			else	{
				if(nargs == 1) {
					sc = stack[sl - 1];
					sl--;
					printf("%c %s;\n", c, (char*)&sc);
				}
				else	{
					sc = stack[sl - 1];
					sl--;
					printf("%s %c ", (char*)&sc, c);
					sc = stack[sl - 1];
					sl--;
					printf("%s;\n",(char*)&sc);
				}
			}
			/* Push the returned results, if any, back onto the stack. */
            stack[sl] = *(unsigned int*)res;
            ++sl;
        }
        ++strpos;
    }
	/* If there is only one value in the stack */
	/* That value is the result of the calculation. */
	if(sl == 1) {
		sc = stack[sl - 1];
		sl--;
		printf("%s is a result\n", (char*)&sc);
		return true;
	}
	/* If there are more values in the stack */
	/* (Error) The user input has too many values. */
	return false;
}
 
#ifdef STANDALONE
int main() {
    /* functions: A() B(a) C(a, b), D(a, b, c) ... */
    /* identifiers: 0 1 2 3 ... and a b c d e ... */
    /* operators: = - + / * % ! */
    const char *input = "a = D(f - b * c + d, !e, g)";
    char output[128];
    printf("input: %s\n", input);
    if(shunting_yard(input, output))    {
        printf("output: %s\n", output);
        if(!execution_order(output))
            printf("\nInvalid input\n");
    }
    return 0;
}
#endif

