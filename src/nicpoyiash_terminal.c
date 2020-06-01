/*  @file nicpoyiash_terminal.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief Terminal interaction dynamic behaviour implementation
 */

#include "nicpoyiash_terminal.h"

// Boolean value that indicated whether the terminal
// is going to continue asking for commands from the user.
// This value is turned to zero, if the shell exits.
int terminalActive = 1;
// Number of processes forked so far.
int forkedProcesses = 0;
// Whether the terminal should wait blocked for the user,
// to complete the input for the previous command.
int blockedForInput = 0;

/**
 * @brief Function the displays the command line prompt,
 * which signs that the shell is ready to get new commands from the user.
 */
void printCommandPrompt() {
	printf("%d-nicpoyia-sh>", forkedProcesses);
}

/**
 * @brief Function that starts the terminal interaction with the user.
 * This function handles the terminal user I/O interaction.
 */
void startTerminal() {
	char nextUserCommand[MAX_SCRIPT_SIZE];
	nextUserCommand[0] = '\0';
	while (terminalActive) {
		// Release any completed background processes and jobs
		releaseCompleteBackgroundProcesses();
		if (!blockedForInput) {
			// nicpoyia-sh command line prompt is displayed
			printCommandPrompt();
		}
		// Read user command and dynamically allocate the appropriate space to store it.
		fgets(nextUserCommand, MAX_SCRIPT_SIZE, stdin);
		char *inputScript = (char*) malloc(
				(strlen(nextUserCommand) + 1) * sizeof(char));
		if (inputScript == NULL ) {
			perror("malloc error");
			return;
		}
		strcpy(inputScript, nextUserCommand);
		// Remove the end-of-line character
		inputScript[strlen(inputScript) - 1] = '\0';
		// Ordinary command execution
		if (!blockedForInput) {
			int lastForkedProcesses = executeScript(inputScript);
			// If something went wrong during the user command execution
			if (lastForkedProcesses != -1) {
				forkedProcesses += lastForkedProcesses;
			}
		}
		// Continue blocked command, if any
		else {
			continueBashExecution(inputScript);
		}
		// Reset variables
		nextUserCommand[0] = '\0';
		terminalActive = !exitNow();
		// Check if any command left waiting to feed it with input.
		// Also, check if any read operation is waiting to read characters.
		blockedForInput = inputWaiting() || readFromUser();
	}
}
