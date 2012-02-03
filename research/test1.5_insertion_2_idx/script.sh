#! /bin/bash
PROG=../../dbms
TIMEFORMAT=%R
echo Insertion on indices y6\(uni,nod\) 
r=$((time $PROG 10 < test_1_100.txt > /dev/null) 2>&1)
echo Size:100 Elapsed time: ${r}s
rm -r data

r=$((time $PROG 10 < test_1_1000.txt > /dev/null) 2>&1)
echo Size:1000 Elapsed time: ${r}s
rm -r data

r=$((time $PROG 10 < test_1_2000.txt > /dev/null) 2>&1)
echo Size:2000 Elapsed time: ${r}s
rm -r data

r=$((time $PROG 10 < test_1_6000.txt > /dev/null) 2>&1)
echo Size:6000 Elapsed time: ${r}s
rm -r data

r=$((time $PROG 10 < test_1_8000.txt > /dev/null) 2>&1)
echo Size:8000 Elapsed time: ${r}s
rm -r data

r=$((time $PROG 10 < test_1_10000.txt > /dev/null) 2>&1)
echo Size:10000 Elapsed time: ${r}s
rm -r data
