/*  @file nicpoyiash_interpreter.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Shell interpreter implementation.
 *  Used by:
 *  	- The terminal
 *  	- Interpreter-mode execution of the shell
 */

#include "nicpoyiash_interpreter.h"

/**
 * @brief Function that executes an entire script, given in a single string
 *
 * @param script The full script string as given
 * @return The number of forked processes: OK / -1: Error occurred
 */
int executeScript(char *script) {
	char *scriptCopy = (char*) malloc((strlen(script) + 1) * sizeof(char));
	if (scriptCopy == NULL ) {
		perror("malloc error");
		return -1;
	}
	strcpy(scriptCopy, script);
	// Count the jobs given
	char *words[MAX_ARGS_SIZE];
	int wordsCount = splitJobs(scriptCopy, NULL, 1, 0, words);
	if (wordsCount == -1)
		return -1;
	// Allocate space from the system
	char **splittedJobs = (char**) malloc(wordsCount * sizeof(char*));
	if (splittedJobs == NULL ) {
		perror("malloc error");
		return -1;
	}
	strcpy(scriptCopy, script);
	// Split and execute the individual jobs
	int jobsCount = splitJobs(scriptCopy, &splittedJobs, 0, wordsCount, words);
	free(script);
	// Script copy is not freed, since is is maintained in a
	// divided form as the splittedJobs array
	int forkedProcesses = 0;
	int i;
	for (i = 0; i < jobsCount; i++) {
		int jobResult = executeJob(splittedJobs[i]);
		if (jobResult != -1)
			forkedProcesses += jobResult;
	}
	return forkedProcesses;
}

/**
 * @brief Function that executes a script given its full command arguments vector.
 *
 * @param args Number of command arguments
 * @param argv Command arguments vector / array
 * @return The number of forked processes: OK / -1: Error occurred
 */
int executeScriptUsingArguments(int args, char *argv[]) {
	// Concatenate script to avoid invalid word-tokenization on whitespace characters
	char *script = (char*) malloc(MAX_SCRIPT_SIZE * sizeof(char));
	int argsIndex;
	for (argsIndex = 1; argsIndex < args; argsIndex++) {
		strcat(script, argv[argsIndex]);
		if (argv[argsIndex][strlen(argv[argsIndex]) - 1] != ';')
			strcat(script, " ");
	}
	char *scriptCopy = (char*) malloc((strlen(script) + 1) * sizeof(char));
	if (scriptCopy == NULL ) {
		perror("malloc error");
		return -1;
	}
	strcpy(scriptCopy, script);
	free(script);
	return executeScript(scriptCopy);
}
