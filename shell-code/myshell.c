#include  <stdio.h>
#include  <sys/types.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <sys/wait.h>
#include  <limits.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

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

int main(int argc, char* argv[]) {
  char  line[MAX_INPUT_SIZE];
  char  **tokens;

  while(1) {
    /* BEGIN: TAKING INPUT */
    memset(line, 0, sizeof(line));
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) strcpy(cwd, "?");
    printf("%s $ ", cwd);
    fflush(stdout);

    int rc = scanf("%1[^\n]", line); // after searching the internet I found about this
                                /**
                                    Read a sequence of characters up to (but not including) the next newline \n.
                                    Stops reading when it sees \n.
                                    Appends a '\0' at the end to make line a proper C string.
                                */
    if (rc == EOF) { printf("\n"); break; }
    if (rc == 0) { getchar(); continue; }
    getchar(); // after pressing the resturn button on the screen this will be used to read the \n character from the input buffer
               // so that next time its not seen during scanf, it will be already consumed


    {
      size_t L = strlen(line);
      if (L < MAX_INPUT_SIZE - 1) { line[L] = '\n'; line[L+1] = '\0'; } //terminate with new line
    }
    tokens = tokenize(line);

    // empty command -> prompt again
    if (tokens[0] == NULL) {
      free_tokens(tokens);
      continue;
    }

    if (strcmp(tokens[0], "cd") == 0) {
      if (tokens[1] == NULL || tokens[2] != NULL) {
        fprintf(stderr, "cd: usage: cd <directory>\n");
      } else if (chdir(tokens[1]) != 0) {
        perror("cd");
      }
      free_tokens(tokens);
      continue;
    }

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      free_tokens(tokens);
      continue;
    }

    if (pid == 0) {
      execvp(tokens[0], tokens);
      perror("execvp");
      _exit(1);
    } else {
      int status = 0;
      if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
      } else {
        if (WIFEXITED(status)) {
          int code = WEXITSTATUS(status);
          if (code != 0) {
            printf("EXITSTATUS: %d\n", code);
            fflush(stdout);
          }
        } else {
          printf("EXITSTATUS: 1\n");
          fflush(stdout);
        }
      }
      free_tokens(tokens);
    }
  }
  return 0;
}
