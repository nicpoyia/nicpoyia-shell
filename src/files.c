/*  @file files.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief File-redirection functions implementation
 */

#include "files.h"

/**
 * @brief Redirects a file to a process standard input
 *
 * @param filename Filename to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdIn(char *filename) {
	int fd;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		perror("error while opening file for reading");
		return -1;
	}
	return redirectStdInFd(fd);
}

/**
 * @brief Redirects a file descriptor to a process standard input
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdInFd(int fd) {
	dup2(fd, STDIN_FILENO);
	return fd;
}

/**
 * @brief Redirects a process standard output to a file.
 *
 * @param filename Filename to redirect
 * @param appendMode Flag to show whether the append symbol used (>>)
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdOut(char *filename, int appendMode) {
	int flags = O_WRONLY | O_CREAT;
	if (appendMode) {
		flags |= O_APPEND;
	} else {
		flags |= O_TRUNC;
	}
	int fd;
	if ((fd = open(filename, flags, 0660)) < 0) {
		/* Open to-file */
		perror("error while opening file for writing");
		return -1;
	}
	return redirectStdOutFd(fd);
}

/**
 * @brief Redirects a process standard output to a file descriptor
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdOutFd(int fd) {
	dup2(fd, STDOUT_FILENO);
	return fd;
}

/**
 * @brief Redirects a process standard error to a file.
 *
 * @param filename Filename to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdErr(char *filename) {
	int fd;
	if ((fd = open(filename, O_WRONLY | O_CREAT, 0660)) < 0) {
		/* Open to-file */
		perror("error while opening file for writing");
		return -1;
	}
	return redirectStdErrFd(fd);
}

/**
 * @brief Redirects a process standard error to a file descriptor
 *
 * @param fd File desciptor to redirect
 * @return Error code: 0: OK / -1: Error
 */
int redirectStdErrFd(int fd) {
	dup2(fd, STDERR_FILENO);
	return fd;
}
