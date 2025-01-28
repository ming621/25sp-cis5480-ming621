#include "./Vec.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_LEN 4096U
#define PROMPT "give me a decimal integer: "

#define INITIAL_CAPACITY 10U

#define BASE_10 10

int main() {
  printf("Hello! Feel free to modify this program as needed!\n");

  char input[BUF_LEN] = {0};
  Vec numbers = vec_new(INITIAL_CAPACITY, NULL);

  while (true) {
    ssize_t res = write(STDERR_FILENO, PROMPT, strlen(PROMPT));
    if (res < 0) {
      fprintf(stderr, "error prompting user\n");
      exit(EXIT_FAILURE);
    }

    res = read(STDIN_FILENO, input, BUF_LEN - 1);
    if (res < 0) {
      fprintf(stderr, "error prompting user\n");
      exit(EXIT_FAILURE);
    } else if (res == 0) {
      break;
    }

    input[res] = '\0';

    char* end = NULL;
    int val = (int)strtol(input, &end, BASE_10);
    if (!isspace(*end)) {
      fprintf(stderr, "error parsing user input into an integer!\n");
      continue;
    }

    // intptr_t is a C defined type of integer
    // that is safe to cast to a pointer
    // uintptr_t is an unsigned version
    vec_push_back(&numbers, (ptr_t)(intptr_t)val);

    fprintf(stderr, "You have typed in the number(s):");
    for (size_t i = 0; i < numbers.length; i++) {
      fprintf(stderr, " %d", (int)(intptr_t)vec_get(&numbers, i));
    }
    fprintf(stderr, "\n");
  }

  vec_destroy(&numbers);

  return EXIT_SUCCESS;
}
