#include "penn-shredder.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#define CATCHPHRASE "Bwahaha ... Tonight, I dine on turtle soup!\n"

// Global Variables
static volatile pid_t current_child = 0; //current child pid for parent process
static volatile sig_atomic_t alarm_flag = 0; //track if the alarm happened
static int timeout = 0; //timeout settings

// ===========================================================
// Signal Handlers
// ===========================================================
void handle_sigalrm(int signo) {
  (void)signo;
  alarm_flag = 1;
  kill(current_child, SIGKILL);
}

void handle_sigint(int signo) {
  (void)signo;
  if (current_child == 0) {
    write(STDERR_FILENO, "\n", 1);
  } else {
    kill(current_child, SIGINT);
  }
}

// ===========================================================
// Trim leading and trailing whitespace
// ===========================================================
static size_t trim(char* buffer) {
  char* temp = buffer;
  while (*temp == ' ' || *temp == '\t' || *temp == '\n') {
    temp++;
  }

  char* end = temp + strlen(temp);
  while (end > temp && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
    end--;
  }

  *end = '\0';
  size_t len = end - temp;
  if (temp != buffer) {
    size_t idx = 0;
    while (temp[idx] != '\0') {
      buffer[idx] = temp[idx];
      idx++;
    }
    buffer[idx] = '\0';
  }
  return len;
}

// ===========================================================
// Parse a command to array
// ===========================================================
static char** parse(char* cmd, int* argc) {
  if(cmd == NULL || argc == NULL){
    return NULL;
  }

  int count = 0;
  char* temp = strdup(cmd);
  char* token = strtok(temp, " \t");

  while (token != NULL) {
    count++;
    token = strtok(NULL, " \t");
  }
  free(temp);

  if(count == 0){
    *argc = 0;
    return NULL;
  }

  char** argv = malloc((count + 1) * sizeof(char*));
  if (!argv) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  int idx = 0;
  token = strtok(cmd, " \t");
  while (token != NULL) {
    argv[idx++] = token;
    token = strtok(NULL, " \t");
  }
  argv[count] = NULL;
  *argc = count;
  return argv;
}

// ===========================================================
// Read Command Line from standard input
// ===========================================================
bool readCommandLine(char* cmdBuffer, size_t buffer_size) {
  if (write(STDERR_FILENO, "penn-shredder# ", strlen("penn-shredder# ")) < 0) {
    perror("write failed");
    return false;
  }

  size_t numBytes = read(STDIN_FILENO, cmdBuffer, buffer_size - 1);
  if (numBytes == -1) {
    if (errno == EINTR) {
      cmdBuffer[0] = '\0';
      return true;
    }
    perror("read failed");
    return false;
  }

  if (numBytes == 0) {
    return false;
  }

  cmdBuffer[numBytes] = '\0';
  return true;
}

// ===========================================================
// use sigaction to setup signal handlers for parent
// ===========================================================
void setupParentSignals(void) {
  // Set up signal handlers
  struct sigaction alarm_action;
  memset(&alarm_action, 0, sizeof(alarm_action));
  alarm_action.sa_handler = handle_sigalrm;
  sigemptyset(&alarm_action.sa_mask);
  alarm_action.sa_flags = 0;
  if (sigaction(SIGALRM, &alarm_action, NULL) == -1) {
    perror("SIGALRM error");
    exit(EXIT_FAILURE);
  }

  struct sigaction signal_action;
  memset(&signal_action, 0, sizeof(signal_action));
  signal_action.sa_handler = handle_sigint;
  sigemptyset(&signal_action.sa_mask);
  signal_action.sa_flags = 0;
  if (sigaction(SIGINT, &signal_action, NULL) == -1) {
    perror("SIGINT error");
    exit(EXIT_FAILURE);
  }
}

// ===========================================================
// parse and execute the command by forking a child process 
// to execute and wait for it to terminate
// ===========================================================
void runCommand(char* cmd, char* envp[]) {
  int argc_child = 0;
  char* cmd1 = strdup(cmd);
  char** argv1 = parse(cmd1, &argc_child);
  if (!argv1) {  
    fprintf(stderr, "Invalid command input\n");
    free(cmd1); 
    return;
  }

  if (argc_child == 0) {
    free(cmd1);
    free(argv1);
    return;
  }

  alarm_flag = 0;
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    free(cmd1);
    free(argv1);
    exit(EXIT_FAILURE);
  }
  // if child process
  if (pid == 0) {
    struct sigaction dfl;
    memset(&dfl, 0, sizeof(dfl));
    dfl.sa_handler = SIG_DFL;
    sigemptyset(&dfl.sa_mask);
    sigaction(SIGINT, &dfl, NULL);

    execve(argv1[0], argv1, envp);
    perror("execve failed");
    exit(EXIT_FAILURE);
  }
  // if parent process
  if (pid > 0) {
    // set the sig_handler
    current_child = pid;
    setupParentSignals();

    if (timeout > 0) {
      alarm(timeout);
    } else {
      alarm(0);
    }

    int status;
    waitpid(pid, &status, 0);

    if (alarm_flag) {
      write(STDERR_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
    }

    alarm(0);
    current_child = 0;
    free(cmd1);
    free(argv1);
  }
}

// ===========================================================
// Main
// ===========================================================
int main(int argc, char* argv[], char* envp[]) {
  if (argc > 2) {
    fprintf(stderr, "should not pass more than 2 arguments");
    return EXIT_FAILURE;
  }
  if (argc == 2) {
    char* end;
    long timeout_long = strtol(argv[1], &end, 10);
    if(*end != '\0' || timeout_long < 0){
      fprintf(stderr, "Invalid timeout");
      exit(EXIT_FAILURE);
    }
    timeout = (int)timeout_long;
  }

  const int max_line_length = 4096;
  char cmd[max_line_length];
  while (true) {
    // 1. read user input
    if (!readCommandLine(cmd, max_line_length)) {
      break;
    }
    // 2.trim whitespace
    size_t len = trim(cmd);
    if (len == 0) {
      continue;
    }
    // 3. run command (do fork things)
    runCommand(cmd, envp);
  }
  return EXIT_SUCCESS;
}