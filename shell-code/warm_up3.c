#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char** argv){
    if(argc!=3){
        printf("enter 2 args please along wth binary you are running\n");
        exit(-1);
    }

    int pid = fork();

    if(pid<0){
        exit(1);
    }
    if (pid == 0) {
        // child
        char *args[] = { argv[1], argv[2], NULL };
        execvp(argv[1], args);
        perror("execvp");   // only reached if exec fails
        _exit(1); //_exit immediately ends process without flushing I/O buffers again (avoids duplicate output since buffers may be inherited from parent).
    }
    else{
        // parent
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Command successfully completed\n");
        } else {
            printf("EXITSTATUS: %d\n",
                   WIFEXITED(status) ? WEXITSTATUS(status) : 1);
        }
    }
    return 0;
}