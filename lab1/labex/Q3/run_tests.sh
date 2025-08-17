#!/bin/bash
TESTCASE=$1
SOL="a.out"

rm -f $SOL *.o

if [ ! -f buffer_impl.c ]
then
	echo ""
	echo "ERROR: buffer_impl.c not found!"
	echo ""
fi

if [ ! -f solution.c ]
then
	echo ""
	echo "ERROR: solution.c not found!"
	echo ""
fi

gcc -c buffer_impl.c -o buffer_impl.o
gcc -c solution.c -o solution.o
gcc buffer_impl.o solution.o -o a.out

if [ ! -x a.out ]
then
	echo ""
	echo "compilation failed"
	echo ""
	exit 5
fi

rm -rf outputs
mkdir outputs

if [ ! -d expected_outputs ]; then
	echo "expected output directory missing!"
	exit 1
fi


# testcase number is $1
run_testcase () {
	TESTFILE="testcases/testcase$1.txt"
	echo "running testcase $1, TESTFILE is $TESTFILE"

	OUTPUT_FILE="outputs/output$t.txt"
	echo "./$SOL $1 > $OUTPUT_FILE"
	./$SOL $1 > "$OUTPUT_FILE"

	diff_output=`diff -Naur "$OUTPUT_FILE"  "expected_outputs/output$t.txt"`

	echo ""
	if [[ $diff_output == "" ]];
	then
		echo Test $t passed
	else
		echo Test $t failed
	fi
	echo ""
}

for t in 1 2 3 4 5; do
	run_testcase $t
done

./a.out $TESTCASE
