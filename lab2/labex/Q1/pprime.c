#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>

extern int isPrime(long x); // Returns zero if composite.
                            // Returns one if prime. You may use this if you want.
int find_primes(long M)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("forking error");
        exit(1);
    }
    int total = 0;
    if (pid == 0)
    {
        // child process makign another fork
        // so total of 2 child of main parent

        pid_t pid2 = fork();

        if (!pid2)
        {
            int count = 0;
            for (int i = 2; i < M / 2; ++i)
            {
                count += isPrime(i);
            }
            count %= 256;
            exit(count);
        }
        else
        {

            int count = 0;
            for (int i = M / 2 + 1; i <= M; ++i)
            {
                count += isPrime(i);
            }
            int value_status = 0;
            waitpid(pid2, &value_status, 0);
            count += WEXITSTATUS(value_status);
            // exit(count % 256);
            exit(count % 256);
        }

    } //
    int value_status_2;
    waitpid(pid, &value_status_2, 0);
    return WEXITSTATUS(value_status_2);
}