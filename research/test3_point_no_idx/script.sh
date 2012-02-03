#! /bin/bash
PROG=../../dbms
TIMEFORMAT=%R
# test3 5000
echo Insert data from test3_5000.txt
r=$((time $PROG 10 < test3_5000.txt > /dev/null) 2>&1)
echo insertion: ${r}s
echo lookup keys from 5000
r=$((time $PROG 10 < lookup1_5000.txt > /dev/null) 2>&1)
echo lookup1: ${r}s
r=$((time $PROG 10 < lookup2_5000.txt > /dev/null) 2>&1)
echo lookup2: ${r}s
r=$((time $PROG 10 < lookup3_5000.txt > /dev/null) 2>&1)
echo lookup3: ${r}s
r=$((time $PROG 10 < lookup4_5000.txt > /dev/null) 2>&1)
echo lookup4: ${r}s
r=$((time $PROG 10 < lookup5_5000.txt > /dev/null) 2>&1)
echo lookup5: ${r}s
rm -r data

# test3 10000
echo
echo Insert data from test3_10000.txt
r=$((time $PROG 10 < test3_10000.txt > /dev/null) 2>&1)
echo insertion: ${r}s
echo lookup keys from 10000
r=$((time $PROG 10 < lookup1_10000.txt > /dev/null) 2>&1)
echo lookup1: ${r}s
r=$((time $PROG 10 < lookup2_10000.txt > /dev/null) 2>&1)
echo lookup2: ${r}s
r=$((time $PROG 10 < lookup3_10000.txt > /dev/null) 2>&1)
echo lookup3: ${r}s
r=$((time $PROG 10 < lookup4_10000.txt > /dev/null) 2>&1)
echo lookup4: ${r}s
r=$((time $PROG 10 < lookup5_10000.txt > /dev/null) 2>&1)
echo lookup5: ${r}s
rm -r data
