#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>

int main()
{
    int i = 1;

    // fork();
    pid_t pid = fork();
    if (pid == 0)
        i = 20;
    printf("PRINTING %d", i);
    return 0;
}