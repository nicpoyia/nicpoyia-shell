/*  @file string_processing.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief String processing utility functions implementation
 */

#include "string_processing.h"

/**
 * @brief Function that removes all spaces before the first printable character of the string.
 *
 * @param string The string to process
 */
void removeSpacesFromBeginning(char **string) {
	// Remove spaces from the beginning of the string
	int firstPos = 0;
	while ((*string)[firstPos] == ' ') {
		(*string) += sizeof(char);
		firstPos++;
	}
}

/**
 * @brief Function that transforms all string's characters to the corresponding lower-case ones
 *
 * @param string The string to process
 * @return Whether there is a difference between the string before and after processing
 */
int toLowerCase(char *string) {
	int diff = 0;
	int i;
	for (i = 0; i < strlen(string); i++) {
		char lowerCase = tolower(string[i]);
		if (string[i] != lowerCase)
			diff = 1;
		string[i] = lowerCase;
	}
	return diff;
}
