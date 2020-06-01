/*  @file bash_builtin_functions.h
 *  @author Nicolas Poyiadjis
 *
 *  @brief Bash built-in functions header
 */

#include "bash_builtin_functions.h"

// Flag that is turned to one if the exit command is executed.
// Used to let the terminal know when it should be exit.
int exitEnabled = 0;
// Flag that is turned to one if a bash command needs input.
// The terminal should block and wait for the user's input, to be used as the commands input.
int waitForInput = 0;
// Command waiting for input, if any.
char *commandWaiting = NULL;

//--- Read operation ---//
// Terminal blocked for read operation
int waitToRead = 0;
// Variable waiting to be read
char *variableToRead = NULL;

/**
 * @brief Function that defines whether the shell should terminate now.
 *
 * @return Whether the terminal should immediately exit
 */
int exitNow() {
	return exitEnabled;
}

/**
 * @brief Function that defines if the terminal should wait for the user input, to be used for a command.
 *
 * @return Whether the terminal should wait for input
 */
int inputWaiting() {
	return waitForInput;
}

/**
 * @brief Function to notify the terminal to read a value for the read operation in the next prompt.
 *
 * @return Whether the terminal should read from the user
 */
int readFromUser() {
	return waitToRead;
}

/**
 * @brief Function that continues the execution of a command, that lacked some input (terminal has blocked).
 *
 * @param inputScript The next script entered afte the blocking one
 * @return Whether the command entered is a bash built-in command
 */
int continueBashExecution(char *inputScript) {
	int continueRsult = executeBashBuiltinFunction(commandWaiting, &inputScript,
			1);
	waitForInput = 0;
	commandWaiting = NULL;
	return continueRsult;
}

/**
 * @brief Function that escapes characters to be passed to system calls
 *
 * @param buffer A character sequence to be escaped
 * @return The escaped character sequence
 */
char* escape(char* buffer) {
	int i, j;
	int l = strlen(buffer) + 1;
	char esc_char[] = { '\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\' };
	char essc_str[] = { 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\' };
	char* dest = (char*) calloc(l * 2, sizeof(char));
	char* ptr = dest;
	for (i = 0; i < l; i++) {
		for (j = 0; j < 8; j++) {
			if (buffer[i] == esc_char[j]) {
				*ptr++ = '\\';
				*ptr++ = essc_str[j];
				break;
			}
		}
		if (j == 8)
			*ptr++ = buffer[i];
	}
	*ptr = '\0';
	return dest;
}

/**
 * @brief Function that creates a full bash command by concatenating the command name and its arguments.
 *
 * @param bashCommand The pure command name
 * @param commandArguments The command arguemnts given
 * @param beg Index to start concatenating arguments from
 * @param fin Index to stop concatenating arguments
 * @return A full system call to be executed
 */
char *concatenateArguments(char *bashCommand, char **commandArguments, int beg,
		int fin) {
	if (bashCommand == NULL )
		return NULL ;
	if (commandArguments == NULL )
		return bashCommand;
	if (beg > fin)
		return bashCommand;
	char *fullCommand = (char*) malloc(MAX_COMMAND_LENGTH * sizeof(char));
	if (fullCommand == NULL ) {
		perror("malloc error");
		return NULL ;
	}
	fullCommand = strcpy(fullCommand, escape(bashCommand));
	if (strcmp(bashCommand, "read") == 0){
		strcpy(fullCommand,"");
	}
	else{
		strcat(fullCommand, " ");
	}
	// No need of double quotes in case of a single argument
	if ((beg == 0) && (fin == 0)) {
		strcat(fullCommand, escape(commandArguments[beg]));
		return fullCommand;
	}
	// In case of more than one arguments
	strcat(fullCommand, escape(commandArguments[beg]));
	int i;
	for (i = beg + 1; i <= fin; i++) {
		strcat(fullCommand, " ");
		strcat(fullCommand, escape(commandArguments[i]));
	}
	return fullCommand;
}

/**
 * @brief Function that checks if the command is an environmental variable setting (e.g. PS1=TEST)
 *
 * @param commandName The command name to check
 * @return The index that the '=' delimiter is positioned / 0: If the command is not suck a statement
 */
int isEnvSet(char *commandName) {
	int comIndex = 0;
	while (comIndex < strlen(commandName)) {
		if (commandName[comIndex] == '=')
			return comIndex;
		comIndex++;
	}
	return 0;
}

/**
 *
 * Bash built-in functions implementation.
 *
 */

void executeDot(char *commandName, char **commandArguments, int args) {
	system(concatenateArguments(commandName, commandArguments, 0, args - 1));
}

void executeSource(char **commandArguments, int args) {
	system(concatenateArguments("source", commandArguments, 0, args - 1));
}

void executeCd(char **commandArguments, int args) {
	chdir(commandArguments[0]);
}

void executeDeclare(char **commandArguments, int args) {
	if (args == 0) {
		system("declare");
		return;
	}
	putenv(commandArguments[0]);
}

void executeTypeset(char **commandArguments, int args) {
	if (args == 0) {
		system("typeset");
		return;
	}
	putenv(commandArguments[0]);
}

void executeEcho(char **commandArguments, int args) {
	char *echoCommand = concatenateArguments("echo", commandArguments, 0,
			args - 1);
	system(echoCommand);
}

void executeExec(char **commandArguments, int args) {
	execvp(commandArguments[0], commandArguments);
}

void executeExit(char **commandArguments, int args) {
	exitEnabled = 1;
}

void executeExport(char **commandArguments, int args) {
	putenv(commandArguments[0]);
}

void executeHistory(char **commandArguments, int args) {
	// If too many arguments
	if (args > 1) {
		printf("nicpoyia-sh: history: too many arguments\n");
		return;
	}
	// If history command with a numeric argument
	int parValue;
	if ((args == 1) && (sscanf(commandArguments[0], "%d", &parValue) == 1)) {
		//
		//
		//
		return;
	}
	// If plain history command
	//
	//
	//
}

void executeKill(char **commandArguments, int args) {
	if (args == 0) {
		printf(
				"kill: usage: kill [-s sigspec | -n signum | -sigspec] pid | jobspec ... or kill -l [sigspec]\n");
		return;
	}
	// If no signal specified, send the default signal SIGTERM
	if (args == 1) {
		int pid = atoi(commandArguments[0]);
		kill(pid, SIGTERM);
		return;
	}
	// If a signal is specified, it should be like "kill -9 1234"
	if (commandArguments[0][0] != '-')
		printf(
				"kill: usage: kill [-s sigspec | -n signum | -sigspec] pid | jobspec ... or kill -l [sigspec]\n");
	else {
		// Ignore the '-' symbol attached in the front of the signal code given
		char *signalNumberString = commandArguments[0] + 1;
		// Get the signal code and thepid
		int signalCode = atoi(signalNumberString);
		int pid = atoi(commandArguments[1]);
		// Send the signal
		kill(pid, signalCode);
	}
}

/**
 * @brief Function that gets obtains a specific substring and returns a pointer to it.
 *
 * @param str The original string
 * @param begin The index within the string to begin getting characters from
 * @param len Length of substring
 * @return The substring produced
 */
char* subString(const char* str, size_t begin, size_t len) {
	if (str == 0 || strlen(str) == 0 || strlen(str) < begin
			|| strlen(str) < (begin + len))
		return 0;

	return strndup(str + begin, len);
}

/**
 * @brief Function that parses a let command to define its elements.
 * Fills in the two values variables and the operator character variable.
 *
 * @return 0: OK / -1: Error
 */
int parseLetCommand(const char *argument, char **variable, int *value1,
		int *value2, char *operator) {
	// Discover the operators indices within the character sequence
	int argIndex = 0;
	int equalIndex = 0;
	int operatorIndex = 0;
	while (argIndex < strlen(argument)) {
		switch (argument[argIndex]) {
		case '=':
			equalIndex = argIndex;
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			operatorIndex = argIndex;
			break;
		}
		argIndex++;
	}
	// If a parser error occurred
	if ((equalIndex == 0) || (operatorIndex == 0))
		return -1;
	if (abs(equalIndex - operatorIndex) <= 1)
		return -1;
	// Get the values and the operator using the discovered indices
	(*operator) = argument[operatorIndex];
	char *value1String;
	char *value2String;
	value1String = subString(argument, equalIndex + 1,
			operatorIndex - equalIndex - 1);
	(*value1) = atoi(value1String);
	value2String = subString(argument, operatorIndex + 1,
			strlen(argument) - operatorIndex - 1);
	(*value2) = atoi(value2String);
	(*variable) = subString(argument, 0, equalIndex);
	return 0;
}

void executeLet(char **commandArguments, int args) {
	if (args == 0) {
		printf("nicpoyia-sh: let: expression expected\n");
		return;
	}
	char *variableName;
	int value1;
	int value2;
	char operator;
	if (parseLetCommand(commandArguments[0], &variableName, &value1, &value2,
			&operator) == -1)
		return;
	int result = 0;
	switch (operator) {
	case '+':
		result = value1 + value2;
		break;
	case '-':
		result = value1 - value2;
		break;
	case '*':
		result = value1 * value2;
		break;
	case '/':
		result = value1 / value2;
		break;
	case '%':
		result = value1 % value2;
		break;
	}
	// Add or replace the evaluated expression into the desired variable
	char resultString[16];
	sprintf(resultString, "%d", result);
	setenv(variableName, resultString, 1);
}

void executeLocal(char **commandArguments, int args) {
	if (args == 0) {
		waitForInput = 1;
		commandWaiting = "local";
		return;
	}
	char *localCommand = concatenateArguments("local", commandArguments, 0,
			args - 1);
	system(localCommand);
}

void executeLogout(char **commandArguments, int args) {
	// If the logout fail, then print the ucush message to prompt the user to use the exit command
	// If the logout succeed, then no message is printed, and the user is logged out.
	// This should be allowed only if the running instance of nicpoyia-sh is controlled over an SSH connection,
	// or other means of login procedure.
	if (system("logout 2> /dev/null"))
		printf("nicpoyia-sh: logout: not login shell: use `exit'\n");
}

void executePwd(char **commandArguments, int args) {
	char workingDirectory[MAX_DIR_LENGTH];
	if (getcwd(workingDirectory, MAX_DIR_LENGTH) != NULL )
		printf("%s\n", workingDirectory);
	else
		perror("pwd error:");
}

void executeRead(char **commandArguments, int args) {
	// If the command is going to read a variable without using an option
	if (waitToRead) {
		setenv(variableToRead, commandArguments[0], 1);
		waitToRead = 0;
		free(variableToRead);
		variableToRead = NULL;
		return;
	}
	// If not input is given, just the command
	if (args == 0) {
		waitForInput = 1;
		commandWaiting = "read";
		return;
	}
	// If only an option selector has passed as argument
	if ((args == 2) && (commandArguments[1][0] == '-')) {
		printf(
				"read: usage: read [-ers] [-a array] [-d delim] [-i text] [-n nchars] [-N nchars] [-p prompt] [-t timeout] [-u fd] [name ...]\n");
		return;
	}
	if (args >= 1) {
		// If the command is going to read
		if (!waitForInput) {
			// Argument index of the variable specified for reading, -1 if not specified.
			int variableIndex = -1;
			int i;
			for (i = 0; i < args; i++) {
				if ((commandArguments[i][0] != '-')
						&& (commandArguments[i][0] != '"'))
					variableIndex = i;
			}
			if (variableIndex >= 0) {
				// Check and parse the option selector tag
				if (variableIndex >= 2) {
					if (strlen(commandArguments[0]) == 2) {
						switch (commandArguments[0][1]) {
						// -p option selector tag
						case 'p':
							if ((variableIndex - 1) > 0) {
								// Remove quotes (the characters are escaped)
								// Do not use more that one arguments in case quotes not used properly
								(commandArguments[1])++;
								commandArguments[variableIndex - 1][strlen(
										commandArguments[args - 1]) - 1] = '\0';
								// Print user-selected prompt message
								printf("%s\n",
										concatenateArguments("read",
												commandArguments, 1,
												(variableIndex - 1)));
							}
							break;
							//
							// OTHER OPTIONS...
							//
							//
						}
					}
				}
				// Execute the read procedure
				variableToRead = (char*) malloc(
						strlen(commandArguments[variableIndex] + 1)
								* sizeof(char));
				if (variableToRead == NULL ) {
					perror("malloc error");
					return;
				}
				strcpy(variableToRead, commandArguments[variableIndex]);
				waitToRead = 1;
				commandWaiting = "read";
				return;
			}
		}
	}
}

void executeClear(char **commandArguments, int args) {
	system("clear");
}

void executeSetEnv(char *commandName) {
	putenv(commandName);
}

/**
 * @brief Function that checks whether a command represents a bash built-in function.
 * If it is a bash built-in function, then it is executed.
 *
 * @param commandName The pure command name
 * @param commandArguments commandArguments Arguments array
 * @param args Number of arguments passed
 * @return Error code:
 * 		 0: If the command is not a bash built-in function
 * 		 1: If the command is a bash built-in function and it has been executed.
 * 		-1: If an error occurred.
 */
int executeBashBuiltinFunction(char *commandName, char **commandArguments,
		int args) {
	if (commandName[0] == '.') {
		executeDot(commandName, commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "source") == 0) {
		executeSource(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "cd") == 0) {
		executeCd(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "declare") == 0) {
		executeDeclare(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "typeset") == 0) {
		executeTypeset(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "echo") == 0) {
		executeEcho(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "exec") == 0) {
		if (args == 0)
			return -1;
		executeExec(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "exit") == 0) {
		executeExit(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "export") == 0) {
		if (args == 0)
			return -1;
		executeExport(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "history") == 0) {
		executeHistory(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "kill") == 0) {
		executeKill(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "let") == 0) {
		executeLet(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "local") == 0) {
		executeLocal(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "logout") == 0) {
		executeLogout(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "pwd") == 0) {
		executePwd(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "read") == 0) {
		executeRead(commandArguments, args);
		return 1;
	}
	if (strcmp(commandName, "clear") == 0) {
		executeClear(commandArguments, args);
		return 1;
	}
	int envDelPos;
	if ((envDelPos = isEnvSet(commandName)) > 0) {
		executeSetEnv(commandName);
		return 1;
	}
	return 0;
}
