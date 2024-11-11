//Ayush Singh
//OS assignmnt 3
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h> //opening creating and truncation to
#include <time.h>


int  parseCommands(char* args);
void execute_timedatectl();
void fork_c_process();
void p_processes_p(pid_t c_p_id);
void leave();
void enter_directory();
void status_of_pro(int* errorSignal);
void otherCommands(int* errorSignal);
void handle_SIGTSTP();
void else_command_block();

#define MAXIMUM 	512		
#define CMD	2048	
#define PROCESSES	1000	


int   number_of_arguments = 0;				
char* argument_list[MAXIMUM];	
int   allowBackground = 1;		
int   background_process = 0;			
char  currentDir[100];			
int   processes[PROCESSES];	
int   numProcesses = 0;			
int   processStatus;



struct sigaction SIGINTAction;	
struct sigaction SIGTSTPAction; 

int main() {
	char string_argument_number[2048];						// String of all arguemnts
    //used geeks for geeks on this to handle when user presses control c or Z
    // CTRL-C
    //sets the signal handler for SIGINT to SIG_IGN, which means the signal is ignored. 
    //This is a common way to prevent the default action of terminating the program when the user presses CTRL-C
    SIGINTAction.sa_handler=SIG_IGN;			
    sigfillset(&SIGINTAction.sa_mask); 			
    sigaction(SIGINT, &SIGINTAction, NULL);		

    // CTRL-Z
    //The SIGTSTP signal is configured to be handled by the function handle_SIGTSTP. Additionally, the SA_RESTART 
    //flag is set, which means that certain system calls interrupted by 
    //SIGTSTP will be restarted rather than returning an error.
	SIGTSTPAction.sa_handler = handle_SIGTSTP; 	
    SIGTSTPAction.sa_flags = SA_RESTART; 		
    sigfillset(&SIGTSTPAction.sa_mask);			
    sigaction(SIGTSTP, &SIGTSTPAction, NULL);	
	
	// Loop to continuously read the lines of shell
	while(1) {
		number_of_arguments = parseCommands(string_argument_number);		
		argument_list[number_of_arguments] = NULL; 				
		else_command_block();						
	}
	return 0;
}

int parseCommands(char* commandArgs) {
    int i, numArgs = 0;
    char tempString[CMD];

    // Print prompt
    printf(": ");
    fflush(stdout);

    // Read user input into commandArgs
    fgets(commandArgs, CMD, stdin);
    strtok(commandArgs, "\n"); // Remove trailing newline

    // Get first of argument
    char* pass = strtok(commandArgs, " ");

    // Get the rest of the tokens
    while (pass != NULL) {
        // Write argument to global variable
        argument_list[numArgs] = pass;

        // Handle the expansion of variable $$
        for (i = 1; i < strlen(argument_list[numArgs]); i++) {
            if (argument_list[numArgs][i] == '$' && argument_list[numArgs][i - 1] == '$') {
                // Replace $$ with NULL
                argument_list[numArgs][i] = '\0';
                argument_list[numArgs][i - 1] = '\0';

                // Insert pid
                snprintf(tempString, CMD, "%s%d", argument_list[numArgs], getpid());
                argument_list[numArgs] = tempString;
            }
        }

        pass = strtok(NULL, " "); // Get next token
        numArgs++;
    }

    return numArgs;
}

void handle_SIGTSTP() {
    char* f_message;
    int size;

    if (allowBackground == 0) {
        f_message = "\nExiting foreground-only mode\n";
        size = 100;
        allowBackground = 1;
    } else if (allowBackground == 1) {
        f_message = "\nEntering foreground-only mode (& is now ignored)\n";
        size = 100;
        allowBackground = 0;
    } else {
        f_message = "\nError: allowBackground is not 0 or 1\n";
        size = 100;
        allowBackground = 1;
    }

    write(STDOUT_FILENO, f_message, size);
    write(STDOUT_FILENO, ": ", 2);
}


void else_command_block() {
	int errorSignal = 0;	// 1 if status_call() was already called

	if(argument_list[0][0] == '#' || argument_list[0][0] == '\n') {
		// Ignore comments and empty lines
	}
	else if(strcmp(argument_list[0], "exit") == 0) {
		leave(); //exit call to leave actual program
	}
	else if(strcmp(argument_list[0], "cd") == 0) {
		enter_directory(); //cd call to enter directory
	}
	else if(strcmp(argument_list[0], "status") == 0) {
		status_of_pro(&errorSignal); //displlay the status of the thing
	}
	else if (strcmp(argument_list[0], "/bin/timedatectl") == 0) {
        execute_timedatectl(); //this doesnt work had to change it up
    }
	else {
		otherCommands(&errorSignal);

		// Print out the status if it hasn't been already printed
		if(WIFSIGNALED(processStatus) && errorSignal == 0){ 
	        status_of_pro(&errorSignal); 
	    }
	}
}


void leave() {
	// Exit the program without terminating any processes
	if(numProcesses == 0)
		exit(0);
	// There are processes that must be terminated one by one
	else{
		int i;
		for(i = 0; i < numProcesses; i++) 
			kill(processes[i], SIGTERM);
		exit(1);
	}
}

void enter_directory() {
    int error = 0;

    // Change directory based on the number of arguments
    if (number_of_arguments == 1) {
        // No argument provided, change directory to home dir
        error = chdir(getenv("HOME"));
    } else if (number_of_arguments == 2) {
        // Argument provided, navigate to the specified directory
        error = chdir(argument_list[1]);
    } else {
        // Invalid number of arguments
        printf("Usage: cd [directory]\n");
        fflush(stdout);
        return;
    }

    // Check for chdir success
    if (error == 0) {
        // Print the current working directory
        printf("%s\n", getcwd(currentDir, 100));
    } else {
        // Print an error message if chdir failed
        perror(" Sorry ");
    }

    fflush(stdout);
}

void status_of_pro(int* errorSignal) {
	int errHold = 0, sigHold = 0, exitValue;

	waitpid(getpid(), &processStatus, 0);		// Check the status of the last process

	if(WIFEXITED(processStatus)) 
        errHold = WEXITSTATUS(processStatus);	// Return the status of the normally terminated child

    if(WIFSIGNALED(processStatus)) 
        sigHold = WTERMSIG(processStatus);		// Return the status of an abnormally terminated child

    exitValue = errHold + sigHold == 0 ? 0 : 1;

    if(sigHold == 0) 
    	printf("exit value %d\n", exitValue);
    else {
    	*errorSignal = 1;
    	printf("terminated by signal %d\n", sigHold);
    }
    fflush(stdout);
}

void otherCommands(int* errorSignal) { 
    background_process = 0;

    // When there is a &, set this process to be in the background otherwise ignore
    if (strcmp(argument_list[number_of_arguments - 1], "&") == 0) {
        // Only make it a background process if not in foreground-only mode
        if (allowBackground == 1)
           background_process = 1;
        // Ignore the argument for later
        argument_list[number_of_arguments - 1] = NULL;
    }
    pid_t pid;  
    pid = fork();                   // Start process
    processes[numProcesses] = pid;  // Save process pid
    numProcesses++;
    switch (pid) {
        case -1: // Error
            perror("fork() failed\n");
            exit(1);
            break;
        case 0:  // Child
            // Check if the command is /bin/timedatectl
            if (strcmp(argument_list[0], "/bin/timedatectl") == 0) {
                // Execute /bin/timedatectl
                execl("/bin/timedatectl", "timedatectl", (char *)NULL);
                perror("execl failed\n");
                exit(1);
            } else {
                // Execute other commands
               fork_c_process();
            }
            break;

        default: // Parent
           p_processes_p(pid);
    }
    // Wait for the child to finish
    while ((pid = waitpid(-1, &processStatus, WNOHANG)) > 0) {
        printf("background pid %d is done: ", pid);
        fflush(stdout);
        status_of_pro(errorSignal);
    }
}





void fork_c_process() {
	int i, haveInputFile = 0, haveOutputFile = 0;
	char inputFile[CMD], outputFile[CMD];
	// Get command arguments
	for(i = 0; argument_list[i] != NULL; i++) {
		// Input File Arguments
		if(strcmp(argument_list[i], "<") == 0) {
			haveInputFile = 1;
			argument_list[i] = NULL;
			strcpy(inputFile, argument_list[i+1]);
			i++;
		}
		// Output file Arugments
		else if(strcmp(argument_list[i], ">") == 0) {
			haveOutputFile = 1;
			argument_list[i] = NULL;
			strcpy(outputFile, argument_list[i+1]);
			i++;
		}
	}
	// Pass input file info
	if(haveInputFile) {
		int inputFileDes = 0;
		if ((inputFileDes = open(inputFile, O_RDONLY)) < 0) { 
            fprintf(stderr, "cannot open %s for input\n", inputFile);
            fflush(stdout); 
            exit(1); 
        }  
        dup2(inputFileDes, 0);
        close(inputFileDes);
	}
	// Pass output file info
	if(haveOutputFile) {
		int outputFileDes = 0;
		if((outputFileDes = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
			fprintf(stderr, "cannot open %s for output\n", outputFile);
			fflush(stdout); 
			exit(1); 
		}
		dup2(outputFileDes, 1);
        close(outputFileDes);
	}
	// Give new CTRL-C handler so foreground processes can be terminated
	if(!background_process) 
		SIGINTAction.sa_handler=SIG_DFL;
	sigaction(SIGINT, &SIGINTAction, NULL);
	// Run the non-built-in command
	if(execvp(argument_list[0], argument_list) == -1 ) {
        perror(argument_list[0]);
        exit(1); 
    }
}


void p_processes_p(pid_t c_p_id) {
	if(background_process == 1) {
		waitpid(c_p_id, &processStatus, WNOHANG);
		printf("background pid is %d\n", c_p_id);
		fflush(stdout); 
	}
	else {
		waitpid(c_p_id, &processStatus, 0);
	}
}


void execute_timedatectl() {
    pid_t pid = fork();

    switch (pid) {
        case -1:
            perror("fork() failed\n");
            exit(1);
            break;

        case 0:  // Child
            execl("/bin/timedatectl", "timedatectl", (char *)NULL);
            perror("execl failed\n");
            exit(1);
            break;

        default: // Parent
            waitpid(pid, &processStatus, 0);
            break;
    }
}


