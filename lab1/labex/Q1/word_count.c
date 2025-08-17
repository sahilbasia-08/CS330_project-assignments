#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>


/*
 *
 * PROBLEM DESCRIPTION
 *
 * Write a program to count the number of characters, words and lines in a
 * file.
 * This is a replica of wc(1). Check the wc man page, and play with it for a
 * bit.
 *
 *   $ man wc.1                                     # this is the manual
 *
 *   $ wc word_count.c                              # word count for this file
 *   $ wc -w /usr/share/doc/python3/copyright       # random file
 *   $ wc /usr/share/doc/python3/copyright          # random file
 *
 * wc(1) takes OPTIONAL flags (-l, -w, -c, or some combination).
 *
 * Your version takes one flag (NOT optional), and it is always the first
 * argument to the binary.
 * The second argument is the target file.
 * 
 * Throw an error if the program is not invoked correctly, and print a usage
 * message.
 *
 */

/*
 * Brief intro to man pages:
 *   $ man man      # Read the DESCRIPTION section.
 *
 *
 * This question uses these C library functions:
 *   $ man strcmp.3
 *   $ man getline.3
 *   $ man fopen.3
 *   $ man fclose.3
 *   $ man fprintf.3
 *
 * PS for library functions, you should usually read these sections: SYNOPSIS,
 * DESCRIPTION, RETURN VALUE. Sometimes EXAMPLES is helpful.
 */

/*
 * Command Line Arguments:
 *
 * No reasonable program does the same computation on each invocation. There are
 * a handful of ways to influence program behaviour (for a compiled binary) at
 * run time:
 *   - read some_file_name.txt and load matrices from it
 *   - environment variables (in your shell, run $ env to check the environment)
 *   - scanf?
 *   - command line arguments (this question)
 *
 * When a program is invoked by the OS, it is given some "arguments"
 * For example:
 *
 *   $ program_name arg1 arg2 arg3
 *   $ ./a.out 100 output.txt
 *   $ gcc -o square square.c
 *   $ mkdir --help
 *   $ man mkdir
 *
 * The first word/path is the program being invoked.
 * The arguments include the first word _and_ the other words.
 *
 * All languages have some way to access them. Ex sys.argv in python.
 *
 *   pranjal@lab:~$ python3 -i a b c d
 *   >>> import sys
 *   >>> sys.argv
 *   ['a', 'b', 'c', 'd']
 *
 */

/*
 * Arguments in C/C++:
 *
 * In serious programs, int main() takes two arguments, not 0.
 * @argc is the number of CLI arguments
 * @argv is a null-terminated array of pointers to null-terminated strings.
 * (So, argc isn't really needed.)
 *
 * dump_cli_arguments iterates over this list.
 * Compile and run this program on the shell with a few arguments.
 * Steps:
 *
 *   $ gcc word_count.c   # generates a.out
 *   $ ./a.out
 *   $ ./a.out arg1
 *   $ ./a.out 1 2 3 4
 *
 * Slightly complicated examples. Don't scratch your head over these.
 *   $ ./a.out 1 2 3 4 # bash comments aren't passed to a.out!
 *   $ ./a.out "1 2 3 4"
 *   $ ./a.out "\"1 2 3 4\""
 *   $ .a.out "PWD is $PWD and USER is $USER" 1 2 3 4
 *   $ .a.out 'PWD is $PWD and USER is $USER' 1 2 3 4
 *   $ ./a.out *
 */

void dump_cli_arguments(int argc, char *argv[]) {
	fprintf(stderr, "Printing CLI arguments\n");
	fprintf(stderr, "argc is %d || argv is %p\n\n", argc, argv);

	for (int i = 0; i < (argc + 1); i++) {
		fprintf(stderr, "[arg %d @ %p] (char *)%p: %s\n",
			i,
			&argv[i], // the address of the pointer
				  // increases by 8 in each step

			argv[i],  // the value of the pointer (%p)
			argv[i]   // the string at the pointer's address (%s)
			);
	}
	fprintf(stderr, "end of args\n");
	return;
}

/*
 * Aside: managing cli args: languages typically offer dedicated functions to
 * handle arguments.
 *
 * c: $ man getopt.3
 * bash/shell: $ man getopt.1
 * python has an argparse library (import argparse; help(argparse))
 */

// -----------------------
// Your solution begins here
int main(int argc, char *argv[]) {
	dump_cli_arguments(argc, argv);
     
    if(argc != 3){
        exit(-1);
    }
    else{
        int l = 0;
        while(argv[1][l]!='\0'){
            printf("%c\n",argv[1][l]);
            l++;
        }
        char parameter = argv[1][1];
        char *path = argv[2];
        if( parameter == 'l' ){
            // we will count the number of lines then
            char *line = NULL;
            size_t len = 0;
            unsigned long long lines = 0;
            FILE *fptr;
            fptr = fopen(path, "r");
            if(!fptr){
                printf("SUCK !!!\n");
                exit(-1);
            }
            else{
                while(getline(&line, &len, fptr) != -1){
                    lines++;
                }

                printf("Total lines in file are = %llu\n", lines);
            }
        }
        else if( parameter == 'w' ){
            unsigned long long words = 0;
            char c = 'a';
            unsigned long long len = 0;
            unsigned ans = 0;
            FILE *fptr;
            fptr = fopen(path, "r");
            if(!fptr){
                printf("EXIT IN FILE OPENING\n");
                exit(-1);
            }
            else{
                while((c = fgetc(fptr)) != EOF){
                    ans++;
                    if( isspace(c) ) {
                        len = 0;
                    }
                    else if(!len){
                        len = 1;
                        words++;
                    }
                }

                printf("Total words are = %llu and chars are = %u\n",words, ans);
            }
        }
        else if( parameter == 'c' ){
            unsigned long long c_ans = 0;
            FILE *fptr = NULL;
                fptr = fopen(path, "r");

                char c = 'a';
                while((c=fgetc(fptr)) != EOF){
                    c_ans++;
                }
                printf("Total chars in files are = %llu\n", c_ans);
        }
    }
#if 0	// change or remove this when you are done with argc/argv.
	// We've left this so that word_count.c compiles and runs out of the box.

	if (argc != 3) {
		fprintf(stderr, "Usage: %s -x filename\n", argv[0]);
		fprintf(stderr, "where -x is one of -l (lines), -c (characters), or -w (words)\n");
		fprintf(stderr, "Alternate usage notation: %s <-c|-w|-l> <filename>\n", argv[0]);
		return 1;
	}

	char *option = argv[1];
	char *filename = argv[2];

	if (strcmp(option, "-l") != 0 && strcmp(option, "-c") != 0 && strcmp(option, "-w") != 0) {
		fprintf(stderr, "Error: Unknown option '%s'. Use -l, -c, or -w.\n", option);
		return 1;
	}

	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("Error opening file");
		return 1;
	}

	int count = 0;
	char* line = malloc(4096 * sizeof(char));
	size_t line_size = 4096;

	if (strcmp(option, "-c") == 0) {
		size_t retval;
		retval = getline();
		if (retval == -1)
			printf("%d\n", count);
	}
	else if (strcmp(option, "-l") == 0) {
		while (getline() != -1) {
			count++;
		}
	}
	else if (strcmp(option, "-w") == 0) {
		// Note: there may be multiple whitespaces between two words
		// You can use isspace(char)
		size_t chars_read;
	}

	free(line);
	fclose(fp);

	printf("%d\n", count);
#endif
	return 0;
}

