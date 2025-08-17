#!/bin/bash

SRC="word_count.c"
SOL="./word_count"
TEST_DIR="./testcases"

rm -f init $SOL a.out

if [[ ! -f "$SRC" ]];
then
     echo "Failed. The file init.c does not exist"	
     exit -1
fi

# check input for sus keywords

# $1 is the keyword to search for in $SRC
check_lib_func () {
    NA=`grep "system" "$SRC"`
    if [[ "$NA" != "" ]];
    then
        echo "Your solution uses some constructs that is not allowed culprit --> $NA"	
        exit -1

    fi
}

check_lib_func system
check_lib_func fork
check_lib_func "exec"

gcc "$SRC" -o ${SOL}

if [ ! -x "$SOL" ]
then
    echo "compilation failed"
    echo "good night!"
    echo
    exit -1
fi



# first arg is testcase num
# second arg is TESTFILE
do_testcase () {
    t=$1
    TESTFILE=$2
    all_ok=true

    # Test -l, -w, -c: should exit 0 and match wc output
    for cmd in -l -w -c; do
        OUTPUT=$($SOL $cmd "$TESTFILE")
        STATUS=$?
        EXPECTED=$(wc $cmd < "$TESTFILE" | awk '{print $1}')

        if [[ $STATUS -ne 0 || "$OUTPUT" != "$EXPECTED" ]]; then
            echo "testcase $t $cmd failed (expected $EXPECTED, got $OUTPUT, status $STATUS)"
            all_ok=false
        fi
    done

    # Test invalid combined flag -lw: should exit non-zero
    $SOL -lw "$TESTFILE" >/dev/null 2>&1
    STATUS=$?
    if [[ $STATUS -eq 0 ]]; then
        echo "testcase $t -lw failed (expected error, didn't get one)"
        all_ok=false
    fi

    # If all checks passed, print single success line
    if $all_ok; then
        echo "testcase $t passed"
    fi
}
    
# init testcase 3
man perf_event_open > $TEST_DIR/testcase3.txt

# first set of tests
for t in 1 2 3; do
    TESTFILE="$TEST_DIR/testcase${t}.txt"
    do_testcase $t $TESTFILE

done

# second set
for modulo in 3030 2020 1010; do
    tokens=$(($$%$modulo))

    head -n $tokens /proc/kallsyms > "$TEST_DIR/testcase4_$modulo.txt"
    echo > "$TEST_DIR/testcase4_$modulo.txt"
    head -c $tokens /proc/kallsyms > "$TEST_DIR/testcase5_$modulo.txt"
    echo >  "$TEST_DIR/testcase5_$modulo.txt"

    seconds=$(date +"%s")
    tokens=$(($seconds%$modulo))

    head -n $tokens /proc/kallsyms > "$TEST_DIR/testcase6_$modulo.txt"
    echo > "$TEST_DIR/testcase6_$modulo.txt"
    head -c $tokens /proc/kallsyms > "$TEST_DIR/testcase7_$modulo.txt"
    echo > "$TEST_DIR/testcase7_$modulo.txt"

    do_testcase "4_$modulo" "$TEST_DIR/testcase4_$modulo.txt"
    do_testcase "5_$modulo" "$TEST_DIR/testcase5_$modulo.txt"
    do_testcase "6_$modulo" "$TEST_DIR/testcase6_$modulo.txt"
    do_testcase "7_$modulo" "$TEST_DIR/testcase7_$modulo.txt"
done
