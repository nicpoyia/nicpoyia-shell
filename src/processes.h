/*  @file processes.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Process-handling functions header
 */
#ifndef PROCESSES_H_
#define PROCESSES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "bash_builtin_functions.h"
#include "files.h"
#include "pipes.h"

#define MAX_ACTIVE_PROCESSES 10
#define MAX_JOBS_RUNNING 10
#define MAX_PROCESS_SIZE 512

// PIDs of all active processes
// A zero value means a free position
pid_t processes[MAX_ACTIVE_PROCESSES];
// Active processes count;
int actPrCount;

// Native signal handlers
void (*nativeSignalHandlerFPs[32])(int);

// Data containers keeping track of every active job session
//
// Active jobs
int activeJobs;
int jobsRunning[MAX_JOBS_RUNNING];
// Active processes per job
int jobProcessesActive[MAX_JOBS_RUNNING];
int jobPIDs[MAX_JOBS_RUNNING][MAX_ACTIVE_PROCESSES];

/**
 *  @brief Function that initializes the process information
 */
void processesInitialization();

/** @brief Signal handler that handles or forwards any signal received.
 *
 * @param signalCode
 */
void signal_handler(int signalCode);

/** @brief Function to finish a process.
 *
 * @param pid
 * @return Process index: OK / -1: Process could not be finished
 */
int processFinished(int pid);

/** @brief Function that finds the process position in the process allocation table.
 *
 * @param pid
 * @return Process Index: OK / -1: Not found
 */
int getProcessIndex(int pid);

/** @brief Function that releases every background process that has been completed.
 * Used before a job execution, in order to free some process space.
 * Notifies the user that the job-number has been completed.
 */
void releaseCompleteBackgroundProcesses();

/**
 * @brief Function that redirects the I/O of a certain command as described using symbols.
 * Also, it can be used for redirecting the process input or output from/to a pipe.
 *
 * @param command A copy of the full command
 * @param toPipe Type of redirection to a pipe
 * @param fifoName Name of the FIFO file to redirect from/to
 * @param pipelinePos The position of the process in the pipeline
 * @param pipelineCount How many pipelined processes are there
 * @return Number of redirections arguments / 0: OK / -1: Error
 */
int executeRedirections(char *command, int toPipe, char *fifoName,
		int pipelinePos, int pipelineCount);

/** @brief This function is responsible for reserving one out
 * of the ten available process positions is the shell.
 *
 * @return The process index [0-9]: If OK / -1: If the process could not be allocated
 */
int allocateProcess();

/** @brief Function that releases a process that has finished executing.
 *
 * @param processIndex
 * @return 0: If released OK / -1: If no process to release
 */
int deallocateProcess(int processIndex);

/** @brief Function that handles the process creation and concurrent running in the system.
 * This function should be called only after:
 * 	-The process has its own index in the shell
 * 	-The I/O has been properly redirected
 *
 * @param jobIndex The index of the job within the process is executed
 * @param commandName The command name itself
 * @param commandArguments Array of command arguments
 * @param isBackground Whether is is going to be executed in the background
 * @param args The total number of arguments right to the command name
 * @param pipelinePos The position of the process in the pipeline
 * @param pipelineCount How many pipelined processes are there
 * @param pipesArray An array containing names of FIFO files to be used
 * @param processString The full string given from the user for the process
 * @param lastInBackground Whether that last command is given with an ampersand
 * @return The number of forked processes: OK / -1: Error occurred
 */
int executeProcess(int jobIndex, char *commandName, char **commandArguments,
		int isBackground, int args, int pipelinePos, int pipelineCount,
		char *pipesArray[], char *processString, int lastInBackground);

#endif /* PROCESSES_H_ */
