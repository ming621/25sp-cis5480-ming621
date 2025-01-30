#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "catch.hpp"

extern "C" {
  #include "./Vec.h"
}

using namespace std;

static ptr_t kOne   = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(1U)));
static ptr_t kTwo   = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(2U)));
static ptr_t kThree = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(3U)));
static ptr_t kFour  = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(4U)));
// static ptr_t kFive  = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(5U)));

// Check that the provided function
// with the provided args
// will result in the program calling panic()
template<typename F, typename... Args>
bool check_panics(F func, Args... args) { 
  pid_t pid = fork();

  if (pid == -1) {
    perror("Could Not Check That Action Aborts\n");
    return false;
  }

  if (pid == 0) {
    signal(SIGABRT, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);

    func(args...);
    exit(EXIT_FAILURE);
  } 

  int status = 0;
  pid_t wait_ret = waitpid(pid, &status, 0);
  if (wait_ret == -1) {
    perror("Could Not Check That Action Aborts\n");
    return false;
  }

  return WIFSIGNALED(status) && (WTERMSIG(status) == SIGABRT);
}

TEST_CASE("Panic on Out of Bounds Access", "[panic]") {
  Vec v = vec_new(3, nullptr);
  vec_push_back(&v, kOne);

  REQUIRE(check_panics(vec_get, &v, 5));      // Accessing an index out of bounds
  REQUIRE(check_panics(vec_set, &v, 3, kOne)); // Setting an index out of bounds
  vec_destroy(&v);
}

TEST_CASE("Panic on Insert Out of Bounds", "[panic]") {
  Vec v = vec_new(3, nullptr);
  vec_push_back(&v, kOne);

  REQUIRE(check_panics(vec_insert, &v, 10, kTwo)); 
  vec_destroy(&v);
}

TEST_CASE("Panic on Erase Out of Bounds", "[panic]") {
  Vec v = vec_new(3, nullptr);
  vec_push_back(&v, kOne);

  REQUIRE(check_panics(vec_erase, &v, 2));
  vec_destroy(&v);
}

TEST_CASE("Panic on Failed Resize Allocation", "[panic]") {
  Vec v = vec_new(1, nullptr);

  // fixed from before, to crash on all systems. 
  REQUIRE(check_panics(vec_resize, &v, UINT64_MAX)); // this is 16777216 terabytes.
  vec_destroy(&v);
}

// Should be removed; resize takes in a size_t. So this is an incorrect test.
// TEST_CASE("Panic on Resize to Invalid Capacity", "[panic]") {
//   Vec v = vec_new(5, nullptr);

//   REQUIRE(check_panics(vec_resize, &v, -1)); 
//   vec_destroy(&v);
// }

TEST_CASE("Panic on Access After Discards Elements", "[panic]") {
  Vec v = vec_new(10, nullptr);

  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kThree);

  REQUIRE(vec_pop_back(&v)); // Resize down to discard elements
  REQUIRE(check_panics(vec_get, &v, 2)); // Accessing discarded element
  REQUIRE(check_panics(vec_set, &v, 2, kFour)); // Setting discarded element
  vec_destroy(&v);
}

TEST_CASE("Panic on Access After Clear", "[panic]") {
  Vec v = vec_new(5, nullptr);

  vec_push_back(&v, kOne);
  vec_clear(&v);

  REQUIRE(check_panics(vec_get, &v, 0)); 
  REQUIRE(check_panics(vec_set, &v, 0, kTwo)); 
  vec_destroy(&v);
}
