#!/bin/bash 

if [[ ! -f pprime.c ]];
then
     echo "Failed. The file count.c does not exist"	
     exit -1
fi

make clean > /dev/null 2>&1
make > /dev/null 2>&1

if [[ ! -f pprime ]];
then
     echo "Failed. Compilation error"	
     exit -1
fi

rm -rf output
mkdir output
echo -n > output/output.txt

M=500
for tc in 1 2 3 4 5
do
      strace -f -o output/st$tc.out ./pprime $M $tc >> output/output.txt 	
      if [[ $? -eq 0 ]];
      then
              forks=`grep "clone(child_stack" output/st$tc.out | wc -l`
	      if [[ $forks -ne 2 ]];
	      then
	          echo "Test case $tc failed. Not using fork?"
	      else
	         echo "Test case $tc passed"
	      fi
      else
	      echo "Test case $tc failed"
      fi
      M=`echo "$M*10" | bc`
done

