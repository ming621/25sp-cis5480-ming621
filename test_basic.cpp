#include "catch.hpp"
#include <stdlib.h>

extern "C" {
  #include "./Vec.h"
}

using namespace std;

static ptr_t kOne   = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(1U)));
static ptr_t kTwo   = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(2U)));
static ptr_t kThree = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(3U)));
static ptr_t kFour  = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(4U)));
static ptr_t kFive  = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(5U)));
static ptr_t kSixetyEight  = reinterpret_cast<ptr_t>((static_cast<uintptr_t>(68U)));


void noop([[maybe_unused]] ptr_t input) {
  
}

static uintptr_t counter = 0;
static int invocations = 0;

void count_constants(ptr_t input) {
  counter += reinterpret_cast<uintptr_t>(input);
  invocations += 1;
}

// --- Basic Operations ---
TEST_CASE("Construction and Destruction", "[basic]") {
  Vec v = vec_new(3, nullptr);
  REQUIRE(v.data != nullptr);
  REQUIRE(v.capacity == 3);
  REQUIRE(v.length == 0);
  REQUIRE(v.ele_dtor_fn == nullptr);
  vec_destroy(&v);

  Vec v_custom = vec_new(5, count_constants);
  REQUIRE(v_custom.data != nullptr);
  REQUIRE(v_custom.capacity == 5);
  REQUIRE(v_custom.length == 0);
  REQUIRE(v_custom.ele_dtor_fn == count_constants);
  vec_destroy(&v_custom);
}

// Tests the Macros. 
// Students might get bugs where the statements don't compile within an if statement.
TEST_CASE("Check Empty Vector", "[basic]") {
  Vec v = vec_new(5, nullptr);
  REQUIRE(vec_is_empty(&v));
  vec_push_back(&v, kOne);
  REQUIRE_FALSE(vec_is_empty(&v));
  vec_pop_back(&v);
  REQUIRE(vec_is_empty(&v));
  vec_destroy(&v);
}

// --- Push and Pop Operations ---
TEST_CASE("Push Operations", "[push-pop]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  REQUIRE(v.length == 2);
  REQUIRE(vec_len(&v) == 2); //macro check again.

  REQUIRE(vec_get(&v, 0) == kOne);
  REQUIRE(vec_get(&v, 1) == kTwo);
  vec_destroy(&v);
}

TEST_CASE("Pop Operations", "[push-pop]") {
  Vec v = vec_new(5, count_constants);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  REQUIRE(v.length == 2);

  REQUIRE(vec_pop_back(&v));
  REQUIRE(v.length == 1);

  REQUIRE(vec_pop_back(&v));
  REQUIRE(v.length == 0);

  REQUIRE_FALSE(vec_pop_back(&v));
  vec_destroy(&v);
}

// --- Get and Set Operations ---
TEST_CASE("Get Elements", "[get-set]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  REQUIRE(vec_get(&v, 0) == kOne);
  REQUIRE(vec_get(&v, 1) == kTwo);
  vec_destroy(&v);
}

TEST_CASE("Set Elements", "[get-set]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  vec_set(&v, 1, kThree);
  REQUIRE(vec_get(&v, 1) == kThree);
  vec_destroy(&v);
}


TEST_CASE("Set Elements w/Dtor", "[get-set]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(5, count_constants);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  vec_set(&v, 1, kThree);
  REQUIRE(vec_get(&v, 1) == kThree);
  REQUIRE(counter == 2);
  REQUIRE(invocations == 1);

  vec_set(&v, 1, kFour);
  REQUIRE(vec_get(&v, 1) == kFour);
  REQUIRE(counter == 5);
  REQUIRE(invocations == 2);

  vec_set(&v, 0, kFour);
  REQUIRE(vec_get(&v, 0) == kFour);
  REQUIRE(counter == 6);
  REQUIRE(invocations == 3);

  vec_destroy(&v);
}


// --- Resizing ---
TEST_CASE("Manual Resize", "[resize]") {
  Vec v = vec_new(3, nullptr);
  vec_resize(&v, 6);
  REQUIRE(v.capacity == 6);
  REQUIRE(v.length == 0);
  vec_destroy(&v);
}

TEST_CASE("Automatic Resize on Push", "[resize]") {
  Vec v = vec_new(2, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kThree); // Triggers resize
  REQUIRE(v.capacity == 4); // Must be double
  REQUIRE(v.length == 3);
  vec_destroy(&v);
}

TEST_CASE("Automatic Resize on Push with 0 Capacity Vector", "[resize]") {
  Vec v = vec_new(0, nullptr);

  vec_push_back(&v, kOne);
  REQUIRE_FALSE(vec_is_empty(&v));
  REQUIRE(vec_capacity(&v) == 1); // Must increase by 1.

  vec_push_back(&v, kTwo);
  REQUIRE(vec_capacity(&v) == 2); // Must increase to be 2.

  vec_push_back(&v, kThree); // Triggers resize
  REQUIRE(vec_capacity(&v) == 4); // Must be double
  REQUIRE(vec_len(&v) == 3);
  vec_destroy(&v);
}

// --- Insert and Erase Operations ---
TEST_CASE("Insert Elements", "[insert-erase]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kFour);

  vec_insert(&v, 2, kThree);
  REQUIRE(v.length == 4);
  REQUIRE(vec_get(&v, 2) == kThree);
  REQUIRE(vec_get(&v, 3) == kFour);
  REQUIRE(v.capacity == 5); 
  vec_destroy(&v);
}

TEST_CASE("Insert Elements at end", "[insert-erase]") {
  Vec v = vec_new(2, nullptr);
  vec_insert(&v, 0, kOne);
  vec_insert(&v, 1, kTwo);
  vec_insert(&v, 2, kFour);

  vec_insert(&v, 2, kThree);
  REQUIRE(v.length == 4);
  REQUIRE(vec_get(&v, 2) == kThree);
  REQUIRE(vec_get(&v, 3) == kFour);
  REQUIRE(v.capacity >= 4); 
  vec_destroy(&v);
}

TEST_CASE("Erase Elements", "[insert-erase]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kThree);

  vec_erase(&v, 1); // Remove kTwo
  REQUIRE(v.length == 2);
  REQUIRE(vec_get(&v, 1) == kThree);
  REQUIRE(vec_get(&v, 0) == kOne);
  REQUIRE(v.capacity == 5);
  vec_destroy(&v);
}

TEST_CASE("Erase Elements w/Dtor", "[insert-erase]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(10, count_constants);
  
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kThree);
  REQUIRE(v.length == 3);
  REQUIRE(v.capacity == 10);
  REQUIRE(counter == 0);
  REQUIRE(invocations == 0);

  vec_erase(&v, 0);

  REQUIRE(v.length == 2);
  REQUIRE(v.capacity == 10);
  REQUIRE(counter == 1);
  REQUIRE(invocations == 1);

  vec_erase(&v, 0);

  REQUIRE(v.length == 1);
  REQUIRE(v.capacity == 10);
  REQUIRE(counter == 3);
  REQUIRE(invocations == 2);

  vec_erase(&v, 0);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 10);
  REQUIRE(counter == 6);
  REQUIRE(invocations == 3);

  vec_destroy(&v);
  REQUIRE(counter == 6);
  REQUIRE(invocations == 3);
}


// --- Clear Vector ---
TEST_CASE("Clear Vector", "[clear]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(5, count_constants);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  vec_clear(&v);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 5);
  REQUIRE(counter == 3); // Destructor called for kOne and kTwo
  REQUIRE(invocations == 2);
  vec_destroy(&v);
}

TEST_CASE("Clear Vector Multiple Times", "[clear]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(5, count_constants);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);

  vec_clear(&v);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 5);
  REQUIRE(counter == 3); 
  REQUIRE(invocations == 2);

  vec_clear(&v);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 5);
  REQUIRE(counter == 3);
  REQUIRE(invocations == 2);

  vec_clear(&v);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 5);
  REQUIRE(counter == 3);
  REQUIRE(invocations == 2);

  vec_destroy(&v);
}

// --- Destructor Tests ---
TEST_CASE("Destructor Invocation Count", "[count]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(5, count_constants);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kTwo);
  vec_push_back(&v, kThree);

  vec_clear(&v);
  REQUIRE(counter == 6); // Sum of 1 + 2 + 3 :)
  REQUIRE(invocations == 3);

  vec_push_back(&v, kFour);
  vec_destroy(&v);
  REQUIRE(counter == 10); // Adds 4 after destruction
  REQUIRE(invocations == 4);
}

//Harder Tests...

// --- Resizing ---
TEST_CASE("Clearing from 100 to 0", "[clear]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(100, count_constants);

  for (uintptr_t i = 0; i < 100; ++i) {
    vec_push_back(&v, kOne);
  }

  
  vec_clear(&v);
  REQUIRE(v.capacity == 100);
  REQUIRE(v.length == 0);
  REQUIRE(counter == 100); // Destructor called for kOne, 100 times.
  REQUIRE(invocations == 100); //self explanatory
  vec_destroy(&v);
}

// --- Insert and Erase Operations ---
TEST_CASE("Insert Operations with Shifting", "[insert-erase-difficult]") {
  Vec v = vec_new(5, nullptr);
  vec_push_back(&v, kOne);
  vec_push_back(&v, kThree);

  vec_insert(&v, 0, kTwo); // Insert in the beginning
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);
  vec_insert(&v, 0, kTwo);

  vec_insert(&v, 0, kSixetyEight); //i can't spell.

  REQUIRE(vec_get(&v, 0) == kSixetyEight);
  for(int i = 1; i <= 10; i++){
     REQUIRE(vec_get(&v, i) == kTwo);
  }

  REQUIRE(vec_get(&v, 11) == kOne);
  REQUIRE(vec_get(&v, 12) == kThree);

  vec_destroy(&v);
}

// -- inserting into an empty vector --
TEST_CASE("Inserting into an empty vector", "[insert-erase-difficult]") {
  Vec v = vec_new(1, nullptr);
  vec_insert(&v, 0, kTwo); // Insert in the beginning
  vec_insert(&v, 0, kFour); // Insert in the beginning

  REQUIRE(vec_len(&v) == 2);
  REQUIRE(vec_capacity(&v) == 2);
  REQUIRE(vec_get(&v, 0) == kFour);
  REQUIRE(vec_get(&v, 1) == kTwo);

  vec_destroy(&v);

  v = vec_new(0, nullptr);

  vec_insert(&v, 0, kTwo); // Insert in the beginning
  vec_insert(&v, 0, kFour); // Insert in the beginning

  REQUIRE(vec_len(&v) == 2);
  REQUIRE(vec_capacity(&v) == 2);
  REQUIRE(vec_get(&v, 0) == kFour);
  REQUIRE(vec_get(&v, 1) == kTwo);

  vec_destroy(&v);
}


// --- Clear Vector ---
TEST_CASE("Clear Large Vector", "[clear]") {
  counter = 0;
  invocations = 0;

  Vec v = vec_new(50, count_constants);
  for (uintptr_t i = 0; i < 10; ++i) {
    vec_push_back(&v, kOne);
    vec_push_back(&v, kTwo);
    vec_push_back(&v, kThree);
    vec_push_back(&v, kFour);
    vec_push_back(&v, kFive);
  }

  vec_clear(&v);
  REQUIRE(v.length == 0);
  REQUIRE(v.capacity == 50);
  REQUIRE(counter == 150); 
  REQUIRE(invocations == 50);

  counter = 0;
  invocations = 0;

  vec_destroy(&v);
  REQUIRE(counter == 0); 
  REQUIRE(invocations == 0);
}

// --- Stress Testing ---
TEST_CASE("Stress Test with Large Capacity and Operations", "[stress]") {
  Vec v = vec_new(1000, count_constants);

  for (uintptr_t i = 0; i < 1001; ++i) {
    vec_push_back(&v, kOne);
  }
  REQUIRE(v.length == 1001);
  REQUIRE(v.capacity == 2000);

  for (uintptr_t i = 0; i < 500; ++i) {
    vec_erase(&v, 0); // Erase from the front repeatedly
  }

  REQUIRE(v.length == 501);
  REQUIRE(v.capacity == 2000);

  vec_clear(&v);
  REQUIRE(v.length == 0);

  vec_destroy(&v);
  //done...
}
