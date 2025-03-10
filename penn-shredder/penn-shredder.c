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

// Global Variables
static volatile pid_t current_child = 0;      // current child pid for parent
                                              // process
static volatile sig_atomic_t alarm_flag = 0;  // track if the alarm happened
static int timeout = 0;                       // timeout settings

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
// strdup helper function
// ===========================================================
char* my_strdup(const char* str) {
  size_t len = strlen(str);
  char* res = malloc(len + 1);
  if (!res) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }
  for (size_t idx = 0; idx <= len; idx++) {
    res[idx] = str[idx];
  }
  return res;
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
  if (cmd == NULL || argc == NULL) {
    return NULL;
  }

  // parse token and count token in one pass
  char* tokens[MAX_TOKENS];
  int count = 0;

  char* token = strtok(cmd, " \t");
  while (token != NULL) {
    tokens[count] = token;
    count++;
    token = strtok(NULL, " \t");

    if (count >= (MAX_TOKENS - 1)) {
      break;
    }
  }

  // If no tokens found, return NULL
  if (count == 0) {
    *argc = 0;
    return NULL;
  }

  // Allocate the array
  char** argv = malloc((count + 1) * sizeof(char*));
  if (!argv) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < count; i++) {
    argv[i] = tokens[i];
  }
  argv[count] = NULL;

  *argc = count;
  return argv;
}

// ===========================================================
// Read Command Line from standard input
// ===========================================================
bool readCommandLine(char* cmdBuffer, size_t buffer_size) {
  if (write(STDERR_FILENO, PROMPT, strlen(PROMPT)) < 0) {
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
  struct sigaction alarm_action = {0};
  alarm_action.sa_handler = handle_sigalrm;
  sigemptyset(&alarm_action.sa_mask);
  alarm_action.sa_flags = 0;
  if (sigaction(SIGALRM, &alarm_action, NULL) == -1) {
    perror("SIGALRM error");
    exit(EXIT_FAILURE);
  }

  struct sigaction signal_action = {0};
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
  char* cmd_copy = my_strdup(cmd);
  char** argv1 = parse(cmd, &argc_child);
  if (!argv1) {
    free(cmd_copy);
    return;
  }

  if (argc_child == 0) {
    free(cmd_copy);
    free(argv1);
    return;
  }

  alarm_flag = 0;
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    free(cmd_copy);
    free(argv1);
    exit(EXIT_FAILURE);
  }

  // if child process
  if (pid == 0) {
    struct sigaction dfl = {0};
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
    while (1) {
        pid_t wait_res = wait(&status);
        if (wait_res >= 0){
          break;
        }
        if (errno == EINTR){
          continue;
        }
        perror("wait failed");
        exit(EXIT_FAILURE);
    }

    if (alarm_flag) {
      write(STDERR_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
    }

    alarm(0);
    current_child = 0;
    free(cmd_copy);
    free(argv1);
  }
}

// ===========================================================
// Main
// ===========================================================
int main(int argc, char* argv[], char* envp[]) {
  if (argc > 2) {
    return EXIT_FAILURE;
  }
  if (argc == 2) {
    char* end;
    long timeout_long = strtol(argv[1], &end, 10);
    if (*end != '\0' || timeout_long < 0) {
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