#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{

    if (argc < 2)
    {
        printf("Usage atleast: %s <value of fact to find>| example %s 5\nthis is the minimum command line argument you must pass", argv[0], argv[0]);
        exit(1);
    }

    if (argc == 4)
    {
        if (atoi(argv[2]) == atoi(argv[1]))
        {
            // printf("\n%d", atoi(argv[3]));
            printf("%d\n", atoi(argv[3]));
            return atoi(argv[3]);
        }
        int start = atoi(argv[2]);
        int ans = atoi(argv[3]);
        int final = atoi(argv[1]);
        start += 1;
        ans *= start;
        char arg1[20], arg2[20], arg3[20];
        snprintf(arg1, 20, "%d", start);
        snprintf(arg2, 20, "%d", ans);
        snprintf(arg3, 20, "%d", final);
        execl("./fact", argv[0], arg3, arg1, arg2, (char *)NULL);
    }
    else
    {
        int start = 1;
        int ans = 1;
        char arg1[20], arg2[20], arg3[20];
        snprintf(arg1, 20, "%d", start);
        snprintf(arg2, 20, "%d", ans);
        snprintf(arg3, 20, "%d", atoi(argv[1]));
        execl("./fact", argv[0], arg3, arg1, arg2, (char *)NULL);
    }
    /*Your code goes here */
    // as far I know exec replaced the whole binary of current process from ram
    // so it kinds of overwrites the current process

    return 0;
}
