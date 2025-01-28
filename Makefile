# declares which rules are "phony"
# In other words, they are more so commands than build targerts.
# For example "make clean" does not generate a file called `clean`,
#   instead it "cleans up" (deletes) any results of compilation so
#   we only have source files.
# Another example "make all" does not generate a file or executable
#   called "all", it just builds all executables
#   targets (except for extra credit in our case)
.PHONY = clean all tidy-check format

# List the source files
C_SOURCE_FILES = Vec.c main.c panic.c
H_SOURCE_FILES = Vec.h panic.h
TEST_FILES = test_vector.cpp

# list the source files for the macro vector extra credit
MACRO_SOURCE_FILES = vector.h
MACRO_TEST_FILES = test_macro_vector.cpp

# define the commands we will use for compilation and library building
CC = clang-15
CXX = clang++-15

# define useful flags to cc/ld/etc.
# use version gnu2x so that we can use statement expressions for macros
CFLAGS += -g3 -Wall -Werror -Wpedantic --std=gnu2x -gdwarf-4
CXXFLAGS += -g3 -Wall -Werror --std=gnu++2b -gdwarf-4

# makefile rules
all: test_suite main

main: main.c Vec.o panic.o
	$(CC) $(CFLAGS) -o $@ $^

test_suite: test_suite.o test_basic.o test_panic.o Vec.o catch.o panic.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test_suite.o: test_suite.cpp catch.hpp
	$(CXX) $(CXXFLAGS) -c $<

test_macro: test_suite.o test_macro.o catch.o panic.o
	$(CXX) $(CXXFLAGS) -Wno-gnu -o $@ $^

test_macro.o: test_macro.cpp vector.h catch.hpp
	$(CXX) $(CXXFLAGS) -Wno-gnu -c $<

test_basic.o: test_basic.cpp Vec.h catch.hpp
	$(CXX) $(CXXFLAGS) -c $<

test_panic.o: test_panic.cpp Vec.h catch.hpp
	$(CXX) $(CXXFLAGS) -c $<

Vec.o: Vec.c Vec.h
	$(CC) $(CFLAGS) -o $@ -c $<

panic.o: panic.c panic.h
	$(CC) $(CFLAGS) -o $@ -c $<

catch.o: catch.cpp catch.hpp
	$(CXX) $(CXXFLAGS) -c $<

# Phony Targets

# Explanantion of args:
#   -extra-arg=-std=c2x : specifies to check with C23
#   -warnings-as-error= : marks all warnings as errors
#   -checks= : specifies which checks to look for
#     cert-* turns on all errors related to CERT security checks.
#     readability-* turns on all errors related to making code more readable
#     bugprone-* turns on errors that will help identify frequent beginner bugs
tidy-check: 
	clang-tidy-15 \
        --extra-arg=--std=gnu2x \
        --extra-arg=-D_POSIX_C_SOURCE \
	-p . \
	$(C_SOURCE_FILES) $(H_SOURCE_FILES) $(MACRO_SOURCE_FILES)

format:
	clang-format-15 -i --verbose --style=Chromium $(C_SOURCE_FILES) $(H_SOURCE_FILES) $(MACRO_SOURCE_FILES)

clean:
	rm *.o test_suite main test_macro 

