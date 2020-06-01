/*  @file commands.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Command-parsing functions header
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_processing.h"

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
int parseCommand(char *command, char **commandName, char ***arguments);

#endif /* COMMANDS_H_ */
