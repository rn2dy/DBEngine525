#! /bin/bash
PROG=../../dbms
TIMEFORMAT=%R

echo Insert data from test4_10000.txt
r=$((time $PROG 10 < test4_10000.txt > /dev/null) 2>&1)
echo insertion: ${r}s

r=$((time $PROG 10 < lookup_5.txt > /dev/null) 2>&1)
echo lookup \(5%\): ${r}s
r=$((time $PROG 10 < lookup_10.txt > /dev/null) 2>&1)
echo lookup \(10%\): ${r}s
r=$((time $PROG 10 < lookup_20.txt > /dev/null) 2>&1)
echo lookup \(20%\): ${r}s
r=$((time $PROG 10 < lookup_30.txt > /dev/null) 2>&1)
echo lookup \(30%\): ${r}s
r=$((time $PROG 10 < lookup_40.txt > /dev/null) 2>&1)
echo lookup \(40%\): ${r}s
r=$((time $PROG 10 < lookup_50.txt > /dev/null) 2>&1)
echo lookup \(50%\): ${r}s
r=$((time $PROG 10 < lookup_60.txt > /dev/null) 2>&1)
echo lookup \(60%\): ${r}s
r=$((time $PROG 10 < lookup_70.txt > /dev/null) 2>&1)
echo lookup \(70%\): ${r}s
rm -r data
