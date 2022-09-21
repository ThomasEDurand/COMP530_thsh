/* COMP 530: Tar Heel SHell */

#include "thsh.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


int main(int argc, char **argv, char **envp) {
  // flag that the program should end
  bool finished = 0;
  int input_fd = 0; // Default to stdin
  int ret = 0;

  // Lab 1:
  // Add support for parsing the -d option from the command line
  // and handling the case where a script is passed as input to your shell
  // Lab 1: Your code here
  int debugMode = 0;
  if(argc>1 && argv!=NULL && argv[1]!=NULL && argv[1][0]=='-' && argv[1][1]=='d' && (argv[1][2]=='\0' || argv[1][2]==' ')){
    debugMode = 1;
  }

  FILE * stream;
  int nonInteractive = 0;
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
  int execScript = 0;
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

    //PRINT PROMPT IF EXECUTING NORMALLY
    if(execScript == 0 && nonInteractive == 0){
    if (!input_fd) {
      ret = print_prompt();
      if (ret <= 0) {
	    // if we printed 0 bytes, this call failed and the program
	    // should end -- this will likely never occur.
	    finished = true;
	    break;
      }
    }
    }

    // MUTUALTY EXCLUSIVE WITH NONINTERACTIVE 
    if(execScript == 1){ // SCRIPT GIVEN ON COMMAND LINE
        char line[1024];
        if (fgets(line, MAX_PIPELINE, stream)==NULL){
            execScript = 0;
            continue;
        } else {
            pipeline_steps = parse_line(line, 0, parsed_commands, &infile, &outfile);
        }
    } else if(nonInteractive==1){ // SCRIPT PASSED IN AS ARG
        char line[1024];
        if(fgets(line, MAX_PIPELINE, stream)==NULL){
            return 0;
        }
        pipeline_steps = parse_line(line, 0, parsed_commands, &infile, &outfile);
    } else {
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
    // PRINT PROMPT IF EXECUTING SCRIPT
    if(execScript == 1 || nonInteractive == 1){
    if (!input_fd) {
      ret = print_prompt();
      if (ret <= 0) {
	    // if we printed 0 bytes, this call failed and the program
	    // should end -- this will likely never occur.
	    finished = true;
	    break;
      }
    }
    }


    }

    // Just echo the command line for now
    // file descriptor 1 -> writing to stdout
    // print the whole cmd string (write number of
    // chars/bytes equal to the length of cmd, or MAX_INPUT,
    // whichever is less)
    //
    // Comment this line once you implement
    // command handling
    // dprintf(1, "%s\n", cmd);

    // In Lab 1, you will need to add code to actually run the commands,
    // add debug printing, and handle redirection and pipelines, as
    // explained in the handout.
    //
    // For now, ret will be set to zero; once you implement command handling,
    // ret should be set to the return from the command.

/*
    int inPipe = 0;
    int pipe0[2];
    pipe(pipe0);
    int pipe1[2];
    pipe(pipe1);
    if (pipeline_steps>1){
       inPipe = 1;
    }
*/
    int infileFD = STDIN_FILENO;
    int outfileFD = STDOUT_FILENO;
    int i=0;
    while(i<pipeline_steps){
        if(parsed_commands[i] == NULL || parsed_commands[i][0] == NULL){ // Handle empty commands and comments
            i++;
            ret = 0;
            continue;
        }
        //DEBUG INFO

        char *currCmd = malloc(sizeof(parsed_commands[i][0]));
        strcpy(currCmd, parsed_commands[i][0]);
        if(debugMode){
            strcpy(currCmd, parsed_commands[i][0]);
            fprintf(stderr, "RUNNING: [%s]\n", currCmd);
        }

        //INFILE
        infileFD = open(infile, O_RDWR);
        if(infile == NULL){
            infileFD = STDIN_FILENO;
        } else if (infileFD==-1){
           infileFD = open(infile, O_RDWR | O_CREAT, 0666); 
        }
        
        //OUTFILE 
        outfileFD = open(outfile, O_RDWR);
        if(outfile == NULL){
            outfileFD = STDOUT_FILENO;
        } else if (outfileFD==-1){
            outfileFD = open(outfile, O_RDWR | O_CREAT, 0666);
        }

        //SCRIPT AS INPUT
        if(execScript == 0 && nonInteractive == 0){
            stream = fopen(parsed_commands[i][0], "r");
            if (stream != NULL){
                execScript = 1;
                ret = 0;
                break;
            }
        }

        /*
        // PIPELINE CURRENTLY BROKEN
        if(inPipe == 1){
            printf("pipeline stage: %d\n", i);
            if(i%2==0){
                if(i!=0 && infileFD==STDIN_FILENO){
                    infileFD = pipe1[0];
                }
                if(i!=pipeline_steps-1 && outfileFD!=STDOUT_FILENO){
                    outfileFD = pipe0[1];
                }
            } else {
                if(i!=0){
                    infileFD = pipe0[0];
                }
                if(i!=pipeline_steps-1){
                    outfileFD = pipe1[1];
                }
            }
        }
        */

        ret = run_command(parsed_commands[i], infileFD, outfileFD, 0); 

        //DEBUG INFO
        if(debugMode){
            fprintf(stderr, "ENDED: [%s] (ret=%d)\n", currCmd, ret);
        }
        i++;
    }

    // Do NOT change this if/printf - it is used by the autograder.
    if (ret) {
      printf("Failed to run command - error %d\n", ret);
    }
  }
  // Only return a non-zero value from main() if the shell itself
  // has a bug.  Do not use this to indicate a failed command.
  return 0;
}
