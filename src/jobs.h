/*  @file jobs.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Job-handling functions header
 */

#ifndef JOBS_H_
#define JOBS_H_

#define MAX_SCRIPT_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string_processing.h"
#include "processes.h"
#include "pipes.h"
#include "commands.h"

/**
 * @brief Function that carries out the execution of a complete given jobScript.
 * The job may consist of multiple commands, containing pipes and redirections.
 *
 * @param jobScript
 * @return The number of forked processes / -1: Error occurred
 */
int executeJob(char *jobScript);

/** @brief This function splits the individual jobs, separated by ';' or '&'.
 * The full job script is given as the first parameter,
 * while the second parameter is filled with up to MAX_JOBS individual jobs.
 *
 * @param jobScript The entire job script
 * @param splittedJobs Splitted jobs container
 * @param countOnly Whether to count only / or also to split the script
 * @param wordsCount Number of words in the script
 * @param splittedWords Splitted words container
 * @return The number of jobs discovereds
 */
int splitJobs(char *jobScript, char ***splittedJobs, int countOnly,
		int wordsCount, char *splittedWords[]);

/**
 * @brief Function the discovers the pipeline structure within a given job.
 *
 * @param pipeDelimited A job containing process commands delimiter using pipes
 * @param pipedProcesses Container to be filled with single pipelined processes
 * @param countOnly Whether to count only / or also to split the job
 */
int getPipedProcesses(char *pipeDelimited, char ***pipedProcesses,
		int countOnly);

/**
 * @brief Function that executes a sequence of piped commands and handles their communication.
 *
 * @param pipedCount
 * @param pipedJob
 * @return The number of forked processes / -1: Error occurred
 */
int handlePipedCommands(int pipedCount, char *pipedCommand);

#endif /* JOBS_H_ */
