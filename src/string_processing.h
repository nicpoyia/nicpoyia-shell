/*  @file string_processing.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief String processing utility functions header
 */

#ifndef STRING_PROCESSING_H_
#define STRING_PROCESSING_H_

#include <ctype.h>
#include <string.h>

/**
 * @brief Function that removes all spaces before the first printable character of the string.
 *
 * @param string The string to process
 */
void removeSpacesFromBeginning(char **string);

/**
 * @brief Function that transforms all string's characters to the corresponding lower-case ones
 *
 * @param string The string to process
 * @return Whether thera is a diff between the string before and after processing
 */
int toLowerCase(char *string);

#endif /* STRING_PROCESSING_H_ */
