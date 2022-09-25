/* COMP 530: Tar Heel SHell */

#include "thsh.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int openFile(char * file){ // open file weather it exsits
    int fd = open(file, O_RDWR);
    if (fd==-1){
        fd = open(file, O_RDWR | O_CREAT, 0666);
    }
    return fd;
}

int main(int argc, char **argv, char **envp) {
  // flag that the program should end
  bool finished = 0;
  int input_fd = 0; // Default to stdin
  int ret = 0;

  // Lab 1:
  // Add support for parsing the -d option from the command line
  // and handling the case where a script is passed as input to your shell
  // Lab 1: Your code here
  
  int debugMode = 0, inPipe = 0, execScript = 0, nonInteractive = 0; // FLAGS
  if(argc>1 && argv!=NULL && argv[1]!=NULL && argv[1][0]=='-' && argv[1][1]=='d' && (argv[1][2]=='\0' || argv[1][2]==' ')){
    debugMode = 1;
  }

  FILE * stream;
  if(debugMode != 1 && argv != NULL && argv[1] != NULL){
      nonInteractive = 1;
      stream = fopen(argv[1], "r");
  }

  ret = init_cwd();
  if (ret) {
    printf("Error initializing the current working directory: %d\n", ret);
    return ret;
  }

  ret = init_path();
  if (ret) {
    printf("Error initializing the path table: %d\n", ret);
    return ret;
  } 
  while (!finished) {
    int length;
    // Buffer to hold input
    char cmd[MAX_INPUT];
    // Get a pointer to cmd that type-checks with char *
    char *buf = &cmd[0];
    char *parsed_commands[MAX_PIPELINE][MAX_ARGS];
    char *infile = NULL;
    char *outfile = NULL;
    int pipeline_steps = 0;
    // Reset memory from the last iteration
    for(int i = 0; i < MAX_PIPELINE; i++) {
      for(int j = 0; j < MAX_ARGS; j++) {
          parsed_commands[i][j] = NULL;
      }
    }

    if(execScript == 1 || nonInteractive==1) {
        char line[1024];
        if (fgets(line, MAX_PIPELINE, stream)==NULL){
            if(execScript == 1){
                execScript = 0;
                continue;
            } else {
                return 0;
            }
        }
        pipeline_steps = parse_line(line, 0, parsed_commands, &infile, &outfile);
    }

    //PRINT PROMPT IF EXECUTING NORMALLY
    if (!input_fd) {
        ret = print_prompt();
        if (ret <= 0) { 
	        finished = true;
	        break;
            }
    }
    
    if(execScript == 0 && nonInteractive == 0) {
        // Read a line of input
        length = read_one_line(input_fd, buf, MAX_INPUT);
        if (length <= 0) {
            ret = length;
            break;
        }
        // Pass it to the parser
        pipeline_steps = parse_line(buf, length, parsed_commands, &infile, &outfile);

        
        if (pipeline_steps < 0) {
            printf("Parsing error.  Cannot execute command. %d\n", -pipeline_steps);
            continue;
        }
    }

    int pipeLine[2];
    if (pipeline_steps>1){
       inPipe = 1;
    }

    int infileFD = STDIN_FILENO, outfileFD = STDOUT_FILENO;
    int i=0;
    while(i<pipeline_steps){
        if(parsed_commands[i][0] == NULL){ // Handle empty commands and comments
            i++;
            ret = 0;
            continue;
        }

        // If infile and complex pipeline
        if(inPipe == 1 && i==0 && infile != NULL){
            infileFD = open(infile, O_RDWR);
            if (infileFD==-1){
                infileFD = open(infile, O_RDWR | O_CREAT, 0666);
            }

        }

        char currCmd[sizeof(parsed_commands[i][0])];
        strcpy(currCmd, parsed_commands[i][0]);
        //DEBUG INFO
        if(debugMode){
            fprintf(stderr, "RUNNING: [%s]\n", currCmd);
        }

        //INFILE
        if(infile == NULL && inPipe==0){
            infileFD = STDIN_FILENO;
        } else if (inPipe==0){
            infileFD = openFile(infile);
        }
 
 
        //OUTFILE 
        if(outfile == NULL && inPipe == 0){
            outfileFD = STDOUT_FILENO;
        } else if (inPipe==0){
            outfileFD = openFile(outfile);
        }
        
        //SCRIPT AS INPUT
        struct stat buf;
        if(execScript == 0 && nonInteractive == 0 && stat(currCmd, &buf)==0){
            stream = fopen(parsed_commands[i][0], "r");
            if (stream != NULL){
                execScript = 1;
                ret = 0;
                break;
            }
        }

        // PIPELINE 
        if(inPipe){
            pipe(pipeLine);
            if(i != pipeline_steps-1){
                outfileFD = pipeLine[1];
            } else if(outfile==NULL){
                outfileFD = STDOUT_FILENO;
            } else {
                outfileFD = openFile(outfile);
            }
        }
        
        ret = run_command(parsed_commands[i++], infileFD, outfileFD, 0); 

        if(inPipe){ // SET INFILE FOR NEXT PIPELINE
            close(pipeLine[1]);
            infileFD = pipeLine[0];
        }

        if(debugMode){ //DEBUG INFO
            fprintf(stderr, "ENDED: [%s] (ret=%d)\n", currCmd, ret);
        }
    }
 
    if (ret) { // Do NOT change this if/printf - it is used by the autograder.
      printf("Failed to run command - error %d\n", ret);
    }
  }
  // Only return a non-zero value from main() if the shell itself
  // has a bug.  Do not use this to indicate a failed command.
  return 0;
}
