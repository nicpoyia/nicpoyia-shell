/*  @file files.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief File-redirection functions header
 */

#ifndef FILES_H_
#define FILES_H_

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

/**
 * @brief Redirects a file to a process standard input
 *
 * @param filename Filename to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdIn(char *filename);

/**
 * @brief Redirects a file descriptor to a process standard input
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdInFd(int fd);

/**
 * @brief Redirects a process standard output to a file.
 *
 * @param filename Filename to redirect
 * @param appendMode Flag to show whether the append symbol used (>>)
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdOut(char *filename, int appendMode);

/**
 * @brief Redirects a process standard output to a file descriptor
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdOutFd(int fd);

/**
 * @brief Redirects a process standard error to a file.
 *
 * @param filename Filename to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdErr(char *filename);

/**
 * @brief Redirects a process standard error to a file descriptor
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdErrFd(int fd);

#endif /* FILES_H_ */
