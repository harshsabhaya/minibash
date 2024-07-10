#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_INPUT_SIZE 1024
#define MAXIMUM_ARGS 100
#define MAXIMUM_COMMANDS 4

int bgProcessId = -1;

// This func will take input from user and store in input variable
void readUserArguments(char *input)
{
  printf("\x1b[1;33mminibash$\x1b[0m ");
  fgets(input, MAX_INPUT_SIZE, stdin);

  // Remove the newline character if present
  input[strcspn(input, "\n")] = '\0';
}

// this func will get input as string that user enter and separated string and store in args array and return number of arguments
int parsingArguments(char *input, char **args)
{
  int i = 0;
  // string token func will separate the input as space separated and while loop will run untill each space separated args store in args[]
  args[i] = strtok(input, " ");
  while (args[i] != NULL)
  {
    i++;
    args[i] = strtok(NULL, " ");
  }
  return i;
}

// fork - exec wrapper that will take func as argument and call it from it's child process
void forkExecWrapper(void (*func)(char **args, int argc), char **args, int argc)
{
  pid_t pid = fork();

  if (pid < 0)
    exit(1);
  else if (pid == 0)
    // child
    func(args, argc);
  else
    // parent
    wait(NULL);
}

// counting words from file
void countWordsFromFile(char **args, int argc)
{
  execlp("wc", "wc", "-w", args[1], NULL);
}

/*
 printing concated content of the file to stdout
 this function will create an array like ["cat", "file1", "file2", NULL]
*/
void concateFileContent(char **args, int argc)
{
  char *files[MAXIMUM_ARGS];
  int noOfFiles = 0;
  files[noOfFiles] = "cat";

  for (int i = 0; i < argc; i++)
  {
    if (strcmp(args[i], "~") != 0)
    {
      // Storing file names in array
      noOfFiles += 1;
      files[noOfFiles] = args[i];
    }
  }
  files[noOfFiles + 1] = NULL;

  // ! if number of command more than 4 the return error
  if (noOfFiles > 4)
  {
    printf("\x1b[1;31mError =>\x1b[0m Maximun 4 operations supported \n");
    return;
  }

  execvp("cat", files);
  printf("Execution error");
  exit(1);
}

// Using "fore" getting process to foreground
void setProcessInForeground()
{
  if (bgProcessId == -1)
  {
    printf("No such process in background\n");
    return;
  }
  printf("This process, %d, is now running in foreground\n", bgProcessId);
  waitpid(bgProcessId, NULL, 0);
  bgProcessId = -1;
}

/*
  handle redirection commands and storing output into files
  This function will take args like ["ls", "-l", ">", "filename.txt"] and number of argc i.e 4 here
*/
int handleRedirectionOperations(char **args, int *argc)
{
  int inFd = -1, outFd = -1;
  for (int i = 0; i < *argc; i++)
  {
    if (strcmp(args[i], "<") == 0)
    {
      if (*argc > 6)
      {
        printf("\x1b[1;31mError =>\x1b[0m Maximun 4 arguments are accepted for each command \n");
        return -1;
      }

      inFd = open(args[i + 1], O_RDONLY);
      if (inFd < 0)
        return -1;

      // assigning STDIN descriptor
      dup2(inFd, 0);

      close(inFd);

      for (int j = i; j + 2 <= *argc; j++)
        args[j] = args[j + 2];

      *argc -= 2;
      i--;
    }
    else if (strcmp(args[i], ">") == 0)
    {
      if (*argc > 6)
      {
        printf("\x1b[1;31mError =>\x1b[0m Maximun 4 arguments are accepted for each command \n");
        return -1;
      }
      outFd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (outFd < 0)
        return -1;

      // assigning STDOUT descriptor
      dup2(outFd, 1);

      close(outFd);

      for (int j = i; j + 2 <= *argc; j++)
        args[j] = args[j + 2];

      *argc -= 2;
      i--;
    }
    else if (strcmp(args[i], ">>") == 0)
    {
      if (*argc > 6)
      {
        printf("\x1b[1;31mError =>\x1b[0m Maximun 4 arguments are accepted for each command \n");
        return -1;
      }
      outFd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
      if (outFd < 0)
        return -1;

      // assigning STDOUT descriptor
      dup2(outFd, 1);

      close(outFd);

      for (int j = i; j + 2 <= *argc; j++)
        args[j] = args[j + 2];

      *argc -= 2;
      i--;
    }
  }
  return 0;
}

// This func takes args that user enter and run using execvp
int runCommandsFromArgs(char **args, int argc)
{
  // if user did not enter any command then just return i.e when enter hits
  if (argc < 1)
    return -1;

  int isBgProcess = 0;

  // "dter" => Kill the current running minibash
  if (strcmp(args[0], "dter") == 0)
  {
    // Parent process will be the bash in which minibash program is running so kill the parent process
    kill(getppid(), SIGKILL);
    exit(0);
  }
  // syntax: "# filename" =>  will return the number of word present in the file i.e # sample.txt
  else if (strcmp(args[0], "#") == 0 && argc == 2)
  {
    forkExecWrapper(countWordsFromFile, args, argc);
    return 0;
  }
  // Syntax: file1 ~ file2 ~ file3 ~ file4 => this will return the concated content of all the file in sequence
  else if (args[1] != NULL && strcmp(args[1], "~") == 0)
  {
    forkExecWrapper(concateFileContent, args, argc);
    return 0;
  }
  // "fore" is used for taking background process to foreground
  else if (strcmp(args[0], "fore") == 0)
  {
    setProcessInForeground();
    return 0;
  }
  // "+" is used for running process in background e.g ex8 3 4 a + (only 4 argc accepted)
  else if (strcmp(args[argc - 1], "+") == 0)
  {
    isBgProcess = 1;
    args[argc - 1] = NULL;
    argc--;

    if (argc < 1 || argc > 4)
    {
      printf("\x1b[1;31mError =>\x1b[0m Maximun 4 arguments are accepted \n");
      return -1;
    }
  }

  pid_t pid = fork();
  if (pid < 0)
    exit(1);
  else if (pid == 0)
  {
    // Child process
    if (handleRedirectionOperations(args, &argc) == -1)
      exit(1);

    if (argc < 1 || argc > 4)
    {
      printf("\x1b[1;31mError =>\x1b[0m Maximun 4 arguments are accepted \n");
      exit(1);
    }

    if (execvp(args[0], args) < 0)
      exit(1);
  }
  else
  {
    // Parent process
    if (isBgProcess)
    {
      printf("This process, %d, is running in background\n", pid);
      // I have set sleep for 1 sec because program exits before properly running process in background
      sleep(1);
      bgProcessId = pid;
    }
    else
    {
      int status;
      waitpid(pid, &status, 0);
      return status;
    }
  }
  return 0;
}

// This func will handle commands which are pipe separated, and input value will be like this [["ls", "-l"], ["wc", "-w"]]
void executePipeCommands(char *commands[MAXIMUM_COMMANDS + 1][MAXIMUM_ARGS], int noOfCommands)
{
  int fd[2];
  int inputFd = 0;

  for (int i = 0; i < noOfCommands; i++)
  {
    pipe(fd);

    int pid = fork();
    if (pid == 0)
    {
      // every time, in child, first setting input file descriptor so write fd can read from the inputFd
      dup2(inputFd, 0);
      if (i < noOfCommands - 1)
        // write into pipe
        dup2(fd[1], 1);

      // closing
      close(fd[0]);

      if (execvp(commands[i][0], commands[i]) < 0)
        exit(1);
    }
    else
    {
      // wait until child process complete
      wait(NULL);
      close(fd[1]);
      // Here setting input file descripor in a variable because in child process it's closed so child (next commmand) can use to read from pipe
      inputFd = fd[0];
    }
  }
}

// this function will separate pipe command from input string
void handlePipeInput(char *input)
{
  char *commands[MAXIMUM_COMMANDS + 1][MAXIMUM_ARGS];
  char *commandString[MAXIMUM_COMMANDS + 1];

  int noOfCommands = 0;

  // storing each command in commandString as a string e.g ["ls -l", "wc"]
  commandString[noOfCommands] = strtok(input, "|");
  while (commandString[noOfCommands] != NULL)
  {
    noOfCommands++;
    commandString[noOfCommands] = strtok(NULL, "|");
  }

  // ! if number of command more than 4 the return error
  if (noOfCommands > 4)
  {
    printf("\x1b[1;31mError =>\x1b[0m Maximun 4 piping operations supported \n");
    return;
  }

  for (int i = 0; i < noOfCommands; i++)
  {
    // Here, each command will be stored in commands array as e.g [["ls", "-l"], ["wc", "-w"]]
    int argc = parsingArguments(commandString[i], commands[i]);
    if (argc < 1 || argc > 4)
    {
      printf("\x1b[1;31mError =>\x1b[0m Each command should have min 1 and max 4 arguments\n");
      return;
    }
  }

  executePipeCommands(commands, noOfCommands);
}

// Handle commands that are ";" separated
void handleSequenceExecution(char *input)
{
  char *commands[MAXIMUM_COMMANDS + 1];
  int noOfCommands = 0;

  // storing each command in commands as a string e.g ["ls -l", "wc"]
  commands[noOfCommands] = strtok(input, ";");
  while (commands[noOfCommands] != NULL)
  {
    noOfCommands++;
    commands[noOfCommands] = strtok(NULL, ";");
  }

  // ! if number of command more than 4 the return error
  if (noOfCommands > 4)
  {
    printf("\x1b[1;31mError =>\x1b[0m Up to 4 sequential commands supported\n");
    return;
  }

  for (int i = 0; i < noOfCommands; i++)
  {
    char *args[MAXIMUM_ARGS];
    int argc = parsingArguments(commands[i], args);

    if (argc < 1 || argc > 4)
    {
      printf("\x1b[1;31mError =>\x1b[0m Each command should have min 1 and max 4 arguments\n");
      return;
    }
    runCommandsFromArgs(args, argc);
  }
}

// this function will handle condition operations that has used "||"" and/or "&&" upto 4 conditional execution
void handleConditionalStatements(char *input)
{
  /*
    exe -a -b && fail || exe && fail
    command array ["exe", "fail", "exe", "fail"]
    operators array ["&", "|", "&"]
  */
  char *commands[MAXIMUM_COMMANDS + 1];
  char *operators[MAXIMUM_COMMANDS];
  int noOfCommands = 0;

  commands[noOfCommands] = strtok(input, "&&||");
  while (commands[noOfCommands] != NULL)
  {
    operators[noOfCommands] = strtok(NULL, commands[noOfCommands]);
    noOfCommands++;
    commands[noOfCommands] = strtok(NULL, "&&||");
  }

  // ! if number of command more than 4 the return error
  if (noOfCommands > 4)
  {
    printf("\x1b[1;31mError =>\x1b[0m Maximum 4 conditional commands supported\n");
    return;
  }

  int result = 0;
  for (int i = 0; i < noOfCommands; i++)
  {
    char *args[MAXIMUM_ARGS];
    int argc = parsingArguments(commands[i], args);
    if (argc < 1 || argc > 4)
    {
      printf("\x1b[1;31mError =>\x1b[0m Each command should have min 1 and max 4 arguments\n");
      return;
    }

    /*
      This if condition will do below things
      - 1st time "i" will be 0, so, first command execute and return 0 if execution successful otherwise send exit status of the process
      - for 2nd time let's assume that previous command run successful the result is 0 then if operation before 2nd command is && then it will execute, if
        before 2nd command || then it will not execute and vice versa for result is not 0
    */
    if (i == 0 || (result == 0 && strcmp(operators[i - 1], "&") == 0) || (result != 0 && strcmp(operators[i - 1], "|") == 0))
      result = runCommandsFromArgs(args, argc);
  }
}

int main()
{
  char *args[MAXIMUM_ARGS];
  char input[MAX_INPUT_SIZE];

  while (1)
  {
    // Reading user entered command and stored in input
    readUserArguments(input);

    // for conditional execution
    if (strstr(input, "&&") != NULL || strstr(input, "||") != NULL)
      handleConditionalStatements(input);

    // for piping execution
    else if (strchr(input, '|') != NULL)
      handlePipeInput(input);

    // for sequential execution
    else if (strchr(input, ';') != NULL)
      handleSequenceExecution(input);

    /*
     for other operations such as
     # => (counting words),
     ~ => (concate files and print),
     + => (running process in background ),
     fore => (getting process into foregrond),
     <, >, >> => file redirection operations
     */
    else
    {
      int argc = parsingArguments(input, args);
      runCommandsFromArgs(args, argc);
    }
  }

  return 0;
}
