#!/bin/bash 

if [[ ! -f fact.c ]];
then
     echo "Failed. The file count.c does not exist"	
     exit -1
fi

rm -f fact
gcc fact.c -o fact

if [[ ! -f fact ]];
then
     echo "Failed. Compilation error"	
     exit -1
fi

rm -rf output
mkdir output
op_array=(0 6 24 120 720 5040 40320 362880 3628800)

for tc in 1 2 3 4 5 6 7 8
do
      arg=$(($tc+2))	
      op=`strace -f -o output/st$tc.out ./fact $arg`
      if [[ $op -ne ${op_array[$tc]} ]];
      then
	      echo "Test case $tc failed"
              continue;
      fi
      if [[ $? -eq 0 ]];
      then
              execs=`grep "execve(\"./fact\"" output/st$tc.out | wc -l`
	      if [[ $execs -lt $arg-1 ]];
	      then
	          echo "Test case $tc failed. Not using exec or not enough execs?"
	      else
	          echo "Test case $tc passed"
	      fi
      else
	      echo "Test case $tc failed"
      fi
done
