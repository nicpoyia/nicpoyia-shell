/*  @file bash_builtin_functions.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Bash built-in functions implementation
 */

#ifndef BASH_BUILTIN_FUNCTIONS_H_
#define BASH_BUILTIN_FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

#define MAX_COMMAND_LENGTH 512
#define MAX_DIR_LENGTH 1024
#define MAX_INPUT_SIZE 1024

/**
 * @brief Function that defines whether the shell should terminate now.
 *
 * @return Whether the terminal should immediately exit
 */
int exitNow();

/**
 * @brief Function that defines if the terminal should wait for the user input, to be used for a command.
 *
 * @return Whether the terminal should wait for input
 */
int inputWaiting();

/**
 * @brief Function to notify the terminal to read a value for the read operation in the next prompt.
 *
 * @return Whether the terminal should read from the user
 */
int readFromUser();

/**
 * @brief Function that continues the execution of a command, that lacked some input (terminal has blocked).
 *
 * @param inputScript The next script entered afte the blocking one
 * @return Whether the command entered is a bash built-in command
 */
int continueBashExecution(char *inputScript);

/**
 * @brief Function that checks whether a command represents a bash built-in function.
 * If it is a bash built-in function, then it is executed.
 *
 * @param commandName The pure command name
 * @param commandArguments commandArguments Arguments array
 * @param args Number of arguments passed
 * @return Error code:
 * 		 0: If the command is not a bash built-in function
 * 		 1: If the command is a bash built-in function and it has been executed.
 * 		-1: If an error occurred.
 */
int executeBashBuiltinFunction(char *commandName, char **commandArguments,
		int args);

/**
 * @brief Function that gets obtains a specific substring and returns a pointer to it.
 *
 * @param str The original string
 * @param begin The index within the string to begin getting characters from
 * @param len Length of substring
 * @return The substring produced
 */
char* subString(const char* str, size_t begin, size_t len);

#endif /* BASH_BUILTIN_FUNCTIONS_H_ */
