#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

static volatile pid_t current_child = 0;
static volatile sig_atomic_t alarm_flag = 0;
static int timeout = 0;

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


// Trim whitespace from the command
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
    memmove(buffer, temp, len + 1);
  }
  return len;
}

// Parse the command into arguments
static char** parse(char* cmd, int* argc) {
  int count = 0;
  char* temp = strdup(cmd);
  char* token = strtok(temp, " \t");

  while (token != NULL) {
    count++;
    token = strtok(NULL, " \t");
  }
  free(temp);

  char** argv = malloc((count + 1) * sizeof(char*));
  if (!argv) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
  }

  int i = 0;
  token = strtok(cmd, " \t");
  while (token != NULL) {
    argv[i++] = token;
    token = strtok(NULL, " \t");
  }
  argv[count] = NULL;
  *argc = count;
  return argv;
}

int main(int argc, char* argv[], char* envp[]) {
  if (argc > 2) {
    fprintf(stderr, "should not pass more than 2 arguments");
    return EXIT_FAILURE;
  }
  if (argc == 2) {
    timeout = atoi(argv[1]);
  }

  

  const int max_line_length = 4096;
  char cmd[max_line_length];
  while (true) {
    if (write(STDERR_FILENO, "penn-shredder# ", strlen("penn-shredder# ")) <
        0) {
      perror("write failed");
      exit(EXIT_FAILURE);
    }

    memset(&cmd, 0, sizeof(cmd));
    size_t numBytes = read(STDIN_FILENO, cmd, max_line_length - 1);
    if (numBytes == -1) {
      if (errno == EINTR) {
        continue;
      }
      perror("read failed");
      exit(EXIT_FAILURE);
    }

    if (numBytes == 0) {
      break;
    }

    size_t len = trim(cmd);
    if (len == 0) {
      continue;
    }

    int argc1 = 0;
    char* cmd1 = strdup(cmd);
    char** argv1 = parse(cmd1, &argc1);
    if (argc1 == 0) {
      free(cmd1);
      free(argv1);
      continue;
    }

    alarm_flag = 0;
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      free(cmd1);
      free(argv1);
      exit(EXIT_FAILURE);
    }

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

    if (pid > 0) {
      //set the sig_handler
      current_child = pid;
      // Set up signal handlers
      struct sigaction sa;
      memset(&sa, 0, sizeof(sa));
      sa.sa_handler = handle_sigalrm;
      sigemptyset(&sa.sa_mask);
      sa.sa_flags = 0;
      if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("SIGALRM error");
        exit(EXIT_FAILURE);
      }

      struct sigaction sa1;
      memset(&sa1, 0, sizeof(sa1));
      sa1.sa_handler = handle_sigint;
      sigemptyset(&sa1.sa_mask);
      sa1.sa_flags = 0;
      if (sigaction(SIGINT, &sa1, NULL) == -1) {
        perror("SIGINT error");
        exit(EXIT_FAILURE);
      }

      if (timeout > 0) {
        alarm(timeout);
      } else {
        alarm(0);
      }

      int status;
      waitpid(pid, &status, 0);
      
      if(alarm_flag){
        write(STDERR_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
      }

      alarm(0);
      current_child = 0;
      free(cmd1);
      free(argv1);
    }
  }
  return EXIT_SUCCESS;
}