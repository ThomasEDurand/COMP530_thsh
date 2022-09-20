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
    //printf("Running thsh in debug mode\n");
  }

  FILE * stream;
  int nonInteractive = 0;
  if(debugMode != 1 && argv != NULL && argv[1] != NULL){
      printf("interactive mode\n");
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

    if (!input_fd) {
      ret = print_prompt();
      if (ret <= 0) {
	    // if we printed 0 bytes, this call failed and the program
	    // should end -- this will likely never occur.
	    finished = true;
	    break;
      }
    }

    // Reset memory from the last iteration
    for(int i = 0; i < MAX_PIPELINE; i++) {
      for(int j = 0; j < MAX_ARGS; j++) {
          parsed_commands[i][j] = NULL;
      }
    }
    
    if(nonInteractive==1){
        char line[MAX_PIPELINE];
        if(fgets(line, MAX_PIPELINE, stream)==NULL){
            return 0;
        };
        char * newLine = malloc(sizeof(line));
        strcpy(newLine, line);
        pipeline_steps = parse_line(newLine, 0, parsed_commands, &infile, &outfile);
    }
    else {
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
    int inPipe = 0;
    if (pipeline_steps>1){
       inPipe = 1;
    }

    int infileFD = STDIN_FILENO;
    int outfileFD = STDIN_FILENO;
    int i=0;
    char pipeline[4096*4];
    while(i<pipeline_steps){
//        printf("pipeline stage %d\n", i);
        char *currCmd = parsed_commands[i][0];
        if(debugMode){
            fprintf(stderr, "RUNNING:[%s]\n", currCmd);
        }
        
        if(inPipe==0){
            int infileFD = open(infile, O_RDWR);
            if(infile == NULL || infileFD == -1){
  //              printf("infile null\n");
                infileFD = STDIN_FILENO;
            }
            outfileFD = open(outfile, O_RDWR);
            if(outfile == NULL || outfileFD == -1){
    //            printf("oufile null\n");
                outfileFD = STDOUT_FILENO;
            }
        }

        if(inPipe==1){
            if(i==pipeline_steps-1){outfileFD=STDOUT_FILENO;}
            else{
                int outfileFD = open(pipeline, O_RDWR);
                if(outfile == NULL || outfileFD == -1){
      //              printf("outfile null or Error\n");
                    outfileFD = STDOUT_FILENO;
                }
            }
        }

        ret = run_command(parsed_commands[i], infileFD, outfileFD, 0); 

        if(inPipe==1){    
            infileFD = open(pipeline, O_RDWR);
            if(infile == NULL || infileFD == -1){
        //        printf("infile null or Error\n");
                infileFD = STDIN_FILENO;
            }
        }
 
        if(debugMode){
            fprintf(stderr, "ENDED:[%s]\n(ret=%d)\n", currCmd, ret);
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
