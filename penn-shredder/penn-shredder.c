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
}

void handle_sigint(int signo) {
  (void)signo;
  //dprintf(STDERR_FILENO, "[SIGINT HANDLER] current_child = %d\n",
          //current_child);
  if (current_child == 0) {
    //printf(STDERR_FILENO, "[SIGINT HANDLER] No child -> newline\n");
    write(STDERR_FILENO, "\n", 1);
  } else {
    if (kill(-current_child, SIGINT) == -1) {
      perror("[SIGINT HANDLER] kill failed");
    }
  }
}

void handle_sigusr1(int signo) {
  (void)signo;
  // Do nothing, just a signal to synchronize
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

  // Set up signal handlers
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_sigalrm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
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

  struct sigaction sa2;
  memset(&sa2, 0, sizeof(sa2));
  sa2.sa_handler = handle_sigusr1;
  sigemptyset(&sa2.sa_mask);
  sa2.sa_flags = 0;
  if (sigaction(SIGUSR1, &sa2, NULL) == -1) {
    perror("SIGUSR1 error");
    exit(EXIT_FAILURE);
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

      if (setpgid(0, 0) == -1) {
        perror("[CHILD] setpgid failed");
      } else {
        //fprintf(stderr, "[CHILD] setpgid succeeded, PGID=%d\n", getpgid(0));
      }

      // Notify parent that setpgid is done
      kill(getppid(), SIGUSR1);

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

      // Wait for child to complete setpgid
      sigset_t mask;
      sigemptyset(&mask);
      sigsuspend(&mask);

      if (setpgid(pid, pid) == -1) {
        perror("[PARENT] setpgid failed");
      } else {
        //fprintf(stderr, "[PARENT] setpgid succeeded, child's PGID=%d\n",
                //getpgid(pid));
      }

      pid_t child_pgid = getpgid(pid);
      if (tcsetpgrp(STDIN_FILENO, child_pgid) == -1) {
        perror("[PARENT] tcsetpgrp failed");
      } else {
        //fprintf(stderr, "[PARENT] Terminal foreground PGID set to %d\n",
                //child_pgid);
      }

      if (timeout > 0) {
        alarm(timeout);
      } else {
        alarm(0);
      }

      int status;
      pid_t w;
      do {
        w = waitpid(pid, &status, 0);
      } while (w < 0 && errno == EINTR);

      if (w == -1) {
        perror("waitpid error");
        exit(EXIT_FAILURE);
      } else {
        if (alarm_flag) {
          kill(pid, SIGKILL);
          write(STDERR_FILENO, CATCHPHRASE, strlen(CATCHPHRASE));
          waitpid(pid, NULL, 0);
        }
      }

      pid_t parent_pgid = getpgrp();
      struct sigaction ignore, old;
      memset(&ignore, 0, sizeof(ignore));
      ignore.sa_handler = SIG_IGN;
      sigemptyset(&ignore.sa_mask);
      sigaction(SIGTTOU, &ignore, &old);
      if (tcsetpgrp(STDIN_FILENO, parent_pgid) == -1) {
        perror("[PARENT] tcsetpgrp (restore) failed");
      } else {
        //fprintf(stderr, "[PARENT] Terminal foreground PGID restored to %d\n",
                //parent_pgid);
      }

      sigaction(SIGTTOU, &old, NULL);

      alarm(0);
      current_child = 0;
      free(cmd1);
      free(argv1);
    }
  }
  return EXIT_SUCCESS;
}