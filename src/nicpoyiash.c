/*  @file nicpoyiash.c
 *  @author Nicolas Poyiadjis
 *
 *  @brief The main file of the nicpoyia-sh shell
 *
 *  It can start both
 *  	- Terminal interaction
 *  	- Command interpreter (using command line argumens, e.g. ./usysh ls -l)
 */

#include <stdio.h>
#include <signal.h>

#include "nicpoyiash_interpreter.h"
#include "nicpoyiash_terminal.h"
#include "processes.h"

/**
 * @brief The main function of the nicpoyia-sh shell
 *
 * @param args Number of command line arguments
 * @param argv Array of command line arguments containing the usysh command at index 0
 * @return error code 0: OK / -1: Error
 */
int main(int args, char *argv[]) {
	// Initialize process handling
	processesInitialization();
	// Handle all possible signals
	int signalCode;
	for (signalCode = 1; signalCode < 32; signalCode++)
		nativeSignalHandlerFPs[signalCode] = signal(signalCode, signal_handler);
	// Start the terminal interaction, if no argument has been passed
	if (args == 1) {
		startTerminal();
	}
	// If some arguments have been passed:
	// Use the shell interpreter using the script passed as command line arguments.
	else {
		if (executeScriptUsingArguments(args, argv) == -1)
			return -1;
	}
	return 0;
}
