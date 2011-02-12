10 REM *** NUMBERS
20 RESTORE
30 FOR z = 0 TO 10
40 READ a
50 PRINT z ; ": " ; a
60 NEXT z
70 REM *** Strings:
80 RESTORE textStrings
90 READ b$ : 90 READ c$
100 PRINT b$ ; " ... " ; c$ ; "!"
1000 REM
1010 REM ************  DATA 
1020 REM
1030 DATA 2 , 4 , 6 , 8 , 10
1040 DATA 1 , 3 , 5 , 7 , 9
1045 LABEL textStrings
1050 DATA "hello" , "goodbye"
