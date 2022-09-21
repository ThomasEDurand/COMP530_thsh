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

  int nullIdx = 0; 
  for(int i = 0; i < (int) strlen(strEnv)-1; i++){
    if(strEnv[i] == ':' && strEnv[i+1] == ':'){
      return ++nullIdx;
    } else if (strEnv[i] == ':') {
      nullIdx++;
    }
  }
  return -1;
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
  char *pathString = (char *) malloc(4096 * sizeof(char));
  path_table = (char **) malloc(64 * sizeof(char*));
  strcpy(pathString, getenv("PATH"));

  char* token = strtok(pathString, ":");
  token = rmvSlashes(token);
  int i = 0;
  int nullIdx = includeNull();
  while(token != NULL){
    if(i == nullIdx){
        path_table[i++] = "./";
    }
    path_table[i++] = token; 
    token = strtok(NULL, ":");
    token = rmvSlashes(token);
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


static int job_counter = 0;

struct kiddo {
  int pid;
  struct kiddo *next; // Linked list of sibling processes
};

// A job consists of a unique numeric ID and
// one or more processes
struct job {
  int id;
  struct kiddo *kidlets; // Linked list of child processes
  struct job *next; // Linked list of active jobs
};

// A singly linked list of active jobs.
static struct job *jobbies = NULL;

/* Initialize a job structure
 *
 * Returns an integer ID that represents the job.
 */
int create_job(void) {
  struct job *tmp;
  struct job *j = malloc(sizeof(struct job));
  j->id = ++job_counter;
  j->kidlets = NULL;
  j->next = NULL;
  if (jobbies) {
    for (tmp = jobbies; tmp && tmp->next; tmp = tmp->next) ;
    assert(tmp!=j);
    tmp->next = j;
  } else {
    jobbies = j;
  }
  return j->id;
}

/* Helper function to walk the job list and find
 * a given job.
 *
 * remove: If true, remove this job from the job list.
 *
 * Returns NULL on failure, a job pointer on success.
 */
static struct job *find_job(int job_id, bool remove) {
  struct job *tmp, *last = NULL;
  for (tmp = jobbies; tmp; tmp = tmp->next) {
    if (tmp->id == job_id) {
      if (remove) {
        if (last) {
          last->next = tmp->next;
        } else {
          assert (tmp == jobbies);
          jobbies = NULL;
        }
      }
      return tmp;
    }
    last = tmp;
  }
  return NULL;
}

/* Given the command listed in args,
 * try to execute it.
 *
 * If the first argument starts with a '.'
 * or a '/', it is an absolute path and can
 * execute as-is.
 *
 * Otherwise, search each prefix in the path_table
 * in order to find the path to the binary.
 *
 * Then fork a child and pass the path and the additional arguments
 * to execve() in the child.  Wait for exeuction to complete
 * before returning.
 *
 * stdin is a file handle to be used for standard in.
 * stdout is a file handle to be used for standard out.
 *
 * If stdin and stdout are not 0 and 1, respectively, they will be
 * closed in the parent process before this function returns.
 *
 * job_id is the job_id allocated in create_job
 *
 * Returns 0 on success, -errno on failure
 */
int attemptExec(char *args[MAX_ARGS], char* prgmPath, int infileFD, int outfileFD){
    struct stat buf;
    if(stat(prgmPath, &buf) == 0){
        int childPID = fork();
        if(childPID == 0){
            if(infileFD!=STDIN_FILENO){
                dup2(infileFD,STDIN_FILENO);
                close(infileFD);
            }
            if(outfileFD != STDOUT_FILENO){
                dup2(outfileFD,STDOUT_FILENO);
                close(outfileFD);
            }
            execv(prgmPath, args);
            exit(1);
        } else {
            int rv = wait(NULL);
            free(prgmPath);
            return rv;
        }
    }
    return 0;
}

int run_command(char *args[MAX_ARGS], int infileFD, int outfileFD, int job_id) {
  /* Lab 1: Your code here */
  int rv = 0;
  int l = sizeof(path_table)-1; //last entry in terminating char
  char* prgm = args[0];

  
  // Absolute path
  int r = attemptExec(args, prgm, infileFD, outfileFD);
  if(r!=0){return 0;}
 
  // Check if built in
  r = 0;
  int *retval = &r;
  int builtIn = handle_builtin(args, infileFD, outfileFD, retval);
  if(builtIn != 0) {
    return 0;
  }
  // Check paths
  for(int i=0; i<l;i++){
      char *prgmPath = malloc(strlen(path_table[i]) + strlen(prgm)+2);
      strcpy(prgmPath, path_table[i]);
      strcat(prgmPath, "/");
      strcat(prgmPath, prgm);
    
      int rv = attemptExec(args, prgmPath, infileFD, outfileFD);
      if(rv != 0){ return 0;}  
  }
  // Suppress the compiler warning that find_job is not used in the starer code.
  // You may remove this line if/when you use find_job in your code.
  printf("Failed to run command - error -2\n");
  (void)&find_job;
  return rv;
}

/* Wait for the job to complete and free internal bookkeeping
 *
 * job_id is the job_id allocated in create_job
 *
 * exit_code is the exit code from the last child process, if it executed.
 *           This parameter may be NULL, and is only set if the return
 *           value is zero.  This is the same as the wstatus parameter
 *           to waitpid variants, and can be used with functions such
 *           as WIFEXITED.  If this job includes multiple
 *           processes, the exit code will be the last process.
 *
 * Returns zero on success, -errno on error.
 */
int wait_on_job(int job_id, int *exit_code) {
  int ret = 0;
  return ret;
}

