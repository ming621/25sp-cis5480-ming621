#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "catch.hpp"

extern "C" {
  #include "./vector.h"
}

using namespace std;


static uintptr_t kOne = 1;
static uintptr_t kTwo = 2;
static uintptr_t kThree = 3;
static uintptr_t kFour = 4;

void noop([[maybe_unused]] void* input) {}

static uintptr_t counter = 0;
static int invocations = 0;

static void count_constants(void* input) {
    counter += *reinterpret_cast<uintptr_t*>(input);
    invocations += 1;
}

static void string_destructor(void* str) {
    free(*(char**)str);
}

typedef struct __Point{
    int x;
    int y;
} _____Point;

static void free_string_ele(char** ptr_to_ele) {
    free(*ptr_to_ele);
}

// --- Basic Operations ---
TEST_CASE("Simple Constructor len capacity free", "[basic macro]") {
    vector(int) v = vector_new(int, 10, nullptr);
    REQUIRE(v != nullptr);
    REQUIRE(vector_capacity(&v) == 10);
    REQUIRE(vector_len(&v) == 0);
    
    vector_info* vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == nullptr);
    REQUIRE(vinfo->capacity == 10);
    REQUIRE(vinfo->len == 0);

    vector_free(&v);
}

TEST_CASE("Construction and Destruction", "[basic macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 3, NULL);
    REQUIRE(vector_capacity(&vec) == 3);
    REQUIRE(vector_len(&vec) == 0);
    vector_free(&vec);

    vector(uintptr_t) vec_custom = vector_new(uintptr_t, 5, count_constants);
    REQUIRE(vector_capacity(&vec_custom) == 5);
    REQUIRE(vector_len(&vec_custom) == 0);
    vector_free(&vec_custom);
}

TEST_CASE("Check Empty Vector", "[basic macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    REQUIRE(vector_len(&vec) == 0);

    vector_push(&vec, kOne);
    REQUIRE(vector_len(&vec) == 1);

    vector_pop(&vec);
    REQUIRE(vector_len(&vec) == 0);
    vector_free(&vec);
}

// --- Push Operations ---
TEST_CASE("Simple string push", "[push macro]") {
    vector(char*) v = vector_new(char*, 10, (destroy_fn)free_string_ele);

    vector_push(&v, strdup("hello"));
    vector_push(&v, strdup("howdy"));

    vector_info* vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == (destroy_fn)free_string_ele);
    REQUIRE(vinfo->capacity == 10);
    REQUIRE(vinfo->len == 2);

    REQUIRE(strcmp(v[0], "hello") == 0);
    REQUIRE(strcmp(v[1], "howdy") == 0);

    vector_free(&v);
}

TEST_CASE("Simple string push w/ resize", "[push macro]") {
    vector(char*) v = vector_new(char*, 1, (destroy_fn)free_string_ele);
    vector_info* vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == (destroy_fn)free_string_ele);
    REQUIRE(vinfo->capacity == 1);
    REQUIRE(vinfo->len == 0);

    vector_push(&v, strdup("hello"));
    vector_push(&v, strdup("howdy"));

    vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == (destroy_fn)free_string_ele);
    REQUIRE(vinfo->capacity == 2);
    REQUIRE(vinfo->len == 2);

    REQUIRE(strcmp(v[0], "hello") == 0);
    REQUIRE(strcmp(v[1], "howdy") == 0);

    vector_free(&v);
}

TEST_CASE("Simple string push w/ Double Resize", "[push macro]") {
    vector(char*) v = vector_new(char*, 1, (destroy_fn)free_string_ele);
    vector_info* vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == (destroy_fn)free_string_ele);
    REQUIRE(vinfo->capacity == 1);
    REQUIRE(vinfo->len == 0);

    vector_push(&v, strdup("hello"));
    vector_push(&v, strdup("howdy"));

    vinfo = get_vector_header(&v);
    REQUIRE(vinfo->ele_dtor == (destroy_fn)free_string_ele);
    REQUIRE(vinfo->capacity == 2);
    REQUIRE(vinfo->len == 2);

    REQUIRE(strcmp(v[0], "hello") == 0);
    REQUIRE(strcmp(v[1], "howdy") == 0);


    vector_push(&v, strdup("wassup"));
    vector_push(&v, strdup("awesome"));

    vinfo = get_vector_header(&v);

    REQUIRE(vinfo->capacity == 4);
    REQUIRE(vinfo->len == 4);

    vector_free(&v);
}


TEST_CASE("Push Operations", "[push-pop macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    REQUIRE(vector_len(&vec) == 2);
    REQUIRE(vec[0] == kOne);
    REQUIRE(vec[1] == kTwo);
    vector_free(&vec);
}

TEST_CASE("Pop Operations", "[push-pop macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, count_constants);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    REQUIRE(vector_len(&vec) == 2);

    REQUIRE(vector_pop(&vec));
    REQUIRE(vector_len(&vec) == 1);

    REQUIRE(vector_pop(&vec));
    REQUIRE(vector_len(&vec) == 0);

    REQUIRE_FALSE(vector_pop(&vec));
    vector_free(&vec);
}

// --- Get and Set Operations ---
TEST_CASE("Get Elements", "[get-set macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    REQUIRE(vector_get(&vec, 0) == kOne);
    REQUIRE(vector_get(&vec, 1) == kTwo);
    vector_free(&vec);
}

TEST_CASE("Set Elements", "[get-set macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    vector_set(&vec, 1, kThree);
    REQUIRE(vector_get(&vec, 1) == kThree);
    vector_free(&vec);
}

TEST_CASE("Set Elements w/Dtor", "[get-set macro]") {
    counter = 0;
    invocations = 0;

    vector(uintptr_t) vec = vector_new(uintptr_t, 5, count_constants);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    vector_set(&vec, 1, kThree);
    REQUIRE(vector_get(&vec, 1) == kThree);
    REQUIRE(counter == 2);
    REQUIRE(invocations == 1);

    vector_set(&vec, 1, kFour);
    REQUIRE(vector_get(&vec, 1) == kFour);
    REQUIRE(counter == 5);
    REQUIRE(invocations == 2);

    vector_set(&vec, 0, kFour);
    REQUIRE(vector_get(&vec, 0) == kFour);
    REQUIRE(counter == 6);
    REQUIRE(invocations == 3);

    vector_free(&vec);
}

// --- Resizing ---
TEST_CASE("Manual Resize", "[resize macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 3, NULL);
    REQUIRE(vector_capacity(&vec) == 3);
    REQUIRE(vector_len(&vec) == 0);

    vector_resize(&vec, 6);
    REQUIRE(vector_capacity(&vec) == 6);
    REQUIRE(vector_len(&vec) == 0);
    vector_free(&vec);
}

//also tested above. Better safe than sorry.
TEST_CASE("Automatic Resize on Push", "[resize macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 2, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);
    vector_push(&vec, kThree); // Triggers resize
    REQUIRE(vector_capacity(&vec) == 4); // Must be double
    REQUIRE(vector_len(&vec) == 3);
    vector_free(&vec);
}

TEST_CASE("Automatic Resize on Push with 0 Capacity Vector", "[resize macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 0, NULL);

    vector_push(&vec, kOne);
    REQUIRE(vector_capacity(&vec) == 1);

    vector_push(&vec, kTwo);
    REQUIRE(vector_capacity(&vec) == 2);

    vector_push(&vec, kThree);
    REQUIRE(vector_capacity(&vec) == 4);
    REQUIRE(vector_len(&vec) == 3);
    vector_free(&vec);
}

// --- Insert and Erase Operations ---
TEST_CASE("Insert Elements", "[insert-erase macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);
    vector_push(&vec, kFour);

    vector_insert(&vec, 2, kThree);
    REQUIRE(vector_len(&vec) == 4);
    REQUIRE(vector_get(&vec, 2) == kThree);
    REQUIRE(vector_get(&vec, 3) == kFour);
    vector_free(&vec);
}

TEST_CASE("Erase Elements", "[insert-erase macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 5, NULL);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);
    vector_push(&vec, kThree);

    vector_erase(&vec, 1); // Remove kTwo
    REQUIRE(vector_len(&vec) == 2);
    REQUIRE(vector_get(&vec, 1) == kThree);
    REQUIRE(vector_get(&vec, 0) == kOne);
    vector_free(&vec);
}

TEST_CASE("Erase Elements w/Dtor", "[insert-erase macro]") {
    counter = 0;
    invocations = 0;

    vector(uintptr_t) vec = vector_new(uintptr_t, 10, count_constants);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);
    vector_push(&vec, kThree);

    vector_erase(&vec, 0);
    REQUIRE(counter == 1);
    REQUIRE(invocations == 1);

    vector_erase(&vec, 0);
    REQUIRE(counter == 3);
    REQUIRE(invocations == 2);

    vector_erase(&vec, 0);
    REQUIRE(counter == 6);
    REQUIRE(invocations == 3);

    vector_free(&vec);
}

// --- Vector of Structs ---

TEST_CASE("Basic struct Handling", "[type-struct macro]") {
    vector(_____Point) vec = vector_new(_____Point, 2, NULL);
    vector_info* vinfo = get_vector_header(&vec);
    REQUIRE(vinfo->ele_dtor == nullptr);
    REQUIRE(vinfo->capacity == 2);
    REQUIRE(vinfo->len == 0);

    vector_free(&vec);
}


TEST_CASE("Vector of Pointers to Structs", "[type-struct macro]") {
    vector(_____Point *) vec = vector_new(_____Point *, 2, NULL);

    _____Point p1 = {1, 2};

    _____Point p2 = {3, 4};

    vector_push(&(vec), &p1);
    vector_push(&(vec), &p2);
    
    REQUIRE(vector_len(&vec) == 2);
    REQUIRE(vec[0]->x == 1);
    REQUIRE(vec[0]->y == 2);
    REQUIRE(vec[1]->x == 3);
    REQUIRE(vec[1]->y == 4);

    vector_free(&vec);
}

TEST_CASE("Vector of Structs", "[type-struct macro]") {
    vector(_____Point) vec = vector_new(_____Point, 2, NULL);
    _____Point p1 = {1, 2};
    _____Point p2 = {3, 4};
    vector_push(&vec, p1);
    vector_push(&vec, p2);
    REQUIRE(vector_len(&vec) == 2);
    REQUIRE(vec[0].x == 1);
    REQUIRE(vec[0].y == 2);
    REQUIRE(vec[1].x == 3);
    REQUIRE(vec[1].y == 4);
    vector_free(&vec);
}

TEST_CASE("Vector of strings with destructor", "[type-string macro]") {
    vector(char*) vec = vector_new(char*, 2, string_destructor);

    char* s1 = strdup("Hello");
    char* s2 = strdup("World");

    vector_push(&vec, s1);
    vector_push(&vec, s2);

    REQUIRE(vector_len(&vec) == 2);
    REQUIRE(strcmp(vec[0], "Hello") == 0);
    REQUIRE(strcmp(vec[1], "World") == 0);

    vector_free(&vec);
}

// --- Clear Vector ---
TEST_CASE("Clear Vector", "[clear macro]") {
    counter = 0;
    invocations = 0;

    vector(uintptr_t) vec = vector_new(uintptr_t, 5, count_constants);
    vector_push(&vec, kOne);
    vector_push(&vec, kTwo);

    vector_free(&vec);
    REQUIRE(counter == 3); // Destructor called
    REQUIRE(invocations == 2);
}

// --- Stress Test ---
TEST_CASE("Stress Test with Large Capacity and Operations", "[stress macro]") {
    vector(uintptr_t) vec = vector_new(uintptr_t, 1000, count_constants);

    for (uintptr_t i = 0; i < 1001; ++i) {
        vector_push(&vec, kOne);
    }
    REQUIRE(vector_len(&vec) == 1001);
    REQUIRE(vector_capacity(&vec) == 2000);

    for (uintptr_t i = 0; i < 500; ++i) {
        vector_erase(&vec, 0);
    }
    REQUIRE(vector_len(&vec) == 501);

    vector_free(&vec);
}
