#include <stdio.h>
#include <stdlib.h>
#include "buffer_api.h"
int main(int argc, char* argv[]) {
	if(argc != 2){
		perror("usage: ./solution <testcase number>");
		return -1;
	}
	buffer_init(atoi(argv[1]));
	const struct ring_buffer *rb = buffer_get_base();

	/* ------ YOUR CODE ENDS HERE ------*/
	
	/* print formats */
	// printf("0x%...."); // to print sample addresses
	// printf("number of lost records: %....");
	// printf("unknown size: %....");

	/* ------ YOUR CODE ENDS HERE ------*/
	buffer_exit();
	return 0;
}

