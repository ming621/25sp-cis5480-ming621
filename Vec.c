#include "./Vec.h"
#include <stdio.h>
#include <stdlib.h>
#include "./panic.h"

Vec vec_new(size_t initial_capacity, ptr_dtor_fn ele_dtor_fn) {
  // TODO: implement me
  Vec res;
  // check the initial capacity is non-negative
  if (initial_capacity < 0) {
    panic("initial capacity should be non-negative");
  }

  // recall malloc
  // simple eg. int* num = (int*)malloc(sizeof(int)); ... free(num);
  // malloc return NULL if allocation failed
  if (initial_capacity == 0) {
    res.data = NULL;
  } else {
    res.data = (ptr_t*)malloc(initial_capacity * sizeof(ptr_t));
    if (res.data == NULL) {
      panic("memory allocation for data failed");
    }
  }

  // recall that -> is equivalent to (*ptr).member
  res.length = 0;
  res.capacity = initial_capacity;
  res.ele_dtor_fn = ele_dtor_fn;
  return res;
}

// TODO: the rest of the vector functions
/* Gets the specified element of the Vec
 *
 * @param self a pointer to the vector who's element we want to get.
 * @param index the index of the element to get.
 * @returns the element at the specified index.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic()
 */
ptr_t vec_get(Vec* self, size_t index) {
  // check the self is NULL first
  if (self == NULL) {
    panic("self is NULL");
  }

  if (index >= self->length) {
    panic("index out of bound");
  }
  // remember that self->data is an array of pointer, so direct access
  return self->data[index];
}

/* Sets the specified element of the Vec to the specified value
 *
 * @param self    a pointer to the vector who's element we want to set.
 * @param index   the index of the element to set.
 * @param new_ele the value we want to set the element at that index to
 * @returns the element at the specified index.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic()
 */
void vec_set(Vec* self, size_t index, ptr_t new_ele) {
  if (self == NULL) {
    panic("self is NULL");
  }

  if (index >= self->length) {
    panic("index out of bound");
  }

  // remember to clean up the old one
  if (self->ele_dtor_fn != NULL && self->data[index] != NULL) {
    self->ele_dtor_fn(self->data[index]);
  }

  self->data[index] = new_ele;
}

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
void vec_push_back(Vec* self, ptr_t new_ele) {
  if (self == NULL) {
    panic("self is NULL");
  }
  if (self->capacity == 0) {
    self->capacity = 1;
    self->data = (ptr_t*)malloc(self->capacity * sizeof(ptr_t));
    if (self->data == NULL) {
      panic("realloc failed");
    }
  }

  if (self->length == self->capacity) {
    self->capacity *= 2;
    // resize use realloc!
    // recall: void* realloc(void* ptr, size_t new_size);
    // need to check NULL to see if it was succeed
    ptr_t* newdata =
        (ptr_t*)realloc(self->data, self->capacity * sizeof(ptr_t));
    if (newdata == NULL) {
      panic("realloc is failed");
    }
    self->data = newdata;
  }
  self->data[self->length] = new_ele;
  self->length++;
}

/* Removes and destroys the last element of the Vec
 *
 * @param self a pointer to the vector we are popping.
 * @returns true iff an element was removed.
 * @pre Assumes self points to a valid vector.
 * @post The capacity of self stays the same. The removed element is
 * destructed (cleaned up) as specified by the dtor_fn provided in vec_new.
 */
bool vec_pop_back(Vec* self) {
  if (self == NULL) {
    panic("self is NULL");
  }

  // need to check length first to ensure it's not empty
  if (self->length == 0) {
    return false;
  }

  if (self->ele_dtor_fn != NULL && self->data[self->length - 1] != NULL) {
    self->ele_dtor_fn(self->data[self->length - 1]);
  }
  // correct way
  self->length--;
  return true;
}

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
void vec_insert(Vec* self, size_t index, ptr_t new_ele) {
  if (self == NULL) {
    panic("self is NULL");
  }

  if (index < 0 || index > self->length) {
    panic("index out of bound");
  }

  if (self->capacity == 0) {
    self->capacity = 1;
    self->data = (ptr_t*)malloc(self->capacity * sizeof(ptr_t));
    if (self->data == NULL) {
      panic("malloc failed");
    }
  }

  if (self->capacity == self->length) {
    self->capacity *= 2;
    ptr_t* newdata =
        (ptr_t*)realloc(self->data, self->capacity * sizeof(ptr_t));
    if (newdata == NULL) {
      panic("realloc failed");
    }
    self->data = newdata;
  }
  // itearte backward to shift the data
  for (size_t i = self->length; i > index; i--) {
    self->data[i] = self->data[i - 1];
  }
  self->data[index] = new_ele;
  self->length++;
}

/* Erases an element at the specified valid location in the container
 *
 * @param self    a pointer to the vector we want to erase from.
 * @param index   the index of the element we want to erase at. Elements
 *                after this index are "shifted" down one position.
 * @pre Assumes self points to a valid vector. If the index is >= self->length
 * then this function will panic().
 */
void vec_erase(Vec* self, size_t index) {
  if (self == NULL) {
    panic("self is NULL");
  }
  if (index < 0 || index >= self->length) {
    panic("index out of bound");
  }
  // deconstruct the index element only
  if (self->ele_dtor_fn != NULL && self->data[index] != NULL) {
    self->ele_dtor_fn(self->data[index]);
  }
  // no need to deconstruct every element, since it's array of ptr, so just
  // shift
  for (size_t i = index; i < self->length - 1; i++) {
    self->data[i] = self->data[i + 1];
  }
  self->length--;
}

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
void vec_resize(Vec* self, size_t new_capacity) {
  if (self == NULL) {
    panic("self is NULL");
  }
  if (new_capacity > self->capacity) {
    ptr_t* newdata = (ptr_t*)realloc(self->data, new_capacity * sizeof(ptr_t));
    if (newdata == NULL) {
      panic("realloc failed");
    }
    self->data = newdata;
    self->capacity = new_capacity;
  }
}

/* Erases all elements from the container.
 * After this, the length of the vector is zero.
 * Capacity of the vector is unchanged.
 *
 * @param self a pointer to the vector we want to clear.
 * @pre Assumes self points to a valid vector.
 * @post The removed elements are destructed (cleaned up).
 */
void vec_clear(Vec* self) {
  if (self == NULL) {
    panic("self is NULL");
  }

  if (self->ele_dtor_fn != NULL) {
    for (size_t i = 0; i < self->length; i++) {
      if (self->data[i] != NULL) {
        self->ele_dtor_fn(self->data[i]);
      }
    }
  }

  self->length = 0;
}

/* Destruct the vector.
 * All elements are destructed and storage is deallocated.
 * Must set capacity and length to zero. Data is set to NULL.
 *
 * @param self a pointer to the vector we want to destruct.
 * @pre Assumes self points to a valid vector.
 * @post The removed elements are destructed (cleaned up)
 * and data storage deallocated.
 */
void vec_destroy(Vec* self) {
  if (self == NULL) {
    panic("self is NULL");
  }

  if (self->ele_dtor_fn != NULL) {
    for (size_t i = 0; i < self->length; i++) {
      if (self->data[i] != NULL) {
        self->ele_dtor_fn(self->data[i]);
      }
    }
  }
  free(self->data);
  self->data = NULL;
  self->length = 0;
  self->capacity = 0;
}
