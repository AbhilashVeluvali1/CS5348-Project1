#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

void printError();
void batchMode(char *buffer);
void interactive(char *buffer);
void redirection(char *buffer);
void parallel(char *buffer);
char *getAccessPath(char **token);

// global variables
char path[1000] = "/bin";

/** Parse Method
* Parse a string into an array of tokens based on a delimiter.
* @param input     The input string to be parsed.
* @param delimiter The delimiter used to split the input string.
* @return An array of pointers to tokens.
*/
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


/**built-in cmd exit handler
* Handle the built-in "exit" command.
* @param token The token representing the "exit" command (typically NULL).
* This function is responsible for exiting the program when the "exit" command is entered.
* If any arguments are provided after "exit," it prints an error message.
*/
void builtInCmdExit(char *token)
{
  if (token != NULL)
  {
    printError();
  }
  else
  {
    exit(0);
  }
}


/**
* Handle the built-in "chdir" (change directory) command.
* @param token An array of tokens, where token[0] is the command name,
* and token[1] is the directory path to change to.
* This function is responsible for changing the current working directory of the program
* to the specified path. It prints an error message if:
*   1. No directory path is provided.
*   2. More than one argument (directory path) is provided.
*   3. The specified directory path does not exist or cannot be accessed.
*/
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
      printError();
    }
  }
}

/**
* Print a generic error message to the standard error (stderr) stream.
* This function is used to display a predefined error message when an error condition occurs
* in the program. It writes the error message to the stderr stream.
*/
void printError()
{
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}


/**
* Build a colon-separated path string from an array of tokens.
* @param token An array of tokens, where token[0] is the command name,
* and token[1] onwards are the components of the path.
* @return A dynamically allocated string containing the colon-separated path.
* The caller is responsible for freeing this memory when done.
*/
char *pathCmd(char **token)
{

  path[0] = '\0';
  int i = 1;
  while (token[i] != NULL)
  {
    strcat(path, token[i]);
    strcat(path, ":");

    i++;
  }
  return path;
}


// driver method
int main(int argc, const char *argv[])
{
  size_t bufferLen = 0;
  char *buffer = malloc(bufferLen * sizeof(char));
  // Check if a command file is provided as an argument (batch mode).
  if (argv[1] != NULL)
  {
    const char *file = argv[1];
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
      printError();
      exit(0);
    }
    else
    {
      // To check if the file empty
      fseek(f, 0, SEEK_END);
      long size = ftell(f); 
      
      if(size == 0){
        printError();
        exit(0);
      }
      // Process each line from the command file.
      while (getline(&buffer, &bufferLen, f) != -1)
      {
        batchMode(buffer);
      }
    }
    fclose(f);
    exit(0);
  }
  
  // If no command file is provided, enter interactive mode.
  printf("dash> ");
  while (1)
  {  
    
    int getReturn = getline(&buffer, &bufferLen, stdin);
    if(getReturn == 1 && buffer[0] == '\n') {
      // Handle empty input line.
      printf("dash> ");
      continue;

    }

    // Check for various command types.
    if (buffer[0] == '&' || strstr(buffer, "&&") != NULL)
    { 
      // Handle parallel command.
      printError();
      printf("dash> ");
    }
    else if (strstr(buffer, "&") != NULL)
    { 
        // Handle parallel command.   
        parallel(buffer);
        printf("dash> ");
    }
    else if(strstr(buffer,">>") != NULL){
      // Handle append redirection.
      printError();
      printf("dash> ");
    } else if(strstr(buffer,">") != NULL ){
        // Handle redirection (standard output).
        if(strchr(buffer,'>') == strrchr(buffer, '>')){
           redirection(buffer);
           
        }else{
          printError();

        }
        printf("dash> ");
       
    }
    else
    {
      // Handle other interactive commands.   
      interactive(buffer);
      printf("dash> ");
    }
  }
  return 0;
}


/**
* Execute commands in batch mode.
* @param buffer The command buffer to execute.
* This function handles the execution of commands in batch mode.
* It checks for different types of commands (parallel, redirection, etc.) and
* delegates the execution to appropriate functions.
*/
void batchMode(char *buffer)
{
    if(buffer == NULL || buffer[0] == '\n') {
      return;
    }
    if (strstr(buffer, "&&") != NULL)
    {
      printError();
    }
    else if (strstr(buffer, "&") != NULL)
    {
      parallel(buffer);
    } 
    else if(strstr(buffer,">>") != NULL){
      printError();
    } 
    else if(strstr(buffer,">") != NULL ){
      if(strchr(buffer,'>') == strrchr(buffer, '>')){
        redirection(buffer);
        }else{
          printError();
        }
    }
    else
    {
      interactive(buffer);
    }  
}


/**
* Handle and execute interactive mode commands.
* @param buffer The command buffer to execute.
* This function is responsible for handling and executing commands entered in
* interactive mode. It parses the input, checks for built-in commands (e.g., exit,
* path, cd), and executes external commands using fork and execv.
*/
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
      char *searchPath = getAccessPath(tokens);
      if (searchPath != NULL)
      { 
        if (execv(searchPath, tokens) == -1)
        {
          printError();
        }
      }
      else
      {
        printError();
      }
    }
    else
    {
      int status;
      waitpid(cPid, &status, 0); 
    }
  }
}




/**
* Handle command redirection (standard output and error).
* @param buffer The command buffer containing the redirection.
* This function is responsible for handling command redirection (">" operator).
* It extracts the output filename from the command buffer, modifies the command
* to remove the redirection part, and executes the modified command with output
* redirected to the specified file.
*/
void redirection( char *buffer) {
  char *dup = strdup(buffer);
  char **redirectionToken = parse(dup, "\n ");
  char *filename = NULL;
  
  if(strcmp(redirectionToken[0] , ">") == 0){
      printError();
      return;
    }
  
  int i = 0;
  while(redirectionToken != NULL){
    if(strcmp(redirectionToken[i], ">") == 0){
      if(redirectionToken[i+2] == NULL){
        filename = redirectionToken[i+1];
        break;
      }else {
        printError();
        return;
      }
    }
    i++;
  }
  
  if(filename==NULL) {
    printError();
  }
  else {
    char** cmdParser = parse(buffer, " ");
    int i = 0;
    while(cmdParser[i]!=NULL) {
      if(strcmp(cmdParser[i],">")==0) {
        cmdParser[i] = NULL;
        cmdParser[i+1]= NULL;
        break;
      }
      i++;
    }
    int cPid = fork();
    if(cPid==-1) {
      printError();
  }
  else if(cPid==0) {
    int f = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    dup2(f,1);
    dup2(f,2);
    
    if(close(f) == -1){
      printError();
    }
    char* path = getAccessPath(cmdParser);
    if(path!=NULL)
    {
      execv(path,cmdParser);
    }
    else {
      printError();
    }
   
  }
}
}


/**
* Execute multiple commands in parallel.
* @param buffer The command buffer containing multiple commands separated by "&".
* This function is responsible for executing multiple commands concurrently in parallel.
* It splits the input buffer into individual commands and executes each command using
* the `interactive` function.
*/
void parallel(char *buffer)
{
  char *delim = "&\n";
  char **tokens = parse(buffer, delim);
   
  int i = 0;
  while (tokens[i] != NULL)
  {
    if(strstr(tokens[i],">>") != NULL){
      printError();
    } 
    else if(strstr(tokens[i],">") != NULL ){
      //redirection
      if(strchr(tokens[i],'>') == strrchr(tokens[i], '>')){
          redirection(tokens[i]);
        }else{
          printError();
        }
    }else{
        interactive(tokens[i]);
    }
    
    i++;
  }
}

/**
* Find the full path to an executable file by searching through directories in the PATH variable.
* @param token An array of tokens, where token[0] is the command name to search for.
* @return A dynamically allocated string containing the full path to the executable if found,
* or NULL if the command is not found in any of the directories.
*/
char *getAccessPath(char **token)
{
  char* ret= NULL;
  char *delim = ":";
  char **pathToken = parse(path, delim);
  int i = 0;
  char *searchPath = NULL;
  while (pathToken[i] != NULL)
  {
    char *newPath = malloc(sizeof(pathToken[i]) * 20);
    strcat(newPath, pathToken[i]);
    strcat(newPath, "/");
    searchPath = strcat(newPath, token[0]);
    if (access(searchPath, X_OK) == 0)
    {
       ret = searchPath;
    }
    i++;
  }

  return ret;
}
