#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>


int main(){
    pid_t pid = fork(); // parent will give >0 and child =0

    if(pid == 0){
        printf("I am child\n");
        printf("%d\n",getpid());
        exit(1);
    }
    else{
        // printf("I am parent \n");
        // printf("parent %d\n",pid);
        int status;
        waitpid(pid, &status, 0);
        printf("I am parent \n");
        printf("parent %d\n",getpid());
        status = WEXITSTATUS(status);
        printf("status = %d\n",status);
        exit(1);
    }

    return 0;
}