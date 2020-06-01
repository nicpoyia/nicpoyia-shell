/*  @file nicpoyiash_interpreter.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Shell interpreter header.
 *  Used by:
 *  	- The terminal
 *  	- Interpreter-mode execution of the shell
 */

#ifndef NICPOYIASH_INTERPRETER_H_
#define NICPOYIASH_INTERPRETER_H_

#include "files.h"
#include "commands.h"
#include "processes.h"
#include "jobs.h"

#define MAX_ARGS_SIZE 128

/**
 * @brief Function that executes an entire script, given in a single string
 *
 * @param script The full script string as given
 * @return The number of forked processes: OK / -1: Error occurred
 */
int executeScript(char *script);

/**
 * @brief Function that executes a script given its full command arguments vector.
 *
 * @param args Number of command arguments
 * @param argv Command arguments vector / array
 * @return The number of forked processes: OK / -1: Error occurred
 */
int executeScriptUsingArguments(int args, char *argv[]);

#endif /* NICPOYIASH_INTERPRETER_H_ */
