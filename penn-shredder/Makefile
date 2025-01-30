######################################################################
#
#                       Author: Hannah Pan
#                       Date:   01/13/2021
#
# The autograder will run the following command to build the program:
#
#       make -B
#
# To build a version that does not call kill(2), it will run:
#
#       make -B CPPFLAGS=-DEC_NOKILL
#
######################################################################

# name of the program to build
#
PROG=penn-shredder

PROMPT='"$(PROG)\# "'
CATCHPHRASE='"Bwahaha ... Tonight, I dine on turtle soup!\n"'

# Remove -DNDEBUG during development if assert(3) is used
#
override CPPFLAGS += -DNDEBUG -DPROMPT=$(PROMPT) -DCATCHPHRASE=$(CATCHPHRASE)

CC = clang-15
CXX = clang++-15

# Replace -O1 with -g for a debug version during development
#
CFLAGS += -g3 -gdwarf-4 -Wall -Werror -Wno-gnu -I. -I.. --std=gnu2x
CXXFLAGS += -g3 -gdwarf-4 -Wall -Werror -Wpedantic -Wno-gnu -I. -I.. --std=gnu++2b

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
HEADERS = $(wildcard *.h)

.PHONY : all clean tidy-check format

all: $(PROG)

$(PROG) : $(OBJS) $(HEADERS)
	$(CC) -o $@ $(OBJS)

%.o: %.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean :
	$(RM) $(OBJS) $(PROG)

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
        -warnings-as-errors=* \
        -header-filter=.* \
        $(SRCS) $(HEADERS)

format:
	clang-format-15 -i --verbose --style=Chromium $(SRCS) $(HEADERS)

