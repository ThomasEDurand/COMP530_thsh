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


/* Handle a cd command.  */
int handle_cd(char *args[MAX_INPUT], int stdin, int stdout) {

  // You will implement this in Lab 1.
  // Just return 42 for now (for testing).
  // printf("cd handled\n");
  return 42;
}

/* Handle an exit command. */
int handle_exit(char *args[MAX_ARGS], int stdin, int stdout) {
  exit(0);
  return 0; // Does not actually return
}



static struct builtin builtins[] = {{"cd", handle_cd},
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
      //printf("found %s\n", builtins[i].cmd);
      rv = builtins[i].func(args, stdin, stdout);
      *retval = rv;
      
      //printf("%d\n", retval*);
      return 1;
      //int (*fp)(char *args[MAX_ARGS], int stdin, int stdout);
      //fp = builtins[i].func;
      //printf("%p\n", fp);
      //rv = &fp(args,stdin,stdout);
    }
  }
  return 0;
}

