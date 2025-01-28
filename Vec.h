#ifndef VEC_H_
#define VEC_H_

#include <stdbool.h>
#include <stddef.h>  // for size_t

typedef void* ptr_t;
typedef void (*ptr_dtor_fn)(ptr_t);

typedef struct vec_st {
  ptr_t* data;
  size_t length;
  size_t capacity;
  ptr_dtor_fn ele_dtor_fn;
} Vec;

/*!
 * Creates a new empty Vec(tor) with the specified initial_capacity
 * and specified function to clean up elements in the vector.
 *
 * @param initial_capacity the initial capacity of the newly created vector, non
 * negative
 * @param ele_dtor_fn      a function pointer to a function that takes in a
 *                         ptr_t (a vector element) and cleans it up. This
 *                         is commonly just `free` but custom functions can be
 *                         passed in. NULL can also be passed in to specify
 *                         that there is no cleanup function that needs to
 *                         be called on each element.
 * @returns a newly created vector with specified capacity, 0 length and
 * the specified element destructor (cleanup) function.
 * @post if memory allocation fails, the function will panic.
 */
Vec vec_new(size_t initial_capacity, ptr_dtor_fn ele_dtor_fn);

/* Returns the current capacity of the Vec
 * Written as a function-like macro
 *
 * @param vec, a pointer to the vector we want to grab the capacity of.
 */
// TODO: finish this macro
#define vec_capacity(vec) ((vec)->capacity)

/* Returns the current length of the Vec
 * written as a function-like macro
 *
 * @param vec, a pointer to the vector we want to grab the len of.
 */
// TODO: finish this macro
#define vec_len(vec) ((vec)->length)

/* Checks if the Vec is empty
 * written as a function-like macro
 *
 * @param vec, a pointer to the vector we want to check emptiness of.
 */
// TODO: finish this macro: check the length is 0 or not.
// recall length is the current number of elements in the vector
#define vec_is_empty(vec) ((vec)->length == 0)

/* Gets the specified element of the Vec
 *
 * @param self a pointer to the vector who's element we want to get.
 * @param index the index of the element to get.
 * @returns the element at the specified index.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic()
 */
ptr_t vec_get(Vec* self, size_t index);

/* Sets the specified element of the Vec to the specified value
 *
 * @param self    a pointer to the vector who's element we want to set.
 * @param index   the index of the element to set.
 * @param new_ele the value we want to set the element at that index to
 * @returns the element at the specified index.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic()
 */
void vec_set(Vec* self, size_t index, ptr_t new_ele);

/* Appends the given element to the end of the Vec
 *
 * @param self      a pointer to the vector we are pushing onto
 * @param new_ele   the value we want to add to the end of the container
 * @pre Assumes self points to a valid vector.
 * @post If a resize is needed and it fails, then this function will panic()
 * @post If after the operation the new length is greater than the old capacity
 * then a reallocation takes place and all elements are copied over.
 * Capacity is doubled. If initial capacity is zero, it is resized to
 * capacity 1. Any pointers to elements prior to this reallocation are
 * invalidated.
 */
void vec_push_back(Vec* self, ptr_t new_ele);

/* Removes and destroys the last element of the Vec
 *
 * @param self a pointer to the vector we are popping.
 * @returns true iff an element was removed.
 * @pre Assumes self points to a valid vector.
 * @post The capacity of self stays the same. The removed element is
 * destructed (cleaned up) as specified by the dtor_fn provided in vec_new.
 */
bool vec_pop_back(Vec* self);

/* Inserts an element at the specified location in the container
 *
 * @param self    a pointer to the vector we want to insert into.
 * @param index   the index of the element we want to insert at.
 *                Elements at this index and after it are "shifted" up
 *                one position. If index is equal to the length, then we insert
 *                at the end of the vector.
 * @param new_ele the value we want to insert
 * @pre Assumes self points to a valid vector. If the index is > self->length
 * then this function will panic().
 * @post If after the operation the new length is greater than the old capacity
 * then a reallocation takes place and all elements are copied over. Capacity is
 * doubled. Any pointers to elements prior to this reallocation are invalidated.
 */
void vec_insert(Vec* self, size_t index, ptr_t new_ele);

/* Erases an element at the specified valid location in the container
 *
 * @param self    a pointer to the vector we want to erase from.
 * @param index   the index of the element we want to erase at. Elements
 *                after this index are "shifted" down one position.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic().
 */
void vec_erase(Vec* self, size_t index);

/* Resizes the container to a new specified capacity.
 * Does nothing if new_capacity <= self->length
 *
 * @param self         a pointer to the vector we want to resize.
 * @param new_capacity the new capacity of the vector.
 * @pre Assumes self points to a valid vector.
 * @post If a resize takes place, then a reallocation takes place and all
 * elements are copied over. Any pointers to elements prior to this
 * reallocation are invalidated.
 * @post The removed elements are destructed (cleaned up).
 */
void vec_resize(Vec* self, size_t new_capacity);

/* Erases all elements from the container.
 * After this, the length of the vector is zero.
 * Capacity of the vector is unchanged.
 *
 * @param self a pointer to the vector we want to clear.
 * @pre Assumes self points to a valid vector.
 * @post The removed elements are destructed (cleaned up).
 */
void vec_clear(Vec* self);

/* Destruct the vector.
 * All elements are destructed and storage is deallocated.
 * Must set capacity and length to zero. Data is set to NULL.
 *
 * @param self a pointer to the vector we want to destruct.
 * @pre Assumes self points to a valid vector.
 * @post The removed elements are destructed (cleaned up)
 * and data storage deallocated.
 */
void vec_destroy(Vec* self);

#endif  // VEC_H_
