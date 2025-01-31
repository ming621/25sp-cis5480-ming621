Name: Ming Gong
PennKey: minggong

submitted file:
 penn-shredder/Makefile
 penn-shredder/penn-shredder.c
 penn-shredder/penn-shredder.h
 penn-shredder/README.md
 penn-vec/Makefile
 penn-vec/Vec.c
 penn-vec/Vec.h
 penn-vec/catch.cpp
 penn-vec/catch.hpp
 penn-vec/main.c
 penn-vec/panic.c
 penn-vec/panic.h
 penn-vec/test_basic.cpp
 penn-vec/test_macro.cpp
 penn-vec/test_panic.cpp
 penn-vec/test_suite.cpp
 penn-vec/vector.h

Work accomplishement:
 everything excluding the extra credit
 (include Signal handling for SIGINT, SIGALRM, command execution, timeout feature, etc.)

Code Layout:
 penn-shredder.c: Main shell logic and function implementation
 penn-shredder.h: header file defining function prototypes
 MakeFile: Build config
 README.md: documentation

Design decision:
 It will first check argc is 2, and write the timeout variable.
 Then, in the while loop, it will write "penn-shredder# " into the shell, and then read the input command.
 Then, it will trim the whitespace from command, and then parse it into array.
 Finally, it will fork and let the child process to execute the command. The parent process will handle signal 
 and deliver signal (ctrl + C) to child process. 
