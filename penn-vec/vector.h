#ifndef VECTOR_H_
#define VECTOR_H_

/*!
 * The idea is that we have T* which is a pointer to the beginning of
 * heap allocated array (or the pointer is NULL). A null pointer is treated as a
 * 0 length, 0 capacity vector.
 *
 * To store the metadata (capacity, length, element destructor function), we
 * have a vector_info_st struct that is implictly stored before the beginning of
 * the elements. If we did something like:
 *
 * vector(int) vec = vector_new(int, 3, NULL);
 * vector_push(&vec, 1);
 *
 * We would get following stored in memory:
 *
 *             +---+
 *         vec | | |
 *             +-+-+
 *               |
 *               |
 *               V    // pointer to heap
 * +------------+---+---+---+
 * | cap = 3    |   |   |   |
 * | len = 1    | 1 | ? | ? |
 * | dtor= NULL |   |   |   |
 * +------------+---+---+---+
 *
 *
 * This means we can index directly into the vector using square brackets, [].
 * Other than square brackets, almost all functions take in a pointer to a
 * vector so that the vector can be modified
 *
 * To make sure that the ele_dtor can work on any type T, the destructor
 * function takes a pointer to an element in the vector.
 *
 * int main() {
 *   // declares an empty vector with 0 cap and no dtor function
 *   vector(int) vec = NULL;
 *
 *   // could also do this to specify an initial capacity
 *   // and a destroy function (NULL if no destroy function)
 *   // vector(int) vec = vector_new(int, 10, NULL);
 *
 *   vector_push(&vec, 3);
 *   vector_push(&vec, 5);
 *
 *   for (size_t i = 0; i < vector_len(&vec); i++) {
 *      printf("%d\n", vec[i]);
 *   }
 *
 *   vector_free(&vec);
 * }
 *
 */

#include <stdbool.h>
#include <stdlib.h>  // malloc, realloc, free
#include "./panic.h"

// the destroy function takes in a pointer
// to an element of the vector.
typedef void (*destroy_fn)(void*);

typedef struct vector_info_st {
  size_t len;
  size_t capacity;
  destroy_fn ele_dtor;
} vector_info;

#define vector(T) T*

// Synopsis:
//  vector_info* get_vector_header(vector(T)* vec);
//
// args:
//  Given a pointer to a vector, returns a pointer to
//  that vector's metadata which contain the stored
//  capacity, length and element destructor.
//
// Given to you:
#define get_vector_header(vec)                                      \
  ({                                                                \
    typeof(vec) __impl_gvh_vec = (vec);                             \
    if (__impl_gvh_vec == NULL) {                                   \
      panic("NULL arg to get_vector_header\n");                     \
    }                                                               \
    *__impl_gvh_vec ? ((vector_info*)(*__impl_gvh_vec)) - 1 : NULL; \
  })

// Synopsis:
//   vector(T) vector_new(T, size_t initial_capacity, destroy_fn
//   element_destroy_fn);
//
// Description:
//
// This is a "factory" function that allocates a new empty vector
// of the type specified by _T_, the capcaity specified by _initial_capacity_,
// the destructor function to destroy elements in the vector.
// The destructor is run on specific elements in the vector when the element is
// removed or the non-empty vector is de-allocated.
//
// args:
// - T: this is not a real parameter, rather, it specifies the type of the
//      vector being created.
// - init_capacity: the initial capacity of the vector
// - dtor: the element destroy fn
//
// returns:
// - a newly allocated vector
//
// example:
// vector(int) v = vector_new(int, 10, NULL);
#define vector_new(T, init_capacity, dtor) \
  ({                                       \
    /* TODO */                             \
    NULL;                                  \
  })

// Synopsis:
//   size_t vector_len(vector(T)* self);
//
// Description:
//
// Returns the length of the specified vector
//
// args:
// - self: a pointer to the vector we want to get the length of
//
// returns:
// - the length of the vector
//
// example:
// vector(int) v = ...;
// size_t len = vector_len(&v);
#define vector_len(self) \
  ({                     \
    /* TODO */           \
    0U;                  \
  })

// Synopsis:
//   size_t vector_capacity(vector(T)* self);
//
// Description:
//
// Returns the capacity of the specified vector
//
// args:
// - self: a pointer to the vector we want to get the length of
//
// returns:
// - the capacity of the vector
//
// example:
// vector(int) v = ...;
// size_t len = vector_capacity(&v);
#define vector_capacity(self) \
  ({                          \
    /* TODO */                \
    0U;                       \
  })

// Synopsis:
//   size_t vector_element_size(vector(T)* self);
//
// Description:
//
// Returns the size of elements in the specified vector
// effectively the same as sizeof(T) when given a pointer to a vector(T).
//
// args:
// - self: a pointer to the vector we want to examine
//
// returns:
// - size of the element of a vector.
//
// example:
// vector(int) v = ...;
// size_t ele_size = vector_element_size(&v); // same as sizeof(int)
#define vector_element_size(self) (4 /* TODO */)

// Synopsis:
//   void vector_resize(vector(T)* self, size_t new_capacity);
//
// Description:
//
// resizes the capacity of the vector so that it can hold up to
// `new_capacity` elements in the vector before a resize would be
// needed to hold more.
//
// If `new_capacity` is <= the current capacity, nothing is done.
//
// args:
// - self: a pointer to the vector we want to resize
// - n: the new capacity we want the vector to be
//
// example:
// vector(int) v = ...;
// vector_resize(&v, vector_capacity(&v) * 2);
#define vector_resize(self, n)                   \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   T vector_get(vector(T)* self, size_t index);
//
// Description:
//
// returns the element at the specified index of the vector
// panics if the index is out of range.
//
// args:
// - self: a pointer to the vector we want to get an element from
// - index: the index of the element we want to get
//
// example:
// vector(int) v = ...;
// vector_get(&v, 0); // Same thing as doing: v[0]; but with bounds checking
#define vector_get(self, index) \
  ({                            \
      /* TODO */                \
  })

// Synopsis:
//   void vector_set(vector(T)* self, size_t index, T new_element);
//
// Description:
//
// sets the element at the specified index of the vector to `new_element`
// panics if the index is out of range.
//
// args:
// - self: a pointer to the vector we want to modify an element of
// - index: the index of the element we want to set
// - new_element: the new value we want to store in the vector
//
// example:
// vector(int) v = ...;
// vector_set(&v, 0, 3);
//
// // Same thing as doing: v[0] = 3; but with bounds checking
#define vector_set(self, index, ...)             \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   void vector_push(vector(T)* self, T new_element);
//
// Description:
// Appends the given element to the end of the vector
// If a resize is needed and it fails, then this function will panic()
// If after the operation the new length is greater than the old capacity
// then a reallocation takes place and all elements are copied over. Any
// pointers to elements prior to this reallocation are invalid..
//
// args:
// - self: a pointer to the vector we want to push onto
// - new_element: the new value we want to store in the vector
//
// example:
// vector(int) v = ...;
// vector_push(&v, 3);
#define vector_push(self, ...)                   \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   bool vector_pop(vector(T)* self);
//
// Description:
// removes and destroys the last element of the vector
//
// args:
// - self: a pointer to the vector we want to pop.
//
// example:
// vector(int) v = ...;
// bool success = vector_pop(&v);
#define vector_pop(self)                         \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   void vector_insert(vector(T)* self, size_t index, T new_element);
//
// Description:
// Inserts the given element to the specified location in the vector.
// panic()'s if the index is > the length of the vector.
// If a resize is needed and it fails, then this function will panic()
// If after the operation the new length is greater than the old capacity
// then a reallocation takes place and all elements are copied over. Any
// pointers to elements prior to this reallocation are invalid.
//
// args:
// - self: a pointer to the vector we want to insert into
// - index: the index of the element we want to insert at.
//          Elements at this index and after it are "shifted" up
//          one position. If index is equal to the length, then we insert
//          at the end of the vector.
// - new_element: the new value we want to store in the vector
//
// example:
// vector(int) v = ...;
// /* if v = {3, 2, 4}; */
// vector_insert(&v, 1, 6);
// /* after: v = {3, 6, 2, 4}; */
#define vector_insert(vec, index, ...)           \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   void vector_erase(vector(T)* self, size_t index);
//
// Description:
// Erases an element at the specified location in the vector.
// the erased element is destructored (cleaned up) with the ele_dtor function.
// panic()'s if the index is >= the length of the vector.
//
// args:
// - self: a pointer to the vector we want to modify
// - index: the index of the element we want to erase at.
//          Elements after this index are "shifted" down one index.
//
// example:
// vector(int) v = ...;
// /* if v = {3, 2, 4}; */
// vector_erase(&v, 2);
// /* after: v = {3, 2}; */
#define vector_erase(vec, index)                 \
  ({                                             \
    /* TODO */                                   \
    ((void)0); /* (optional) return "nothing" */ \
  })

// Synopsis:
//   void vector_free(vector(T)* self);
//
// Description:
// free (Destruct) the vector.
// All elements are destructed and storage is deallocated.
//
// args:
// - self: a pointer to the vector we want to free
//
// example:
// vector(int) v = ...;
// vector_free(&v);
#define vector_free(self) \
  ({                      \
      /* TODO */          \
  })

#endif  // VECTOR_H_
