#ifndef PANIC_H_
#define PANIC_H_

#ifdef DISABLE_PANIC

// If panics are disabled, do nothing
#define panic(error_message) ((void)0)

#else

// if panices are enabled (whith they are by default) then
// call the function that handles panicing
#define panic(error_message) print_and_abort(error_message)

#endif

[[noreturn]] void print_and_abort(const char* error_message);

#endif  // PANIC_H_
