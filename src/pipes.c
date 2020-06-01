/*  @file pipes.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Interprocess communication function implementation
 */

#include "pipes.h"

/**
 * @brief Function that creates FIFO files to interconnect the piped processes.
 * Fills in the pipesArray reference argument.
 *
 * @param pipedProcesses Number of pipelined processes
 * @param pipesArray Container to be filled with FIFO files
 * @return Number of pipes: OK / -1: Error
 */
int createPipes(int pipedProcesses, char *pipesArray[]) {
	int pipesCount = pipedProcesses - 1;
	if (pipesCount > MAX_PIPES_PER_JOB)
		return -1;
	// Maximum pipes allowed = 256 (2^8) + "fifo" (4 characters)
	char *nextFifoName = (char*) malloc(14 * sizeof(char));
	int i;
	for (i = 0; i < pipesCount; i++) {
		sprintf(nextFifoName, "fifo%d", i);
		if (access(nextFifoName, F_OK) == -1) {
			if (mkfifo(nextFifoName, 0777) == -1) {
				perror("mkfifo");
				return -1;
			}
		}
		pipesArray[i] = (char*) malloc(
				(strlen(nextFifoName) + 1) * sizeof(char));
		strcpy(pipesArray[i], nextFifoName);
	}
	return pipesCount;
}

/**
 * @brief Function that destroys FIFO files that interconnected the piped processes.
 *
 * @param pipedProcesses Number of pipelined processes
 * @param pipesArray Container filled with FIFO files
 * @return Number of pipes: OK / -1: Error
 */
int destroyPipes(int pipedProcesses, char *pipesArray[]) {
	int pipesCount = pipedProcesses - 1;
	if (pipesCount > MAX_PIPES_PER_JOB)
		return -1;
	int i;
	for (i = 0; i < pipesCount; i++) {
		char rmCommand[15];
		strcpy(rmCommand, "rm -f ");
		strcat(rmCommand, pipesArray[i]);
		system(rmCommand);
		free(pipesArray[i]);
	}
	return pipesCount;
}
