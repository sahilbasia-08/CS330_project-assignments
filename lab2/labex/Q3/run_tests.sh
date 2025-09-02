#!/bin/bash

if [[ ! -f init.c ]];
then
     echo "Failed. The file init.c does not exist"	
     exit -1
fi

rm -f init

NA=`grep "system" init.c`
if [[ "$NA" != "" ]];
then
     echo "Your solution uses some constructs that is not allowed culprit --> $NA"	
     exit -1

fi

NA=`grep "grep" init.c`
if [[ "$NA" != "" ]];
then
     echo "Your solution uses some constructs that is not allowed culprit --> $NA"	
     exit -1

fi

NA=`grep "fork" init.c`
if [[ "$NA" != "" ]];
then
     echo "Your solution uses some constructs that is not allowed culprit --> $NA"	
     exit -1

fi

NA=`grep "exec" init.c`
if [[ "$NA" != "" ]];
then
     echo "Your solution uses some constructs that is not allowed culprit --> $NA"	
     exit -1

fi

gcc init.c -o init

if [[ ! -f init ]];
then
     echo "Failed. Compilation error"	
     exit -1
fi


# Test case function
test_case() {

    STRING="$1"
    FILE="$2"
    EXPECTED="$3"
    NUM="$4"

    OUTPUT=$(./init "$STRING" "$FILE")
    if [ "$OUTPUT" == "$EXPECTED" ];
    then
      echo "Test case $NUM passed"
    else
      echo "Test case $NUM failed"
    fi

}

# Simple Test Cases
test_case "hello" "testcases/testcase1.txt" "FOUND" "1"
test_case "goodbye" "testcases/testcase2.txt" "NOT FOUND" "2"
test_case "endmarker" "testcases/testcase3.txt" "FOUND" "3"
test_case "unicron" "testcases/testcase4.txt" "NOT FOUND" "4"
test_case "123" "testcases/testcase4.txt" "FOUND" "5"
test_case "1" "testcases/testcase2.txt" "NOT FOUND" "6"
test_case "a6" "testcases/testcase3.txt" "FOUND" "7"
test_case "check" "testcases/testcase8.txt" "Error" "8"
