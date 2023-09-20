#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

void printError();
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
int main()
{
  char *builtInCmds[] = {"cd", "exit", "path"};
  size_t bufferLen = 0;
  char *buffer = malloc(bufferLen * sizeof(char));

  bool isRedirect = false;

  printf("dash> ");
  while (1)
  {
    getline(&buffer, &bufferLen, stdin);
    if(strstr(buffer, ">" ) != NULL){
    isRedirect = true;
     }
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
        char *delim = "|";
        char **pathToken = parse(path, delim);
        int i = 0;
        while (pathToken[i] != NULL)
        {
          char *searchPath = strcat(pathToken[i], tokens[0]);
          if (access(searchPath, X_OK) == 0)
          {
              if(isRedirect != true){
                if (execv(searchPath, tokens) == -1)
              {
                printError();
              }
              }
              
          }
          i++;
        }
      }
      else
      {
        wait(0);
      }
    }
    printf("dash> ");
  }
  return 0;
}