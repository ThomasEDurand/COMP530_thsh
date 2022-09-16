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

#define MAXPATHLEN 300
#define MAXNUMTOKENS 30

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


int init_path(void) {
	//copy path to string
	char *path = malloc(MAXPATHLEN + 1);
	strcpy(path, getenv("PATH"));	

	//count colons
	int count = 0;
	for(int i=0;path[i];i++) {
		if(path[i]==':') {
			count++;
		}
 	}
	
	//make memory for path table here
	count = 50;//without this line, we get segfault, why?
	path_table = (char**) malloc((count + 1 ) * sizeof(char*));

	int i = 0;
	char *token = strtok(path, ":");
	while (token != NULL) {
		//path_table[i++] = (char*)&token;
		path_table[i++] = token;
		token=strtok(NULL, ":");
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

int main() { 
	init_path();
	print_path_table();
	free(path_table);
	return 0;
}
