/*  @file commands.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Command-parsing functions implementation
 */

#include "commands.h"

/** @brief Function the takes the full command.
 * Command includes the executable name/path and [some arguments]
 *
 * Fills in the arguments array of strings
 *
 * @param command The full command string
 * @param commandName Container to store the command name
 * @param arguments Container to store the command arguments
 * @return the number of arguments: OK / -1: Error
 */
int parseCommand(char *command, char **commandName, char ***arguments) {
	// Count the command words
	// Do not count the command name
	int argumentsCount = -1;
	char *commandCopy = (char*) malloc((strlen(command)+1)*sizeof(char));
	if (commandCopy == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(commandCopy, command);
	char *nextWord = strtok(commandCopy, " ");
	while (nextWord != NULL) {
		argumentsCount++;
		nextWord = strtok(NULL, " ");
	}
	// Allocate space for the arguments array
	(*arguments) = (char**) malloc(argumentsCount * sizeof(char*));
	if ((*arguments) == NULL) {
		perror("malloc error");
		return -1;
	}
	// Parse the individual arguments
	// Command name is the first word
	strcpy(commandCopy, command);
	nextWord = strtok(commandCopy, " ");
	char *commandNameCopy = (char*) malloc((strlen(nextWord)+1)*sizeof(char));
	if (commandNameCopy == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(commandNameCopy, nextWord);
	(*commandName) = commandNameCopy;
	// Split the arguments
	// Multiple spaces are ignored
	int argIndex = 0;
	nextWord = strtok(NULL, " ");
	while (argIndex < argumentsCount) {
		if (nextWord != NULL) {
			char *argumentCopy = (char*) malloc((strlen(nextWord)+1)*sizeof(char));
			if (argumentCopy == NULL) {
				perror("malloc error");
				return -1;
			}
			strcpy(argumentCopy, nextWord);
			removeSpacesFromBeginning(&argumentCopy);
			(*arguments)[argIndex] = argumentCopy;
			argIndex++;
		}
		nextWord = strtok(NULL, " ");
	}
	return argumentsCount;
}
