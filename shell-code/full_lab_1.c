#include  <stdio.h>
#include  <sys/types.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <sys/wait.h>
#include  <limits.h>
#include  <signal.h>
#include  <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG 64
#define MAX_CMDS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i = 0; i < (int)strlen(line); i++){
    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
        if (tokenNo < MAX_NUM_TOKENS - 1) {
          tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
          strcpy(tokens[tokenNo++], token);
        }
        tokenIndex = 0;
      }
    } else {
      if (tokenIndex < MAX_TOKEN_SIZE - 1) {
        token[tokenIndex++] = readChar;
      }
    }
  }

  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

static void free_tokens(char **tokens) {
  if (!tokens) return;
  for (int i = 0; tokens[i] != NULL; i++) free(tokens[i]);
  free(tokens);
}

/* ---- Background job tracking ---- */
static pid_t bg_pids[MAX_BG];
static int bg_count = 0;

static void add_bg(pid_t pid) {
  if (bg_count < MAX_BG) bg_pids[bg_count++] = pid;
}

/* ---- Foreground process group tracking for SIGINT ---- */
static volatile sig_atomic_t fg_pgid = 0;

static void sigint_handler(int signo) {
  (void)signo;
  pid_t pg = (pid_t)fg_pgid;
  if (pg > 0) {
    kill(-pg, SIGINT);
  }
}

/* Reap any finished background children (non-blocking) */
static void reap_background_terminated(void) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    printf("Shell: Background process finished\n");
    fflush(stdout);
  }
}

/* Execute one simple command given as a NULL-terminated argv vector.
   background==1: do not wait; set its own process group.
   Returns 0 normally; for foreground returns child exit status via printing if nonzero (as per spec). */
static int launch_simple(char *argv[], int background, pid_t *out_pid, pid_t forced_pgid) {
  if (!argv || !argv[0]) return 0;

  /* Built-in: exit handled in caller; cd handled in caller; & not expected here. */

  pid_t pid = fork();
  if (pid < 0) { perror("fork"); return 1; }

  if (pid == 0) {
    /* Child */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    if (forced_pgid > 0) {
      if (setpgid(0, forced_pgid) < 0 && errno != EACCES) { perror("setpgid"); _exit(1); }
    } else {
      if (setpgid(0, 0) < 0 && errno != EACCES) { perror("setpgid"); _exit(1); }
    }

    execvp(argv[0], argv);
    perror("execvp");
    _exit(1);
  } else {
    /* Parent */
    if (forced_pgid > 0) {
      if (setpgid(pid, forced_pgid) < 0 && errno != EACCES) perror("setpgid(parent)");
    } else {
      if (setpgid(pid, pid) < 0 && errno != EACCES) perror("setpgid(parent)");
    }

    if (out_pid) *out_pid = pid;

    if (background) {
      add_bg(pid);
      return 0;
    } else {
      int status;
      fg_pgid = (forced_pgid > 0 ? forced_pgid : pid);
      if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        fg_pgid = 0;
        return 1;
      }
      fg_pgid = 0;

      if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code != 0) { printf("EXITSTATUS: %d\n", code); fflush(stdout); }
      } else if (WIFSIGNALED(status)) {
        printf("EXITSTATUS: 1\n"); fflush(stdout);
      }
      return 0;
    }
  }
}

/* Build argv vector for a subcommand from tokens[i..j) into out_argv (NULL-terminated).
   Returns argc. */
static int build_argv(char **tokens, int start, int end, char *out_argv[], int cap) {
  int k = 0;
  for (int i = start; i < end && k < cap - 1; i++) {
    out_argv[k++] = tokens[i];
  }
  out_argv[k] = NULL;
  return k;
}

int main(int argc, char* argv[]) {
  (void)argc; (void)argv;

  /* Install SIGINT handler for the shell */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigint_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sigaction(SIGINT, &sa, NULL);

  /* Ignore TTY job-control signals for simplicity */
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);

  char  line[MAX_INPUT_SIZE];
  char  **tokens;

  while (1) {
    /* Reap any finished background children before prompting */
    reap_background_terminated();

    /* BEGIN: TAKING INPUT */
    memset(line, 0, sizeof(line));
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) strcpy(cwd, "?");
    printf("%s $ ", cwd);
    fflush(stdout);

    int rc = scanf("%1023[^\n]", line);
                                /**
                                    Read a sequence of characters up to (but not including) the next newline \n.
                                    Stops reading when it sees \n.
                                    Appends a '\0' at the end to make line a proper C string.
                                */
    if (rc == EOF) { printf("\n"); break; }
    if (rc == 0) { getchar(); continue; }
    getchar(); // after pressing the resturn button on the screen this will be used to read the \n character from the input buffer
               // so that next time its not seen during scanf, it will be already consumed
    /* END: TAKING INPUT */

    {
      size_t L = strlen(line);
      if (L < MAX_INPUT_SIZE - 1) { line[L] = '\n'; line[L+1] = '\0'; } //terminate with new line
    }
    tokens = tokenize(line);

    /* Empty command */
    if (tokens[0] == NULL) {
      free_tokens(tokens);
      continue;
    }

    /* exit command */
    if (strcmp(tokens[0], "exit") == 0) {
      /* kill all background processes (their PGID equals their PID) */
      for (int i = 0; i < bg_count; i++) {
        pid_t pg = bg_pids[i];
        if (pg > 0) kill(-pg, SIGTERM);
      }
      usleep(150000);
      for (int i = 0; i < bg_count; i++) {
        pid_t pg = bg_pids[i];
        if (pg > 0) kill(-pg, SIGKILL);
      }
      int st;
      while (waitpid(-1, &st, WNOHANG) > 0) {}
      free_tokens(tokens);
      break;
    }

    /* built-in cd (only as a simple form: cd <dir>) */
    if (strcmp(tokens[0], "cd") == 0) {
      if (tokens[1] == NULL || tokens[2] != NULL) {
        fprintf(stderr, "cd: usage: cd <directory>\n");
      } else if (chdir(tokens[1]) != 0) {
        perror("cd");
      }
      free_tokens(tokens);
      continue;
    }

    /* Detect parallel (&&&) or serial (&&) mode by scanning tokens.
       Assumptions per spec: spaces around && and &&&; only one mode per line. */
    int is_parallel = 0, is_serial = 0;
    for (int i = 0; tokens[i] != NULL; i++) {
      if (strcmp(tokens[i], "&&&") == 0) { is_parallel = 1; break; }
      if (strcmp(tokens[i], "&&") == 0)   { is_serial = 1; }
    }

    if (!is_parallel && !is_serial) {
      /* Single command; handle trailing & for background */
      int bg = 0;
      int last = 0;
      while (tokens[last] != NULL) last++;
      if (last > 0 && strcmp(tokens[last - 1], "&") == 0) {
        bg = 1;
        tokens[last - 1] = NULL;
      }

      (void)launch_simple(tokens, bg, NULL, 0);
      free_tokens(tokens);
      continue;
    }

    /* Build subcommands split by && or &&& */
    int split_pos[MAX_CMDS+1];
    int scount = 0;
    split_pos[scount++] = 0;
    for (int i = 0; tokens[i] != NULL; i++) {
      if ((!is_parallel && strcmp(tokens[i], "&&") == 0) ||
          ( is_parallel && strcmp(tokens[i], "&&&") == 0)) {
        tokens[i] = NULL; /* terminate previous argv in-place */
        split_pos[scount++] = i + 1;
        if (scount >= MAX_CMDS) break;
      }
    }

    if (is_serial) {
      /* Execute each subcommand in foreground, one after another */
      for (int s = 0; s < scount; s++) {
        char *argvv[MAX_NUM_TOKENS];
        build_argv(tokens, split_pos[s], /*end*/split_pos[s+1] ? split_pos[s+1]-1 : split_pos[s], argvv, MAX_NUM_TOKENS);
        if (!argvv[0]) continue;

        if (strcmp(argvv[0], "cd") == 0) {
          if (argvv[1] == NULL || argvv[2] != NULL) fprintf(stderr, "cd: usage: cd <directory>\n");
          else if (chdir(argvv[1]) != 0) perror("cd");
          continue;
        }
        (void)launch_simple(argvv, 0, NULL, 0);
      }
      free_tokens(tokens);
      continue;
    }

    if (is_parallel) {
      /* Launch all in parallel, same foreground PGID (first child's PID) */
      pid_t pids[MAX_CMDS];
      int pcnt = 0;

      /* First pass: create argv vectors and launch; record pids */
      int first_group_pgid = 0;
      for (int s = 0; s < scount; s++) {
        char *argvv[MAX_NUM_TOKENS];
        build_argv(tokens, split_pos[s], /*end*/split_pos[s+1] ? split_pos[s+1]-1 : split_pos[s], argvv, MAX_NUM_TOKENS);
        if (!argvv[0]) continue;
        if (strcmp(argvv[0], "cd") == 0) {
          if (argvv[1] == NULL || argvv[2] != NULL) fprintf(stderr, "cd: usage: cd <directory>\n");
          else if (chdir(argvv[1]) != 0) perror("cd");
          continue;
        }

        pid_t cpid = -1;
        if (first_group_pgid == 0) {
          (void)launch_simple(argvv, 0, &cpid, 0); /* it will set its own pgid=cpid */
          first_group_pgid = cpid;
        } else {
          (void)launch_simple(argvv, 0, &cpid, first_group_pgid); /* join same pgid */
        }
        if (cpid > 0 && pcnt < MAX_CMDS) pids[pcnt++] = cpid;
      }

      /* Now wait for all pids in the parallel group */
      fg_pgid = first_group_pgid;
      int remaining = pcnt;
      while (remaining > 0) {
        int status;
        pid_t w = waitpid(-fg_pgid, &status, 0); /* wait for any in group */
        if (w > 0) {
          remaining--;
          if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code != 0) { printf("EXITSTATUS: %d\n", code); fflush(stdout); }
          } else if (WIFSIGNALED(status)) {
            printf("EXITSTATUS: 1\n"); fflush(stdout);
          }
        } else if (w < 0 && errno == ECHILD) {
          break;
        }
      }
      fg_pgid = 0;

      free_tokens(tokens);
      continue;
    }

    /* Fallback */
    free_tokens(tokens);
  }

  return 0;
}
