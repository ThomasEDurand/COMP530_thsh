/* COMP 530: Tar Heel SHell
 *
 * This file implements a table of builtin commands.
 */
#include "thsh.h"
#include <stdlib.h>

struct builtin {
  const char * cmd;
  int (*func)(char *args[MAX_ARGS], int stdin, int stdout);
};


static char old_path[MAX_INPUT];
static char cur_path[MAX_INPUT];

/* This function needs to be called once at start-up to initialize
 * the current path.  This should populate cur_path.
 *
 * Returns zero on success, -errno on failure.
 */
int init_cwd(void) {
  return 0;
}

/* Handle a cd command.  */
int handle_cd(char *args[MAX_INPUT], int stdin, int stdout) {
  // Note that you need to handle special arguments, including:
  // "-" switch to the last directory
  // "." switch to the current directory.  This should change the
  //     behavior of a subsequent "cd -"
  // ".." go up one directory
  //
  // Hint: chdir can handle "." and "..", but saving
  //       these results may not be the desired outcome...

  // Lab 1: Your code here
  //
  getcwd(cur_path, MAX_INPUT);
  char* cdArg = args[1]; // cd supports 1 arg
  char* newCwd;
  if(cdArg == NULL || cdArg[0] == '\0'){ 
    newCwd = getenv("HOME");  
    getcwd(cur_path, MAX_INPUT);
  } else if (cdArg[0] == '.' && cdArg[1] == '.' && (cdArg[2] == '\0' || cdArg[2] == ' ')) {
    newCwd = "..";
  } else if (cdArg[0] == '.' && (cdArg[1] == ' ' || cdArg[1] == '\0')) {
    newCwd = ".";
  } else if (cdArg[0] == '.' && cdArg[1] == '/'){
    newCwd = cdArg;
  } else if (cdArg[0] == '-'){
    newCwd = old_path;
  } else {
    newCwd = cdArg;
  }

  if(chdir(newCwd) != 0){
    printf("Failed to run command - error -1\n");
  } else {
    strcpy(old_path, cur_path);
    getcwd(cur_path, MAX_INPUT);
  }
  return 42;
}

/* Handle an exit command. */
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout) {
  exit(0);
  return 0; // Does not actually return
}

/* Handle Go heels */
int handle_goheels(char *args[MAX_ARGS], int stdin, int stdout) {
  printf("GO HEELS!\n"); 
  return 2022; // Does not actually return
}





static struct builtin builtins[] = {{"cd", handle_cd},
				    {"goheels", handle_goheels},
				    {"exit", handle_exit},
				    {NULL, NULL}};

/* This function checks if the command (args[0]) is a built-in.
 * If so, call the appropriate handler, and return 1.
 * If not, return 0.
 *
 * stdin and stdout are the file handles for standard in and standard out,
 * respectively. These may or may not be used by individual builtin commands.
 *
 * Places the return value of the command in *retval.
 *
 * stdin and stdout should not be closed by this command.
 *
 * In the case of "exit", this function will not return.
 */
int handle_builtin(char *args[MAX_ARGS], int stdin, int stdout, int *retval) {
  int rv = 0;
  int s = sizeof(builtins)/sizeof(builtins[0])-1;

  for(int i = 0; i<s;i++){
    if (strcmp(args[0],builtins[i].cmd)==0){ 
      rv = builtins[i].func(args, stdin, stdout);
      *retval = rv;
      return 1;
    }
  }
  return 0;
}

/* This function initially prints a default prompt of:
 * thsh>
 *
 * In Lab 1, Exercise 3, you will add the current working
 * directory to the prompt.  As in, if you are in "/home/foo"
 * the prompt should be:
 * [/home/foo] thsh>
 *
 * Returns the number of bytes written
 */
int print_prompt(void) {
  int ret = 0;
  // Print the prompt
  // file descriptor 1 -> writing to stdout
  // print the whole prompt string (write number of
  // bytes/chars equal to the length of prompt)
  //
  const char *prompt = "thsh> ";

  // Lab 1: Your code here
  char * extendedPrompt = malloc(sizeof(char) * (9 + strlen(getenv("PWD")))); // P = 2 bracks + 6 thsh + 1 term
  strcat(extendedPrompt, "[");  
  getcwd(cur_path, MAX_INPUT);
  strcat(extendedPrompt, cur_path);
  strcat(extendedPrompt, "] ");
  strcat(extendedPrompt, prompt);
  prompt = extendedPrompt; 

  ret = write(1, prompt, strlen(prompt));
  return ret;
}
