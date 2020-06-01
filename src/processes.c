/*  @file processes.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Process-handling functions implementation
 */

#include "processes.h"

/** @brief Function that finds the job in which a process was launched.
 *
 * @param pid
 * @return Job Index: OK / -1: Not found
 */
int getJobIndex(int pid) {
	int i, j;
	for (i = 0; i < MAX_JOBS_RUNNING; i++)
		for (j = 0; j < MAX_ACTIVE_PROCESSES; j++)
			if (jobPIDs[i][j] == pid)
				return i;
	return -1;
}

/** @brief Function that finds the process position in the process allocation table.
 *
 * @param pid
 * @return Process Index: OK / -1: Not found
 */
int getProcessIndex(int pid) {
	int i;
	for (i = 0; i < MAX_ACTIVE_PROCESSES; i++)
		if (processes[i] == pid)
			return i;
	return -1;
}

/** @brief Function that removes a process from the job's running processes.
 *
 * @param jobIndex
 * @param pid
 * @return 0: OK / -1: Not found
 */
int jobProcessCompleted(int jobIndex, int pid) {
	if (jobProcessesActive[jobIndex] == 0)
		return -1;
	int i;
	for (i = 0; i < MAX_ACTIVE_PROCESSES; i++) {
		if (jobPIDs[jobIndex][i] == pid) {
			jobPIDs[jobIndex][i] = 0;
			jobProcessesActive[jobIndex]--;
			if (jobProcessesActive[jobIndex] == 0) {
				jobsRunning[jobIndex] = 0;
				activeJobs--;
				printf("[%d]+\tJob Finished (done/exited/stopped)\n",
						(jobIndex + 1));
			}
			return 0;
		}
	}
	return -1;
}

/** @brief Function that releases every background process that has been completed.
 * Used before a job execution, in order to free some process space.
 * Notifies the user that the job-number has been completed.
 */
void releaseCompleteBackgroundProcesses() {
	int backgroundProcesses = actPrCount;
	int i;
	for (i = 0; i < backgroundProcesses; i++) {
		// For every active process
		int nextPid = processes[i];
		int nextPidStatus;
		// Return immediately, if no child-process with PID=nextPid has finished (WNOHANG)
		waitpid(nextPid, &nextPidStatus, WNOHANG);
		// Check if completed
		if ((WIFEXITED(nextPidStatus)) || (WIFSTOPPED(nextPidStatus))
				|| (WIFSIGNALED(nextPidStatus))) {
			// If so, release it from the allocation table.
			int jobIndex = getJobIndex(nextPid);
			if (jobIndex != -1)
				jobProcessCompleted(jobIndex, nextPid);
			deallocateProcess(i);
		}
	}
}

// Flag that shows whether a process is in the foreground.
// Variable contains the PID of the foreground process.
// If the flags is enabled:
// 	Any terminal signal is be forwarded to the foreground process.
int foregroundProcess = 0;

/** @brief Signal handler that handles or forwards any signal received.
 *
 * @param signalCode
 */
void signal_handler(int signalCode) {
	if (foregroundProcess == 0) {
		// Handle the signal if no process is in the foreground
		//
		// Revert back to the native signal handler
		void *signal_handler = nativeSignalHandlerFPs[signalCode];
		nativeSignalHandlerFPs[signalCode] = signal(signalCode, signal_handler);
		// Send the signal, so as to be handled by the native signal handler
		kill(getpid(), signalCode);
		// Replace the native signal handler again
		nativeSignalHandlerFPs[signalCode] = signal(signalCode, signal_handler);
	} else {
		// Forward the signal to the foreground process, if any
		kill(foregroundProcess, signalCode);
	}
}

// Standard I/O file descriptors for the current process
int stdinFD = STDIN_FILENO;
int stdoutFD = STDOUT_FILENO;
int stderrFD = STDERR_FILENO;

/** @brief Function that starts a process within a running job.
 *
 * @param jobIndex
 * @param pid
 * @return Process index: OK / -1: Process could not be started
 */
int processStarted(int jobIndex, int pid) {
	if (jobProcessesActive[jobIndex] == MAX_ACTIVE_PROCESSES)
		return -1;
	jobPIDs[jobIndex][jobProcessesActive[jobIndex]] = pid;
	jobProcessesActive[jobIndex]++;
	return 0;
}

/** @brief Function to finish a process.
 *
 * @param pid
 * @return Process index: OK / -1: Process could not be finished
 */
int processFinished(int pid) {
	if (pid == -1)
		return -1;
	int i;
	for (i = 0; i < MAX_ACTIVE_PROCESSES; i++) {
		if (processes[i] == pid) {
			deallocateProcess(i);
			return 0;
		}
	}
	return -1;
}

/**
 *  @brief Function that initializes the process information
 */
void processesInitialization() {
	int prInit;
	for (prInit = 0; prInit < MAX_ACTIVE_PROCESSES; prInit++) {
		processes[prInit] = 0;
	}
	actPrCount = 0;
}

/** @brief This function is responsible for reserving one out
 * of the ten available process positions is the shell.
 *
 * @return The process index [0-9]: If OK / -1: If the process could not be allocated
 */
int allocateProcess() {
	if (actPrCount == MAX_ACTIVE_PROCESSES) {
		printf("Insufficient Resources\n");
		return -1;
	}
	int prIndex = 0;
	while (processes[prIndex]) {
		prIndex++;
	}
	processes[prIndex] = 1;
	actPrCount++;
	return prIndex;
}

/** @brief Function that releases a process that has finished executing.
 *
 * @param processIndex
 * @return 0: If released OK / -1: If no process to release
 */
int deallocateProcess(int processIndex) {
	if (actPrCount == 0)
		return -1;
	if (processes[processIndex] == 0)
		return -1;
	processes[processIndex] = 0;
	actPrCount--;
	return 0;
}

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
		char *pipesArray[], char *processString, int lastInBackground) {
	// Reset standard I/O file descriptors
	stdinFD = STDIN_FILENO;
	stdoutFD = STDOUT_FILENO;
	stderrFD = STDERR_FILENO;
	// Allocate process space within the shell
	int processIndex = allocateProcess();
	if (processIndex == -1) {
		return -1;
	}
	// If any processes need to be forked
	if (actPrCount == 0)
		return -1;
	if (processes[processIndex] == 0)
		return -1;
	// PROCESS EXECUTION
	int processPid;
	if ((processPid = fork()) == -1) {
		perror("fork error");
		return -1;
	}
	if (isBackground) {
		// Remove the background ampersand character from the command name
		if (commandName[strlen(commandName) - 1] == '&')
			commandName = subString(commandName, 0, strlen(commandName) - 1);
	}
	//------------------------------ Parent-Process ------------------------------//
	if (processPid > 0) {
		// Store PID of forked process
		processes[processIndex] = processPid;
		if (isBackground) {
			// Remove the background ampersand character from the command name
			if (commandName[strlen(commandName) - 1] == '&')
				commandName = subString(commandName, 0,
						strlen(commandName) - 2);
			processStarted(jobIndex, processPid);
		} else {
			foregroundProcess = processPid;
			// Wait for the process to complete
			int childStatus;
			wait(&childStatus);
			// Release the process
			deallocateProcess(processIndex);
			foregroundProcess = 0;
		}
		if (!lastInBackground) {
			// Release the process
			deallocateProcess(processIndex);
			foregroundProcess = 0;
		}
		free(commandName);
		int i;
		for (i = 0; i < args; i++)
			free(commandArguments[i]);
		free(commandArguments);
		return 1;
	}
	//------------------------------ Child-Process ------------------------------//
	else {
		// Define and execute I/O redirections
		int IOArgs;
		if ((IOArgs = executeRedirections(processString, 0, 0, 0, 0)) == -1)
			return -1;
		int nonIOArgs = args - IOArgs;
		//
		// Handle pipe redirections redirections
		//
		// Read from the previous pipe (except first process)
		if (executeRedirections(commandName, 1, pipesArray[pipelinePos - 1],
				pipelinePos, pipelineCount) == -1)
			return -1;
		// Write to the next pipe (except last process)
		if (executeRedirections(commandName, 2, pipesArray[pipelinePos],
				pipelinePos, pipelineCount) == -1)
			return -1;
		//
		// Filter out the arguments about redirections
		char *nonIOArguments[nonIOArgs + 2];
		// Clear container in case of dangling data pointed
		int argsCounter;
		for (argsCounter = 0; argsCounter < nonIOArgs + 2; argsCounter++)
			nonIOArguments[argsCounter] = NULL;
		// Fill in the execution arguments
		int i;
		for (i = 0; i < nonIOArgs; i++)
			nonIOArguments[i + 1] = commandArguments[i];
		nonIOArguments[nonIOArgs + 1] = NULL;
		nonIOArguments[0] = commandName;
		// Replace the text-segment
		int execRes;
		execRes = execvp(commandName, nonIOArguments);
		if (execRes == -1) {
			perror("execvp");
			return -1;
		}
	}
	//---------------------------------------------------------------------------//
	return 0;
}

/** @brief Function that checks whether a character represents a number in the range [0-9].
 *
 * @param character
 * @return 1 if it is a file descriptor / 0 if it is not a number.
 */
int isFileDescriptor(char character) {
	int charOffset = character - '0';
	if ((charOffset >= 0) && (charOffset <= 2))
		return 1;
	if (character == '&')
		return 1;
	return 0;
}

/** @brief Function that checks if a redirection string is a file descriptor.
 * ---> File descriptors are defined using the '&' symbol.
 *
 * @param redirectionString
 * @return the file descriptor number if is is a file descriptor / 0 if it is not a file descriptor.
 */
int checkIfFd(char *redirectionString) {
	if (redirectionString == NULL)
		return 0;
	if (strlen(redirectionString) == 0)
		return 0;
	// If it is a file descriptor, parse the text after the '&' as a number
	if (redirectionString[0] == '&') {
		int fdNumber = atoi(redirectionString + 1);
		return fdNumber;
	}
	return 0;
}

/**
 * @brief Function that checks whether a character is a redirection symbol (<,>)
 *
 * @param character
 * @return 1: true / 0: false
 */
int isRedirectionSymbol(char character) {
	if (character == '<')
		return 1;
	if (character == '>')
		return 1;
	return 0;
}

/** @brief Method that finds the redirection string defined for a certain redirection string
 *
 *  * Redirection Strings:
 * 0 < / 0<
 * 1 > / 1>
 * 2 > / 2>
 * & > / &>
 *
 * Fills in the target character array by reference
 * Returns the redirection type:
 * 	- 1: Redirection from to StdIn
 * 	- 2: Redirection from StdOut
 * 	- 3: Redirection from StdErr
 * 	- 4: Redirection from both StdOut and StdErr
 * 	- 5: Redirection from StdOut (append mode)
 *
 * 	Returns 0 if no valid redirection occurs
 * 	Returns -1 if an error occurred
 *
 * @param redString The entire redirection string
 * @param target The redirection target pointer
 * @return redirection type / 0 / -1
 */
int findRedirections(char *redString, char **target) {
	if (redString == NULL)
		return 0;
	// Split the redirection type and target
	// Redirection Type
	char *redType;
	int redTypeLength;
	if (isRedirectionSymbol(redString[0])) {
		if (redString[0] == redString[1])
			redTypeLength = 2;
		else
			redTypeLength = 1;
	} else {
		if ((redString[1] == ' ') && (isRedirectionSymbol(redString[2])))
			redTypeLength = 3;
		else
			redTypeLength = 2;
	}
	redType = (char*) malloc((redTypeLength + 1) * sizeof(char));
	if (redType == NULL) {
		perror("malloc error");
		return -1;
	}
	int i;
	for (i = 0; i < redTypeLength; i++)
		redType[i] = redString[i];
	redType[redTypeLength] = '\0';
	// Redirection Target
	int redTargetLength = strlen(redString) - redTypeLength;
	char *redTarget = (char*) malloc(redTargetLength * sizeof(char));
	if (redTarget == NULL) {
		perror("malloc error");
		return -1;
	}
	int targetIndex = 0;
	for (i = 0; i < redTargetLength; i++) {
		char nextCharacter = redString[i + redTypeLength];
		if (nextCharacter != ' ') {
			redTarget[targetIndex] = nextCharacter;
			targetIndex++;
		}
	}
	redTarget[targetIndex] = '\0';
	(*target) = redTarget;
	// Define the type of redirection
	// Check the first and last characters of the redirection type
	// E.g. 1 > will be checked by the first and third characters
	// while &> will be checked by the first and second characters
	switch (redType[redTypeLength - 1]) {
	case '<':
		if ((strlen(redType) == 1) || (redType[0] == '0'))
			return 1;
		break;
	case '>':
		if (redTypeLength == 1)
			return 2;
		else
			switch (redType[strlen(redType) - 2]) {
			case '1':
				return 2;
				break;
			case '2':
				return 3;
				break;
			case '&':
				return 4;
				break;
			case '>':
				return 5;
				break;
			}
		break;
	}
	return 0;
}

/**
 * @brief Function that checks whether a string is a redirection (>..., ...<)
 *
 * @param string
 * @return 0 if it is not a redirection / 1 if has an internal redirection (e.g. 1>..., 0<...) / 2 if has an external redirection (e.g. 1>, 0<).
 */
int isRedirection(char *string) {
	if (string == NULL)
		return 0;
	if (strlen(string) == 0)
		return 0;
	// If external redirection exist
	if (isRedirectionSymbol(string[strlen(string) - 1]))
		return 2;
	int i;
	// Check for internal redirections
	for (i = 0; i < strlen(string); i++)
		if (isRedirectionSymbol(string[i]))
			return 1;
	// If no redirection found
	return 0;
}

/** @brief Function that extracts the redirection statements from the full command string.
 * The redirection statements rest after the command arguments.
 *
 * For example:
 * 		command1 arg1 arg2 > file1.txt 2> file2.txt
 * 			would produce the following array:
 * 			[> file1.txt, 2> file2.txt]
 *
 * Fills in the redArgs referenced via an argument, that represents
 * the number of arguments that have to do with redirections.
 *
 * @param commandCopy Copy of the full command
 * @param redirectionsArray Redirection arguments array
 * @param redArgs Redirection arguments count
 * @return the number of redirections
 */
int splitRedirectionStrings(char *commandCopy, char***redirectionsArray,
		int *redArgs) {
	int redirections = 0;
	// Allocate temporary space for the maximum redirections possible:
	// (input, output, error = 3 possible redirections).
	char **redStrings = (char**) malloc(3 * sizeof(char*));
	if (redStrings == NULL) {
		perror("malloc error");
		return -1;
	}
	// Parse the command by splitting a copy of it by the whitespace character
	char nextString[MAX_PROCESS_SIZE] = "";
	int nextStringArgs = 0;
	int redirectionType;
	int redSymbolPassed = 0;
	int isRedirectionFlag = 0;
	char *nextCommandWord = strtok(commandCopy, " ");
	(*redArgs) = 0;
	// Word-tokenization procedure
	while (nextCommandWord != NULL) {
		nextStringArgs++;
		// Ignore command and arguments
		if (!(isFileDescriptor(nextCommandWord[0]))
				&& !(isRedirectionSymbol(nextCommandWord[0]))) {
			if (!redSymbolPassed && !redirections) {
				nextCommandWord = strtok(NULL, " ");
				nextStringArgs = 0;
				continue;
			}
		}
		// For the redirection region of the command
		strcat(nextString, nextCommandWord);
		strcat(nextString, " ");
		redirectionType = isRedirection(nextCommandWord);
		// If the word is a redirection at all
		if (redirectionType == 1) {
			isRedirectionFlag = 1;
		}
		// If a redirection symbol occurred at the end of the word
		else if (redirectionType == 2) {
			if (redSymbolPassed == 0)
				redSymbolPassed = 1;
			// Return NULL pointer if a redirection syntax error occur
			else
				return -1;
		}
		// If a redirection symbol (<,>) has occurred before, and a string follows
		else if (redirectionType == 0) {
			if (redSymbolPassed) {
				isRedirectionFlag = 1;
				redSymbolPassed = 0;
			}
		}
		// The next redirection statement has been parsed, and so it is stored
		if (isRedirectionFlag) {
			(*redArgs) += nextStringArgs;
			nextStringArgs = 0;
			char *nextRedirectionCopy = (char*) malloc(
					(strlen(nextString) + 1) * sizeof(char));
			if (nextRedirectionCopy == NULL) {
				perror("malloc error");
				return -1;
			}
			strcpy(nextRedirectionCopy, nextString);
			redStrings[redirections] = nextRedirectionCopy;
			redirections++;
			// Clear the next string container
			nextString[0] = '\0';
			redSymbolPassed = 0;
			isRedirectionFlag = 0;
		}
		nextCommandWord = strtok(NULL, " ");
	}
	// Reallocate space to return an array with the required size,
	// according to the redirections found while parsing.
	*redirectionsArray = (char**) malloc(redirections * sizeof(char*));
	if ((*redirectionsArray) == NULL) {
		perror("malloc error");
		return -1;
	}
	// Copy the found redirections
	int i;
	for (i = 0; i < redirections; i++) {
		(*redirectionsArray)[i] = redStrings[i];
	}
	// Release the temporary array space
	free(redStrings);
	return redirections;
}

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
		int pipelinePos, int pipelineCount) {
	// Pipe redirection
	if (toPipe) {
		// Read from pipe
		if (toPipe == 1) {
			if (pipelinePos > 0) {
				int fifoFD;
				if ((fifoFD = open(fifoName, O_RDONLY)) == -1) {
					perror("opening fifo for reading");
					return -1;
				}
				if (dup2(fifoFD, stdinFD) == -1)
					return -1;
			}
		}
		// Write to pipe
		else if (toPipe == 2) {
			if (pipelinePos < (pipelineCount - 1)) {
				int fifoFD;
				if ((fifoFD = open(fifoName, O_WRONLY)) == -1) {
					perror("opening fifo for writing");
					return -1;
				}
				if (dup2(fifoFD, stdoutFD) == -1)
					return -1;
			}
		}
		return 0;
	}
	// Parse the command to find the redirections
	char *redirectFromInput = NULL;
	char *redirectToStdOut = NULL;
	char *redirectToStdOutAppend = NULL;
	char *redirectToStdErr = NULL;
	// Get the redirection statements after the command arguments
	char *commandCopy = (char*) malloc((strlen(command) + 1) * sizeof(char));
	if (commandCopy == NULL) {
		perror("malloc error");
		return -1;
	}
	strcpy(commandCopy, command);
	char **redirectionStrings;
	int redArgs;
	int redStringsCount = splitRedirectionStrings(commandCopy,
			&redirectionStrings, &redArgs);
	if (redirectionStrings == NULL) {
		return -1;
	}
	int redCounter;
	for (redCounter = 0; redCounter < redStringsCount; redCounter++) {
		char *nextRedString = redirectionStrings[redCounter];
		char *redTarget;
		int redType = findRedirections(nextRedString, &redTarget);
		if (redType == -1)
			return -1;
		switch (redType) {
		case 1:
			redirectFromInput = redTarget;
			break;
		case 2:
			redirectToStdOut = redTarget;
			break;
		case 3:
			redirectToStdErr = redTarget;
			break;
		case 4:
			redirectToStdOut = redTarget;
			redirectToStdErr = redTarget;
			break;
		case 5:
			redirectToStdOutAppend = redTarget;
			break;
		}
	}
	// Standard Input redirection
	if (redirectFromInput != NULL) {
		int fd;
		if ((fd = checkIfFd(redirectFromInput)) > 0) {
			if (redirectStdInFd(fd) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
			stdinFD = fd;
		} else {
			if ((stdinFD = redirectStdIn(redirectFromInput)) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
		}
	}
	// Standard Output redirection (append mode >>)
	if (redirectToStdOutAppend != NULL) {
		int fd;
		if ((fd = checkIfFd(redirectToStdOutAppend)) > 0) {
			if (redirectStdOutFd(fd) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
			stdoutFD = fd;
		} else {
			if ((stdoutFD = redirectStdOut(redirectToStdOutAppend, 1)) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
		}
	}
	// Standard Output redirection
	else if (redirectToStdOut != NULL) {
		int fd;
		if ((fd = checkIfFd(redirectToStdOut)) > 0) {
			if (redirectStdOutFd(fd) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
			stdoutFD = fd;
		} else {
			if ((stdoutFD = redirectStdOut(redirectToStdOut, 0)) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
		}
	}
	// Standard Error redirection
	if (redirectToStdErr != NULL) {
		int fd;
		if ((fd = checkIfFd(redirectToStdErr)) > 0) {
			if (redirectStdErrFd(fd) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
			stderrFD = fd;
		} else {
			if ((stderrFD = redirectStdErr(redirectToStdErr)) == -1)
				fprintf(stderr, "Error while redirecting input/output\n");
		}
	}
	return redArgs;
}
