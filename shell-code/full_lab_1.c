#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG 64
#define MAX_CMDS 64

char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for (i = 0; i < (int)strlen(line); i++)
    {
        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t')
        {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0)
            {
                if (tokenNo < MAX_NUM_TOKENS - 1)
                {
                    tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
                    strcpy(tokens[tokenNo++], token);
                }
                tokenIndex = 0;
            }
        }
        else
        {
            if (tokenIndex < MAX_TOKEN_SIZE - 1)
            {
                token[tokenIndex++] = readChar;
            }
        }
    }

    free(token);
    tokens[tokenNo] = NULL;
    return tokens;
}

static void free_tokens(char **tokens)
{
    if (!tokens)
        return;
    for (int i = 0; tokens[i] != NULL; i++)
        free(tokens[i]);
    free(tokens);
}

static pid_t bg_pids[MAX_BG];
static int bg_count = 0;

static void add_bg(pid_t pid)
{
    if (bg_count < MAX_BG)
        bg_pids[bg_count++] = pid;
}

static volatile sig_atomic_t got_sigint = 0;
static pid_t fg_list[MAX_CMDS];
static volatile sig_atomic_t fg_count = 0;

static void sigint_handler(int signo)
{
    (void)signo;
    got_sigint = 1;
    for (int i = 0; i < fg_count; i++)
        if (fg_list[i] > 0)
            kill(fg_list[i], SIGINT);
}

static void reap_background_terminated(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("Shell: Background process finished\n");
        fflush(stdout);
    }
}

static int launch_simple(char *argv[], int background, pid_t *out_pid, pid_t forced_pgid)
{
    (void)forced_pgid;
    if (!argv || !argv[0])
        return 0;

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_DFL;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);

        execvp(argv[0], argv);
        perror("execvp");
        _exit(1);
    }
    else
    {
        if (out_pid)
            *out_pid = pid;

        if (background)
        {
            add_bg(pid);
            return 0;
        }
        else
        {
            int status;
            fg_list[0] = pid;
            fg_count = 1;
            if (waitpid(pid, &status, 0) < 0)
            {
                perror("waitpid");
                fg_count = 0;
                return 1;
            }
            fg_count = 0;

            if (WIFEXITED(status))
            {
                int code = WEXITSTATUS(status);
                if (code != 0)
                {
                    printf("EXITSTATUS: %d\n", code);
                    fflush(stdout);
                }
            }
            else if (WIFSIGNALED(status))
            {
                printf("EXITSTATUS: 1\n");
                fflush(stdout);
            }
            return 0;
        }
    }
}

static int build_argv(char **tokens, int start, int end, char *out_argv[], int cap)
{
    int k = 0;
    for (int i = start; i < end && k < cap - 1; i++)
    {
        out_argv[k++] = tokens[i];
    }
    out_argv[k] = NULL;
    return k;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    char line[MAX_INPUT_SIZE];
    char **tokens;

    while (1)
    {
        reap_background_terminated();

        memset(line, 0, sizeof(line));
        got_sigint = 0;

        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd)))
            strcpy(cwd, "?");
        printf("%s $ ", cwd);
        fflush(stdout);

        int rc = scanf("%1023[^\n]", line);
        if (rc == EOF)
        {
            printf("\n");
            break;
        }
        if (rc == 0)
        {
            getchar();
            continue;
        }
        getchar();
        {
            size_t L = strlen(line);
            if (L < MAX_INPUT_SIZE - 1)
            {
                line[L] = '\n';
                line[L + 1] = '\0';
            }
        }
        tokens = tokenize(line);

        if (tokens[0] == NULL)
        {
            free_tokens(tokens);
            continue;
        }

        if (strcmp(tokens[0], "exit") == 0)
        {
            for (int i = 0; i < bg_count; i++)
            {
                pid_t pg = bg_pids[i];
                if (pg > 0)
                    kill(pg, SIGTERM);
            }
            usleep(150000);
            for (int i = 0; i < bg_count; i++)
            {
                pid_t pg = bg_pids[i];
                if (pg > 0)
                    kill(pg, SIGKILL);
            }
            int st;
            while (waitpid(-1, &st, WNOHANG) > 0)
            {
            }
            free_tokens(tokens);
            break;
        }

        if (strcmp(tokens[0], "cd") == 0)
        {
            if (tokens[1] == NULL || tokens[2] != NULL)
            {
                fprintf(stderr, "cd: usage: cd <directory>\n");
            }
            else if (chdir(tokens[1]) != 0)
            {
                perror("cd");
            }
            free_tokens(tokens);
            continue;
        }

        int is_parallel = 0, is_serial = 0;
        for (int i = 0; tokens[i] != NULL; i++)
        {
            if (strcmp(tokens[i], "&&&") == 0)
            {
                is_parallel = 1;
                break;
            }
            if (strcmp(tokens[i], "&&") == 0)
            {
                is_serial = 1;
            }
        }

        if (!is_parallel && !is_serial)
        {
            int bg = 0;
            int last = 0;
            while (tokens[last] != NULL)
                last++;
            if (last > 0 && strcmp(tokens[last - 1], "&") == 0)
            {
                bg = 1;
                tokens[last - 1] = NULL;
            }

            (void)launch_simple(tokens, bg, NULL, 0);
            free_tokens(tokens);
            continue;
        }

        int split_pos[MAX_CMDS + 1];
        int scount = 0;
        split_pos[scount++] = 0;
        for (int i = 0; tokens[i] != NULL; i++)
        {
            if ((!is_parallel && strcmp(tokens[i], "&&") == 0) ||
                (is_parallel && strcmp(tokens[i], "&&&") == 0))
            {
                tokens[i] = NULL;
                split_pos[scount++] = i + 1;
                if (scount >= MAX_CMDS)
                    break;
            }
        }
        split_pos[scount] = -1;

        if (is_serial)
        {
            for (int s = 0; s < scount; s++)
            {
                char *argvv[MAX_NUM_TOKENS];
                int end = split_pos[s + 1];
                if (end < 0)
                {
                    int i = split_pos[s];
                    while (tokens[i] != NULL)
                        i++;
                    end = i;
                }
                build_argv(tokens, split_pos[s], end, argvv, MAX_NUM_TOKENS);
                if (!argvv[0])
                    continue;

                if (strcmp(argvv[0], "cd") == 0)
                {
                    if (argvv[1] == NULL || argvv[2] != NULL)
                        fprintf(stderr, "cd: usage: cd <directory>\n");
                    else if (chdir(argvv[1]) != 0)
                        perror("cd");
                    if (got_sigint)
                    {
                        got_sigint = 0;
                        break;
                    }
                    continue;
                }

                (void)launch_simple(argvv, 0, NULL, 0);

                if (got_sigint)
                {
                    got_sigint = 0;
                    break;
                }
            }
            free_tokens(tokens);
            continue;
        }

        if (is_parallel)
        {
            pid_t pids[MAX_CMDS];
            int pcnt = 0;

            for (int s = 0; s < scount; s++)
            {
                char *argvv[MAX_NUM_TOKENS];
                int end = split_pos[s + 1];
                if (end < 0)
                {
                    int i = split_pos[s];
                    while (tokens[i] != NULL)
                        i++;
                    end = i;
                }
                build_argv(tokens, split_pos[s], end, argvv, MAX_NUM_TOKENS);
                if (!argvv[0])
                    continue;
                if (strcmp(argvv[0], "cd") == 0)
                {
                    if (argvv[1] == NULL || argvv[2] != NULL)
                        fprintf(stderr, "cd: usage: cd <directory>\n");
                    else if (chdir(argvv[1]) != 0)
                        perror("cd");
                    continue;
                }

                pid_t cpid = -1;
                (void)launch_simple(argvv, 0, &cpid, 0);
                if (cpid > 0 && pcnt < MAX_CMDS)
                    pids[pcnt++] = cpid;
            }

            fg_count = 0;
            for (int i = 0; i < pcnt; i++)
                fg_list[fg_count++] = pids[i];

            int remaining = pcnt;
            while (remaining > 0)
            {
                int status;
                pid_t w = waitpid(-1, &status, 0);
                if (w > 0)
                {
                    remaining--;
                    if (WIFEXITED(status))
                    {
                        int code = WEXITSTATUS(status);
                        if (code != 0)
                        {
                            printf("EXITSTATUS: %d\n", code);
                            fflush(stdout);
                        }
                    }
                    else if (WIFSIGNALED(status))
                    {
                        printf("EXITSTATUS: 1\n");
                        fflush(stdout);
                    }
                }
                else if (w < 0 && errno == ECHILD)
                {
                    break;
                }
            }
            fg_count = 0;
            got_sigint = 0;

            free_tokens(tokens);
            continue;
        }

        free_tokens(tokens);
    }

    return 0;
}
