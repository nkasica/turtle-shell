#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sysexits.h>
#define MAX_LINE_LENGTH 1024
#define MAX_DIRS 255
#define DELIMITERS " \t\n\v\f\r"
#define UNIX_CMD 88

/* directory stack object for "pushd", "dirs", and "popd" shell commands */
typedef struct {
  uint8_t num_dirs;
  char *dir_stack[MAX_DIRS];
  char tmp_holder[MAX_LINE_LENGTH + 1];
} directory_stack;

/* returns user input; maximum length is 1024 characters */
int get_line(char *buffer, uint16_t buf_size) {

  if (!fgets(buffer, buf_size, stdin)) {
    return EXIT_FAILURE;
  } else { /* remove newline character */
    buffer[strcspn(buffer, "\n")] = 0;
  }

  return EXIT_SUCCESS;
}

/* parses user input; fills args with tokenized form of user_input,
   and sets num_arg_out to the total number of arguments the user had */
int parse_input(char *user_input, char **args, uint8_t *num_arg_out) {
  int num_arg = 0;
  char *token = strtok(user_input, DELIMITERS);

  if (token == NULL) {
    printf("NULL token\n");
    return EXIT_FAILURE;
  }

  while (token != NULL) {
    args[num_arg++] = token;
    token = strtok(NULL, DELIMITERS);
  }
  *num_arg_out = num_arg;

  return EXIT_SUCCESS;
}

/* determines whether or not the command is a UNIX command or a
   built-in shell command; if it is a shell command, executes it */
int get_command_type(char *cmd, char **args, int num_args, directory_stack *stack, uint8_t *status) {

  if (!strcmp(cmd, "exit") || !strcmp(cmd, "quit")) { /* "exit" and "quit" */

    printf("Goodbye!\n");
    *status = 0;
    return EXIT_SUCCESS;

  } else if (!strcmp(cmd, "cd")) { /* "cd" */

    if (num_args == 2) { /* cd to argument */
      if (!chdir(args[1])) {
      } else {
        printf("Cannot change to directory %s\n", args[1]);
      }
    } else if (num_args == 1) { /* cd to home directory */
      if (!chdir(getenv("HOME"))) {
      } else {
        printf("Cannot change to directory %s\n", getenv("HOME"));
      }
    } else {
      printf("Invalid number of arguments.\n");
    }
    return EXIT_SUCCESS;

  } else if (!strcmp(cmd, "help")) { /* "help" */

    printf("*****Welcome to turtle!*****\n"
           "To execute a UNIX command, simply type in the command you want.\n"
           "turtle also includes support for many other commands, including:\n"
           "* quit/exit\n"
           "* cd\n"
           "* dirs\n"
           "* pushd\n"
           "* popd\n"
           "Have fun!\n");
    return EXIT_SUCCESS;

  } else if (!strcmp(cmd, "pushd")) {

    if (num_args == 2) {
      if (stack->num_dirs < MAX_DIRS) {
        getcwd(stack->tmp_holder, MAX_LINE_LENGTH + 1);
        if (!chdir(args[1])) {
          stack->dir_stack[stack->num_dirs] = malloc(strlen(stack->tmp_holder) + 1);
          strcpy(stack->dir_stack[stack->num_dirs], stack->tmp_holder);
          stack->num_dirs++;
        } else {
          printf("Cannot change to directory %s\n", args[1]);
        }
      } else {
        printf("Directory stack is full\n");
      }
    } else {
      printf("Invalid number of arguments.\n");
    }
    return EXIT_SUCCESS;

  } else if (!strcmp(cmd, "dirs")) {

    uint8_t i;
    for (i = 0; i < stack->num_dirs; i++) {
      printf("%s\n", stack->dir_stack[i]);
    }
    return EXIT_SUCCESS;

  } else if (!strcmp(cmd, "popd")) {

    if (stack->num_dirs == 0) {
      printf("Directory stack is empty\n");
    } else {
      chdir(stack->dir_stack[stack->num_dirs - 1]);
      free(stack->dir_stack[stack->num_dirs - 1]);
      stack->num_dirs--;
    }
    return EXIT_SUCCESS;

  }

  return UNIX_CMD;
}

/* executes UNIX commands */
void execute_unix_cmd(char **args) {
  pid_t pid;

  if ((pid = fork()) == -1) {

    printf("Forking process failed.\n");

  } else if (pid == 0) { /* child */

    execvp(args[0], args);
    printf("Failed to execute %s\n", args[0]);
    fflush(stdout);
    exit(EX_OSERR);

  } else { /* parent */

    wait(NULL);

  }
}


int main() {
  char buffer[MAX_LINE_LENGTH + 1] = {0}; /* storage for shell input */
  char *args[MAX_LINE_LENGTH + 1] = {0};
  uint8_t running = 1; /* bool representing running status of shell */
  uint8_t num_args = 0;
  directory_stack stack = {0};

  do {
    printf("turtle: ");

    /* reads user-inputted line from stdin */
    if (get_line(buffer, sizeof(buffer)) ==  EXIT_FAILURE) {
      printf("Input could not be read.\n");
      continue;
    }

    /* parses user input into args array */
    if (parse_input(buffer, args, &num_args) == EXIT_FAILURE) {
      continue;
    }

    /* determines the type of command and executes the command */
    if (get_command_type(args[0], args, num_args, &stack, &running) == UNIX_CMD) {
      execute_unix_cmd(args);
    }

    /* reset args */
    memset(args, 0, MAX_LINE_LENGTH + 1);

  } while (running);

  return EXIT_SUCCESS;
}
