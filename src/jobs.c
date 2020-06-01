/*  @file jobs.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Job-handling functions implementation
 */

#include "jobs.h"

/* @brief Function that starts a job.
 *
 *  @return Job index: OK / -1: Job could not be started
 */
int jobStarted() {
	if (activeJobs == MAX_JOBS_RUNNING) {
		printf("Insufficient Resources\n");
		return -1;
	}
	activeJobs++;
	int i;
	for (i = 0; i < MAX_JOBS_RUNNING; i++)
		if (jobsRunning[i] == 0) {
			jobsRunning[i] = 1;
			jobProcessesActive[i] = 0;
			return i;
		}
	return -1;
}

/**
 * @brief Check if background symbol '&' occurs and is so, it is removed from the job script
 *
 * @param jobScript
 * @param arguments
 * @param args
 * @return Whether is a background job
 */
int isBackground(char **jobScript, char **arguments, int *args) {
	if ((*args) == 0) {
		if ((*jobScript)[strlen(*jobScript) - 1] == '&')
			return 1;
		return 0;
	}
	// Check if the background ampersand character has been passed separately
	if ((strcmp(arguments[(*args) - 1], "&") == 0)
			|| (arguments[(*args) - 1][strlen(arguments[(*args) - 1]) - 1]
					== '&')) {
		(*args)--;
		return 1;
	}
	// Ignore all possible whitespace characters at the end of the script
	removeSpacesFromBeginning(jobScript);
	int indexToTheEnd = strlen(*jobScript) - 1;
	char lastChar = (*jobScript)[indexToTheEnd];
	// If '& character is found at the end of the script'
	if (lastChar == '&') {
		// Terminate the string right after the '&' character
		(*jobScript)[indexToTheEnd] = '\0';
		return 1;
	}
	// If '&' not found at the end
	return 0;
}

/**
 * @brief Function that executes a sequence of piped commands and handles their communication.
 *
 * @param pipedCount
 * @param pipedJob
 * @return The number of forked processes / -1: Error occurred
 */
int handlePipedCommands(int pipedCount, char *pipedJob) {
	char **pipedProcesses = (char**) malloc(pipedCount * sizeof(char*));
	if (pipedProcesses == NULL) {
		perror("malloc error");
		return -1;
	}
	char *pipedJobCopy = (char*) malloc((strlen(pipedJob) + 1) * sizeof(char));
	if (pipedJobCopy == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(pipedJobCopy, pipedJob);
	if (getPipedProcesses(pipedJobCopy, &pipedProcesses, 0) == -1)
		return -1;
	int forkedProcesses = 0;
	// Create the intermediate pipes
	char *pipesArray[pipedCount - 1];
	if (pipedCount > 1) {
		if (createPipes(pipedCount, pipesArray) == -1)
			return -1;
	}
	// Start a new job to execute the processes
	int jobIndex = 0;
	if (pipedCount > 1) {
		int jobIndex = jobStarted();
		if (jobIndex == -1)
			return -1;
	}
	// Execute the piped processes
	int lastInBackground = (pipedJob[strlen(pipedJob) - 1] == '&');
	int processError = 0;
	int i;
	for (i = 0; i < pipedCount; i++) {
		// Parse arguments
		char *commandName;
		char **commandArguments;
		int backgroundProcess = 1;
		int argsCount = parseCommand(pipedProcesses[i], &commandName,
				&commandArguments);
		// Define whether the process is a background or a foreground one
		// In case of more that one piped processes , they should be executed in the background,
		// in order not to be blocked due to possible full pipes
		int userBackground = isBackground(&commandName, commandArguments,
				&argsCount);
		if (pipedCount == 1)
			backgroundProcess = userBackground;
		// If the command is a bash built-in function,
		// it is executed within the program, without any forked processes (returns 0 forked count).
		if (executeBashBuiltinFunction(commandName, commandArguments,
				argsCount))
			continue;
		// Allocate job space in not a bash built-in function/command
		if (pipedCount == 1) {
			jobIndex = jobStarted();
			if (jobIndex == -1)
				return -1;
		}
		// Launch the process in the system (if it is a valid command)
		// Check for validity as a system command
		char commandExistenceCheck[strlen(commandName) + 18];
		char commandNameProcessedCut[strlen(commandName) + 1];
		char commandNameProcessedLower[strlen(commandName) + 1];
		strcpy(commandNameProcessedCut, commandName);
		if (commandNameProcessedCut[strlen(commandNameProcessedCut) - 1] == '&')
			commandNameProcessedCut[strlen(commandNameProcessedCut) - 1] = '\0';
		strcpy(commandNameProcessedLower, commandNameProcessedCut);
		int caseDifferrent = toLowerCase(commandNameProcessedLower);
		strcpy(commandExistenceCheck, "which ");
		strcat(commandExistenceCheck, commandNameProcessedLower);
		strcat(commandExistenceCheck, " &>/dev/null");
		if ((system(commandExistenceCheck)) == 0 && (!caseDifferrent)) {
			int executionResult = executeProcess(jobIndex, commandName,
					commandArguments, backgroundProcess, argsCount, i,
					pipedCount, pipesArray, pipedProcesses[i],
					lastInBackground);
			// Display the background status of the job
			if (lastInBackground)
				printf("[%d] %d (%s) Job: %s\n", jobIndex + 1,
						jobPIDs[jobIndex][i], commandNameProcessedCut,
						pipedJob);
			if (executionResult != -1)
				forkedProcesses += executionResult;
		} else {
			fprintf(stderr, "nicpoyia-sh: %s: command not found\n",
					commandNameProcessedCut);
			processError = 1;
		}
	}
	free(pipedJob);
	free(pipedJobCopy);
	// If an error prevented a pipelined a process to start, terminate all already created processes
	if (processError) {
		int processIndex;
		for (processIndex = 0; processIndex < jobProcessesActive[jobIndex];
				processIndex++) {
			kill(jobPIDs[jobIndex][processIndex], SIGKILL);
		}
		// Finish the job
		jobsRunning[jobIndex] = 0;
		activeJobs--;
		return -1;
	}
	// Wait only for pipelines processes, not for background processes
	if ((!lastInBackground) && (pipedCount > 1)) {
		if (pipedCount > 1) {
			// Wait for all background processes of the job to finish
			int pidCounter = 0;
			while (pidCounter < jobProcessesActive[jobIndex]) {
				// Focus on the next process running
				int nextPid = 0;
				int processCounter = 0;
				while ((nextPid == 0) && (processCounter < MAX_ACTIVE_PROCESSES)) {
					nextPid = jobPIDs[jobIndex][processCounter];
					processCounter++;
				}
				int nextPIDStatus;
				// Get the process status
				waitpid(nextPid, &nextPIDStatus, 0);
				jobPIDs[jobIndex][i] = 0;
				processFinished(nextPid);
				pidCounter++;
			}
			jobProcessesActive[jobIndex] = 0;
		}
	}
	if (!lastInBackground) {
		// Finish the job
		jobsRunning[jobIndex] = 0;
		activeJobs--;
	}
	// Destroy pipes
	if (pipedCount > 1) {
		if (destroyPipes(pipedCount, pipesArray) == -1)
			return -1;
	}
	free(pipedProcesses);
	return forkedProcesses;
}

/**
 * @brief Function that carries out the execution of a complete given jobScript.
 * The job may consist of multiple commands, containing pipes and redirections.
 *
 * @param jobScript
 * @return The number of forked processes / -1: Error occurred
 */
int executeJob(char *jobScript) {
	if (jobScript == NULL)
		return 0;
	// Define and handle the pipe-connected processes
	char *pipeConnectedProcessed = (char*) malloc(
			(strlen(jobScript) + 1) * sizeof(char));
	if (pipeConnectedProcessed == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(pipeConnectedProcessed, jobScript);
	int pipedProcessesCount = getPipedProcesses(pipeConnectedProcessed, NULL,
			1);
	strcpy(pipeConnectedProcessed, jobScript);
	int forkedProcesses = handlePipedCommands(pipedProcessesCount,
			pipeConnectedProcessed);
	if (forkedProcesses == -1)
		return -1;
	free(jobScript);
	return forkedProcesses;
}

/**
 * @brief Function that cuts a word into parts according to a specified delimiter.
 * Includes rules for semicolon and ampersand handling.
 *
 * @param word The word to split
 * @param parts Parts container to be filled in
 * @param delimiter Delimiter to be used for tokenization
 * @return The number of parts
 */
int splilToParts(char *word, char***parts, char *delimiter) {
	char * wordCopy = (char*) malloc((strlen(word) + 1) * sizeof(char));
	if (wordCopy == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(wordCopy, word);
	int partsCount = 0;
	char *nextPart = strtok(wordCopy, delimiter);
	while (nextPart != NULL) {
		partsCount++;
		nextPart = strtok(NULL, delimiter);
	}
	(*parts) = (char**) malloc(partsCount * sizeof(char*));
	if ((*parts) == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(wordCopy, word);
	int partIndex = 0;
	nextPart = strtok(wordCopy, delimiter);
	while (nextPart != NULL) {
		// Attach the background symbol, if it is used as a delimiter
		if (delimiter[0] == '&') {
			char *backgroundProcess = (char*) malloc(
					(strlen(nextPart) + 2) * sizeof(char));
			if (backgroundProcess == NULL) {
				perror("malloc error");
				return -1;
			}
			strcpy(backgroundProcess, nextPart);
			if ((partIndex < (partsCount - 1))
					|| (word[strlen(word) - 1] == '&')) {
				strcat(backgroundProcess, "&");
			}
			(*parts)[partIndex] = backgroundProcess;
		} else {
			(*parts)[partIndex] = nextPart;
		}
		partIndex++;
		nextPart = strtok(NULL, delimiter);
	}
	return partsCount;
}

/**
 * @brief Function that splits the command into parts, using both ';' and '&' delimiters
 *
 * @param word The word to split
 * @param parts Parts container to be filled in
 * @return The number of parts
 */
int splitBackgroundAndSerial(char *word, char***parts) {
	// Obtain the elements count using each delimiter
	char **semicolonDevided;
	int semicolonParts;
	if ((semicolonParts = splilToParts(word, &semicolonDevided, ";")) == -1)
		return -1;
	char **ampersandDevided;
	int ampersandParts;
	if ((ampersandParts = splilToParts(word, &ampersandDevided, "&")) == -1)
		return -1;
	// Merge the parts as per the two delimiters
	int totalParts = semicolonParts + ampersandParts + 1;
	(*parts) = (char**) malloc(totalParts * sizeof(char*));
	if ((*parts) == NULL) {
		perror("malloc error");
		return -1;
	}
	int i;
	int totalIndex = 0;
	if (ampersandParts > 0) {
		for (i = 0; i < ampersandParts; i++) {
			free(semicolonDevided);
			int semicolonParts;
			if ((semicolonParts = splilToParts(ampersandDevided[i],
					&semicolonDevided, ";")) == -1)
				return -1;
			int j;
			for (j = 0; j < semicolonParts; j++) {
				(*parts)[totalIndex] = semicolonDevided[j];
				totalIndex++;
				if (totalIndex > totalParts)
					return totalIndex - 1;
			}
		}
	} else {
		(*parts) = semicolonDevided;
		free(ampersandDevided);
	}
	return totalIndex;
}

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
		int wordsCount, char *splittedWords[]) {
	if (countOnly) {
		// Iterate through words
		int wordsIndex = 0;
		char *jobScriptCopy = (char*) malloc(
				(strlen(jobScript) + 1) * sizeof(char));
		strcpy(jobScriptCopy, jobScript);
		char *nextWord = strtok(jobScriptCopy, " ");
		while (nextWord != NULL) {
			wordsIndex++;
			nextWord = strtok(NULL, " ");
		}
		strcpy(jobScriptCopy, jobScript);
		nextWord = strtok(jobScriptCopy, " ");
		int i;
		for (i = 0; i < wordsIndex; i++) {
			splittedWords[i] = nextWord;
			nextWord = strtok(NULL, " ");
		}
		return wordsIndex;
	} else {
		// Iterate through jobs
		int wordIndex = 0;
		int jobIndex = 0;
		// Initialize a container for concatenating words until reading an entire process command
		char *lastConcatJob = (char*) malloc(
				(strlen(jobScript) + 1) * sizeof(char));
		if (lastConcatJob == NULL) {
			perror("malloc error");
			return -1;
		}
		strcpy(lastConcatJob, "");
		while (wordIndex < wordsCount) {
			// Concatenate the next word
			if (strlen(lastConcatJob) == 0)
				strcpy(lastConcatJob, splittedWords[wordIndex]);
			else {
				strcat(lastConcatJob, " ");
				strcat(lastConcatJob, splittedWords[wordIndex]);
			}
			// Check what's next (background character)
			if ((lastConcatJob[strlen(lastConcatJob) - 1] == '&')
					|| (lastConcatJob[strlen(lastConcatJob) - 1] == ';')) {
				char **semicolonAmpDevided;
				int semicolonAmpParts;
				if ((semicolonAmpParts = splitBackgroundAndSerial(lastConcatJob,
						&semicolonAmpDevided)) == -1)
					return -1;
				int semicolonIndex;
				for (semicolonIndex = 0; semicolonIndex < semicolonAmpParts;
						semicolonIndex++) {
					// Store the next job into the array
					if (!countOnly) {
						char *nextJobCopy =
								(char*) malloc(
										(strlen(
												semicolonAmpDevided[semicolonIndex])
												+ 1) * sizeof(char));
						if (nextJobCopy == NULL) {
							perror("malloc error");
							return -1;
						}
						strcpy(nextJobCopy,
								semicolonAmpDevided[semicolonIndex]);
						removeSpacesFromBeginning(&nextJobCopy);
						(*splittedJobs)[jobIndex] = nextJobCopy;
					}
					jobIndex++;
				}
				// Reset container
				int z;
				for (z = 0; z < strlen(lastConcatJob); z++) {
					lastConcatJob[z] = '\0';
				}
				free(semicolonAmpDevided);
			}
			wordIndex++;
		}
		// Process any final command not terminated with a semicolon or ampersand
		if (strlen(lastConcatJob) > 0) {
			char **semicolonAmpDevided;
			int semicolonAmpParts;
			if ((semicolonAmpParts = splitBackgroundAndSerial(lastConcatJob,
					&semicolonAmpDevided)) == -1)
				return -1;
			int semicolonIndex;
			for (semicolonIndex = 0; semicolonIndex < semicolonAmpParts;
					semicolonIndex++) {
				// Store the next job into the array
				if (!countOnly) {
					char *nextJobCopy = (char*) malloc(
							((strlen(semicolonAmpDevided[semicolonIndex]) + 1)
									* sizeof(char)));
					if (nextJobCopy == NULL) {
						perror("malloc error");
						return -1;
					}
					strcpy(nextJobCopy, semicolonAmpDevided[semicolonIndex]);
					removeSpacesFromBeginning(&nextJobCopy);
					(*splittedJobs)[jobIndex] = nextJobCopy;
				}
				jobIndex++;
			}
			// Reset container
			free(semicolonAmpDevided);
		}
		return jobIndex;
	}
}

/**
 * @brief Function the discovers the pipeline structure within a given job.
 *
 * @param pipeDelimited A job containing process commands delimiter using pipes
 * @param pipedProcesses Container to be filled with single pipelined processes
 * @param countOnly Whether to count only / or also to split the job
 */
int getPipedProcesses(char *pipeDelimited, char ***pipedProcesses,
		int countOnly) {
	int processIndex = 0;
	char *nextPipedProcess = strtok(pipeDelimited, "|");
	while (nextPipedProcess != NULL) {
		// Store the next job into the array
		if (!countOnly) {
			char *nextPipedProcessCopy = (char*) malloc(
					(strlen(nextPipedProcess) + 1) * sizeof(char));
			if (nextPipedProcessCopy == NULL) {
				perror("malloc error");
				return -1;
			}
			strcpy(nextPipedProcessCopy, nextPipedProcess);
			removeSpacesFromBeginning(&nextPipedProcessCopy);
			(*pipedProcesses)[processIndex] = nextPipedProcessCopy;
		}
		processIndex++;
		nextPipedProcess = strtok(NULL, "|");
	}
	return processIndex;
}
