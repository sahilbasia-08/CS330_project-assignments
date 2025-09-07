    #include<stdio.h>
    #include<stdlib.h>
    #include<sys/wait.h>
    #include<unistd.h>

    int main(){

        static char* argc[] = {"ls", "-l", (char *)NULL};
        pid_t pid = fork();

        if(pid==0){

            printf("Child overwritter and is running with pid as %d\n",getpid());
            execl("/bin/ls", "ls","-l",(char*)NULL);
            // execlp("ls","-la", (char* )NULL);
            // // execvp("ls",argc);
        }
        else{
            int status;
            waitpid(pid, &status, 0);
            printf("\nParent alone now %d and with status of child was %d\n",getpid(),WEXITSTATUS(status));
            exit(1);
        }
        return 0;
    }