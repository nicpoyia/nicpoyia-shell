/*  @file pipes.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Interprocess communication function header
 */

#ifndef PIPES_H_
#define PIPES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PIPES_PER_JOB 256
#define READ_FROM_PIPE 0
#define WRITE_TO_PIPE 1

/**
 * @brief Function that creates FIFO files to interconnect the piped processes.
 * Fills in the pipesArray reference argument.
 *
 * @param pipedProcesses Number of pipelined processes
 * @param pipesArray Container to be filled with FIFO files
 * @return Number of pipes: OK / -1: Error
 */
int createPipes(int pipedProcesses, char *pipesArray[]);

/**
 * @brief Function that destroys FIFO files that interconnected the piped processes.
 *
 * @param pipedProcesses Number of pipelined processes
 * @param pipesArray Container filled with FIFO files
 * @return Number of pipes: OK / -1: Error
 */
int destroyPipes(int pipedProcesses, char *pipesArray[]);

#endif /* PIPES_H_ */
