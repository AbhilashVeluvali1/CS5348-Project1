#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

void printError();
void batchMode(FILE *f, char *buffer);
void interactive(char *buffer);
// global variables
char path[100] = "/bin/";

// Parse Method
char **parse(char *input, char *delimiter)
{
  int threshold = 20;
  char **token = malloc(threshold * sizeof(char *));
  char *strTok = strtok(input, delimiter);
  int i = 0;
  while (strTok != NULL)
  {
    token[i++] = strTok;
    if (threshold < i)
    {
      threshold *= 2;
      token = realloc(token, threshold * sizeof(char *));
    }
    strTok = strtok(NULL, delimiter);
  }
  return token;
}
// Error Method
void builtInCmdExit(char *token)
{
  if (token != NULL)
  {
    printError();
  }
  else
  {
    printf("\nin exit\n");
    exit(0);
  }
}
// method for chdir()
void chdirCmd(char **token)
{
  if (token[1] == NULL || token[2] != NULL)
  {
    printError();
  }
  else if (token[1] != NULL)
  {
    int result = chdir(token[1]);
    if (result != 0)
    {
      perror("chdir");
    }
  }
}
// custom error method
void printError()
{
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}
// method for path
char *pathCmd(char **token)
{
  path[0] = '\0';
  int i = 1;
  while (token[i] != NULL)
  {
    strcat(path, token[i++]);
    strcat(path, "|");
  }
  return path;
}
// method to redirect
int checkRedirections(char **token)
{
  int isRedirect = -1, i = 0;
  while (strcmp(token[i], ">") != 0)
  {
    i++;
  }
  if (strcmp(token[i], ">") == 0)
  {
    isRedirect = i;
    printf("isRedirect%d\n", isRedirect);
    return isRedirect;
  }
  printf("isRedirect%d\n", isRedirect);
  return isRedirect;
}
// evecv Method
void execute(char **token, char *path)
{
  char *delim = "|";
  char **pathToken = parse(path, delim);
  int i = 0;
  while (pathToken[i] != NULL)
  {
    char *searchPath = strcat(pathToken[i], token[0]);
    if (access(searchPath, X_OK) == 0)
    {
      if (execv(searchPath, token) == -1)
      {
        printError();
      }
    }
    i++;
  }
}
// driver method
int main(int argc, const char *argv[])
{
  char *builtInCmds[] = {"cd", "exit", "path"};
  size_t bufferLen = 0;
  char *buffer = malloc(bufferLen * sizeof(char));
  // batch mode
  if (argv[1] != NULL)
  {
    const char *file = argv[1];
    FILE *f = fopen(file, "r");
    while(getline(&buffer, &bufferLen, f) != -1){
      batchMode(f, buffer);
    }
    fclose(f);
    exit(0); 
  }
  // Interactive mode
  printf("dash> ");
  while (1)
  {
    getline(&buffer, &bufferLen, stdin);
    interactive(buffer);
    printf("dash> ");
  }
  return 0;
}

//batchMode method
void batchMode(FILE *f, char *buffer)
{
  if (f != NULL)
  {
      char *delim = "\n ";
      char **tokens = parse(buffer, delim);
      int i = 0;
      if (strcmp(tokens[0], "exit") == 0)
      {
        builtInCmdExit(tokens[1]);
      }
      else if (strcmp(tokens[0], "path") == 0)
      {
        pathCmd(tokens);
      }
      else if (strcmp(tokens[0], "cd") == 0)
      {
        chdirCmd(tokens);
      }
      else
      {
        int cPid = fork();
        if (cPid == -1)
        {
          perror("fork");
        }
        else if (cPid == 0)
        {
          //char *searchPath = strcat(path, tokens[0]);
          execute(tokens, path);
        }
        else
        {
          wait(NULL);
        }
      }
  }
}

//interactiveMode method
void interactive(char *buffer)
{
  char *delim = "\t\r\n\v\f ";
  char **tokens = parse(buffer, delim);
  if (strcmp(tokens[0], "exit") == 0)
  {
    builtInCmdExit(tokens[1]);
  }
  else if (strcmp(tokens[0], "path") == 0)
  {
    pathCmd(tokens);
  }
  else if (strcmp(tokens[0], "cd") == 0)
  {
    chdirCmd(tokens);
  }
  else
  {
    int cPid = fork();
    if (cPid == -1)
    {
      printError();
    }
    else if (cPid == 0)
    {
      execute(tokens, path);
    }
    else
    {
      wait(0);
    }
  }
}