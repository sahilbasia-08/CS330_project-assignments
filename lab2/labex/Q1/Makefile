all:pprime
pprime:main.o pprime.o
	gcc main.o pprime.o -o pprime -lm
pprime.o:pprime.c
	gcc -c pprime.c
clean:
	rm -f pprime pprime.o
