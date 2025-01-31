#ifndef PENN_SHREDDER_H_
#define PENN_SHREDDER_H_

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

/*!
 * Handles the SIGALRM signal
 * This function sets the `alarm_flag` and terminates the currently 
 * running child process if timeout.
 *
 * @param signo     The signal number (SIGALRM).
 */
void handle_sigalrm(int signo);

/*!
 * Handles the SIGINT signal (Ctrl+C).
 * If a child process is running, parent will deliver the signal to child.
 * Parent still running.
 * If no child process, it return a new penn-shredder# line.
 *
 * @param signo     The signal number (SIGINT).
 */
void handle_sigint(int signo);

/*!
 * Trims leading and trailing whitespace from a given buffer.
 *
 * @param buffer    The input string.
 * @returns The length of the trimmed string.
 */
static size_t trim(char* buffer);

/*!
 * Parses a command string into an argument array.
 *
 * @param cmd   command string
 * @param argc  Pointer to an int that stores count of argument.
 * @returns A dynamically allocated array of argument strings
 * @post return nULL if not valid parameter
 */
static char** parse(char* cmd, int* argc);

/*!
 * Reads a command line from standard input.
 *
 * @param cmdBuffer    input buffer that stores command
 * @param buffer_size  The maximum size of the buffer
 * @returns true if a valid command was read, false if an error occurred or EOF was reached.
 */
bool readCommandLine(char* cmdBuffer, size_t buffer_size);

/*!
 * Sets up signal handlers for SIGALRM and SIGINT in the parent process.
 */
void setupParentSignals(void);

/*!
 * Parses and executes a given command.
 * This function forks a child process, executes the command,
 * and waits for the child process to complete.
 *
 * @param cmd   The command string to execute for child process.
 * @param envp  The environment variables
 */
void runCommand(char* cmd, char* envp[]);


#endif