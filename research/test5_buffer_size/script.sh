#! /bin/bash
PROG=../../dbms
TIMEFORMAT=%R
echo Create and Insert initial data
r=$((time $PROG 10 < data.txt > /dev/null) 2>&1)
echo Insertion time: ${r}s

echo 1 batch
r=$((time $PROG 10 < queries_1.txt > /dev/null) 2>&1)
echo time\(10 buffers\): ${r}s
r=$((time $PROG 1000 < queries_1.txt > /dev/null) 2>&1)
echo time\(1000 buffers\): ${r}s
r=$((time $PROG 3000 < queries_1.txt > /dev/null) 2>&1)
echo time\(3000 buffers\): ${r}s
r=$((time $PROG 5000 < queries_1.txt > /dev/null) 2>&1)
echo time\(5000 buffers\): ${r}s
r=$((time $PROG 6000 < queries_1.txt > /dev/null) 2>&1)
echo time\(6000 buffers\): ${r}s
r=$((time $PROG 7000 < queries_1.txt > /dev/null) 2>&1)
echo time\(7000 buffers\): ${r}s
r=$((time $PROG 8000 < queries_1.txt > /dev/null) 2>&1)
echo time\(8000 buffers\): ${r}s
r=$((time $PROG 10000 < queries_1.txt > /dev/null) 2>&1)
echo time\(10000 buffers\): ${r}s

echo 3 batch
r=$((time $PROG 10 < queries_3.txt > /dev/null) 2>&1)
echo time\(10 buffers\): ${r}s
r=$((time $PROG 1000 < queries_3.txt > /dev/null) 2>&1)
echo time\(1000 buffers\): ${r}s
r=$((time $PROG 3000 < queries_3.txt > /dev/null) 2>&1)
echo time\(3000 buffers\): ${r}s
r=$((time $PROG 5000 < queries_3.txt > /dev/null) 2>&1)
echo time\(5000 buffers\): ${r}s
r=$((time $PROG 6000 < queries_3.txt > /dev/null) 2>&1)
echo time\(6000 buffers\): ${r}s
r=$((time $PROG 7000 < queries_3.txt > /dev/null) 2>&1)
echo time\(7000 buffers\): ${r}s
r=$((time $PROG 8000 < queries_3.txt > /dev/null) 2>&1)
echo time\(8000 buffers\): ${r}s
r=$((time $PROG 10000 < queries_3.txt > /dev/null) 2>&1)
echo time\(10000 buffers\): ${r}s

echo 5 batch
r=$((time $PROG 10 < queries_5.txt > /dev/null) 2>&1)
echo time\(10 buffers\): ${r}s
r=$((time $PROG 1000 < queries_5.txt > /dev/null) 2>&1)
echo time\(1000 buffers\): ${r}s
r=$((time $PROG 3000 < queries_5.txt > /dev/null) 2>&1)
echo time\(3000 buffers\): ${r}s
r=$((time $PROG 5000 < queries_5.txt > /dev/null) 2>&1)
echo time\(5000 buffers\): ${r}s
r=$((time $PROG 6000 < queries_5.txt > /dev/null) 2>&1)
echo time\(6000 buffers\): ${r}s
r=$((time $PROG 7000 < queries_5.txt > /dev/null) 2>&1)
echo time\(7000 buffers\): ${r}s
r=$((time $PROG 8000 < queries_5.txt > /dev/null) 2>&1)
echo time\(8000 buffers\): ${r}s
r=$((time $PROG 10000 < queries_5.txt > /dev/null) 2>&1)
echo time\(10000 buffers\): ${r}s
rm -r data

