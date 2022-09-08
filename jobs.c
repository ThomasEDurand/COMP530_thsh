/* COMP 530: Tar Heel SHell
 *
 * This file implements functions related to launching
 * jobs and job control.
 */

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "thsh.h"

static char ** path_table;

/* Initialize the table of PATH prefixes.
 *
 * Split the result on the parenteses, and
 * remove any trailing '/' characters.
 * The last entry should be a NULL character.
 *
 * For instance, if one's PATH environment variable is:
 *  /bin:/sbin///
 *
 * Then path_table should be:
 *  path_table[0] = "/bin"
 *  path_table[1] = "/sbin"
 *  path_table[2] = '\0'
 *
 * Hint: take a look at getenv().  If you use getenv, do NOT
 *       modify the resulting string directly, but use
 *       malloc() or another function to allocate space and copy.
 *
 * Returns 0 on success, -errno on failure.
 */
int includeNull(void){
  char *strEnv = getenv("PATH");
  for(int i = 0; i < (int) strlen(strEnv)-1; i++){
    if(strEnv[i] == ':' && strEnv[i+1] == ':'){
      return 1;
    }
  }
  return 0;
}

char* rmvSlashes(char *buff){
    if(buff == NULL){
        return NULL;
    }
    int l = strlen(buff)-1;
    while(l >= 0 && buff[l] == '/'){
        l--;
    }
    char* newBuff = (char*) malloc((l+1)*sizeof(char));
    for(int i =0; i<=l;i++){
        newBuff[i] = buff[i];
    }
    newBuff[++l] = '\0';
    return newBuff;
}

int init_path(void) {
  /* Lab 0: Your code here */
  char *pathString = (char *) malloc(4096 * sizeof(char));
  path_table = (char **) malloc(64 * sizeof(char*));
  strcpy(pathString, getenv("PATH"));

  char* token = strtok(pathString, ":");
  token = rmvSlashes(token);
  int i = 0;

  while(token != NULL){
    path_table[i++] = token; 
    token = strtok(NULL, ":");
    token = rmvSlashes(token);
  }
  if (includeNull()){
    path_table[i++] = "./";
  }
  return 0;
}

/* Debug helper function that just prints
 * the path table out.
 */
void print_path_table() {
  if (path_table == NULL) {
    printf("XXXXXXX Path Table Not Initialized XXXXX\n");
    return;
  }

  printf("===== Begin Path Table =====\n");
  for (int i = 0; path_table[i]; i++) {
    printf("Prefix %2d: [%s]\n", i, path_table[i]);
  }
  printf("===== End Path Table =====\n");
}
