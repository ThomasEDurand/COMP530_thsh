/* COMP 530: Tar Heel SHell
 *
 * This module implements command parsing, following the grammar
 * in the assignment handout.
 */

#include "thsh.h"
#include <stdlib.h>

/* This function returns one line from input_fd
 *
 * buf is populated with the contents, including the newline, and
 *      including a null terminator.  Must point to a buffer
 *      allocated by the caller, and may not be NULL.
 *
 * size is the size of *buf
 *
 * Return value: the length of the string (not counting the null terminator)
 *               zero indicates the end of the input file.
 *               a negative value indicates an error (e.g., -errno)
 */
int read_one_line(int input_fd, char *buf, size_t size) {
  int count, rv;
  // pointer to next place in cmd to store a character
  char *cursor;
  // the last character that was written into cmd
  char last_char;

  assert (buf);

  /*
   * We want to continue reading characters until:
   *   - read() fails (rv will become 0) OR
   *   - count == MAX_INPUT-1 (we have no buffer space left) OR
   *   - last_char was '\n'
   * so we continue the loop while:
   *   rv is nonzero AND count < MAX_INPUT - 1 AND last_char != '\n'
   *
   * On every iteration, we:
   *   - increment cursor to advance to the next char in the cmd buffer
   *   - increment count to indicate we've stored another char in cmd
   *     (this is done during the loop termination check with "++count")
   *
   * To make the termination check work as intended, the loop starts by:
   *   - setting rv = 1 (so it's initially nonzero)
   *   - setting count = 0 (we've read no characters yet)
   *   - setting cursor = cmd (cursor at start of cmd buffer)
   *   - setting last_char = 1 (so it's initially not '\n')
   *
   * In summary:
   *   - START:
   *      set rv = 1, count = 0, cursor = cmd, last_char = 1
   *   - CONTINUE WHILE:
   *      rv (is nonzero) && count < MAX_INPUT - 1 && last_char != '\n'
   *   - UPDATE PER ITERATION:
   *      increment count and cursor
   */
  for (rv = 1, count = 0, cursor = buf, last_char = 1;
       rv && (last_char != '\n') && (++count < (size - 1)); cursor++) {

    // read one character
    // file descriptor 0 -> reading from stdin
    // writing this one character to cursor (current place in cmd buffer)
    rv = read(input_fd, cursor, 1);
    last_char = *cursor;
  }
  // null terminate cmd buffer (so that it will print correctly)
  *cursor = '\0';

  // Deal with an error from the read call
  if (!rv) {
    count = -errno;
  }

  return count;
}


/* Parse one line of input.
 *
 * This function should populate a two-dimensional array of commands
 * and tokens.  The array itself should be pre-allocated by the
 * caller.
 *
 * The first level of the array is each stage in a pipeline, at most MAX_PIPELINE long.
 * The second level of the array is each argument to a given command, at most MAX_ARGS entries.
 * In each command buffer, the entry after the last valid entry should be NULL.
 * After the last valid pipeline buffer, there should be one command entry with just a NULL.
 *
 * For instacne, a simple command like "cd" should parse as:
 *  commands[0] = ["cd", '\0']
 *  commands[1] = ['\0']
 *
 * The first "special" character to consider is the vertical bar, or "pipe" ('|').
 * This splits a single line into multiple sub-commands that form the pipeline.
 * We will implement pipelines in lab 2, but for now, just use this character to delimit
 * commands.
 *
 * For instance, the command: "ls | grep foo\n" should be broken into:
 *
 * commands[0] = ["ls", '\0']
 * commands[1] = ["grep", "foo", '\0']
 * commands[2] = ['\0']
 *
 * Hint: Make sure to remove the newline at the end
 *
 * Hint: Make sure the implementation is robust to extra whitespace, like: "grep      foo"
 *       should still be parsed as:
 *
 * commands[0] = ["grep", "foo", '\0']
 * commands[1] = ['\0']
 *
 * This function should ignore anything after the '#' character, as
 * this is a comment.
 *
 * Finally, the command should identify file redirection characters ('<' and '>').
 * The string right after these tokens should be returned using the special output
 * parameters "infile" and "outfile".  You can assume there is at most one '<' and one
 * '>' character in the entire inbuf.
 *
 * For example, in input: "ls > out.txt", the return should be:
 *   commands[0] = ["ls", '\0']
 *   commands[1] = ['\0']
 *   outfile = "out.txt"
 *
 * Hint: Be sure your implementation is robust to arbitrary (or no) space before or after
 *       the '<' or '>' characters.  For instance, "ls>out.txt" and "ls      >      out.txt"
 *       are both syntactically valid, and should parse identically to "ls > out.txt".
 *       Similarly, "ls|grep foo" is also syntactically valid.
 *
 * You do not need to handle redirection of other handles (e.g., "foo 2>&1 out.txt").
 *
 * inbuf: a NULL-terminated buffer of input.
 *        This buffer may be changed by the function
 *        (e.g., changing some characters to \0).
 *
 * length: the length of the string in inbuf.  Should be
 *         less than the size of inbuf.
 *
 * commands: a two-dimensional array of character pointers, allocated by the caller, which
 *           this function populates.
 *
 *
 * return value: Number of entries populated in commands (1+, not counting the NULL),
 *               or -errno on failure.
 *
 *               In the case of a line with no actual commands (e.g.,
 *               a line with just comments), return 0.
 */

// insert a space before and after #, |, >, < to make parasable with strtok
char* addSpaces(char* buff){
    int sCount = 0;
    int buffLen = strlen(buff);
    for(int i = 0; i<buffLen; i++){
        if(buff[i] == '#' || buff[i] == '|' || buff[i] == '>' || buff[i] == '<'){
            sCount++;
        }
    }
    int newBuffLen = buffLen+2*sCount; // new buffer length is the old + 2 spaces per special char
    char* spacedBuffer = (char* ) malloc(newBuffLen*sizeof(char));
  
    int k=0;
    for(int i = 0; i<buffLen; i++){
        if(buff[i] == '#' || buff[i] == '|' || buff[i] == '>' || buff[i] == '<'){
            spacedBuffer[k++] = ' ';
            spacedBuffer[k++] = buff[i];
            spacedBuffer[k++] = ' ';
        } else {
            spacedBuffer[k++] = buff[i];
        }
    }
    spacedBuffer[k-1] = ' '; //overwrite final '\0' to be a space
    spacedBuffer[k] = '\0'; // then add '\0' after
    return spacedBuffer;  
}

int parse_line (char *inbuf, size_t length, char *commands [MAX_PIPELINE][MAX_ARGS], char **infile, char **outfile) {
    inbuf = addSpaces(inbuf);
    char* token = strtok(inbuf, " ");
    char* prev;
    int i = 0, j = 0;
    int k = 0, p = 0; //k output p input buffer 
    commands[i][j] = token;
    while(token != NULL){ 
    // printf("token: %s\n", token);
        if(prev != NULL && prev[0] == '>'){
            outfile[k++] = token;
        } else if (prev != NULL && prev[0] == '<') {
            infile[p++] = token;
        } else {
            if(token[0] == '|'){
	        commands[i++][j] = '\0'; //set end of previous buffer to terminating character
	        j=0;
            } else if(token[0] == '<' || token[0] == '>'){
            // on next cycle will be processed as prev;
            } else if (token[0] == '#'){
                commands[i][j] = '\0';
                break;
            } else {
                commands[i][j++] = token;
            }
        }
        prev = token;
        token = strtok(NULL, " ");
    }
    commands[++i][0] = '\0';
    return i;
    return -ENOSYS;
}
