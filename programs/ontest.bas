
10 a = 0;
20 print "A = "; A
30 gosub SplitIt
40 A=A+1
50 goto 20


60 REM ******************
70 LABEL SplitIt
80 ON A GOSUB 100,200,300,FourHundred,500
90 RETURN


100 REM ******************
110 PRINT "Line 100"
120 RETURN

200 REM ******************
210 PRINT "Line 200"
220 RETURN

300 REM ******************
310 PRINT "Line 300"
320 RETURN

400 REM ******************
410 LABEL FourHundred
420 PRINT "Line 400"
430 RETURN

500 REM ******************
510 PRINT "End."
520 END
