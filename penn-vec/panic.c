#include "./panic.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

[[noreturn]] void print_and_abort(const char* error_message) {
  ssize_t len = (ssize_t)strlen(error_message);
  ssize_t total = 0;
  while (total != len) {
    ssize_t res = write(STDERR_FILENO, error_message, len);
    if (res == 0) {
      break;
    }
    if (res == -1) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }
      break;
    }
    total += res;
  }

  abort();
}
