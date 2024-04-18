//P2-SSOO-23/24

//  MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>			/* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdbool.h>

#include <sys/wait.h>
#include <signal.h>

#define MAX_COMMANDS 8

// files in case of redirection
char filev[3][64];

// to store the execvp second parameter
char *argv_execvp[8];

// CTRL + C to exit 
void siginthandler(int param)
{
	printf("****  Exiting MSH **** \n");
	//signal(SIGINT, siginthandler);
	exit(0);
}

// functions:
// 1. and 2. Execution of simple commands and if in background
void execute_single_command(char ***argvv, int in_background);

// 3. Execution of sequences of commands connected through pipes without redirection
void execute_command_sequence(char ****argvv, int num_commands);

// 4.1 Execution of simple commands with redirections
void execute_single_command_redirection(char ***argvv, char filev[3][64], int in_background);

// 4.2 Execution of sequences of commands connected through pipes with redirection
void execute_command_sequence_with_redirection(char ****argvv, char filev[3][64], int num_commands);

// file redirections
void redirect_file(char *input_file, char *output_file, char *error_file);

// 5.1 mycalc function
int acc = 0; // Variable global para mantener el acumulador para la suma.
void mycalc(char **args);

 // 5.2 myhistory function
void myhistory(char **args);


// command structure
struct command
{
  // Store the number of commands in argvv
  int num_commands;
  // Store the number of arguments of each command
  int *args;
  // Store the commands
  char ***argvv;
  // Store the I/O redirection
  char filev[3][64];
  // Store if the command is executed in background or foreground
  int in_background;
};

int history_size = 20;

// a pointer named history that is intended to point to an array or a single instance of struct command
struct command * history;

// linear buffer
int head = 0;
int tail = 0;

int n_elem = 0;

// free_command function
void free_command(struct command *cmd)
{
    if((*cmd).argvv != NULL)
    {
        char **argv;
        for (; (*cmd).argvv && *(*cmd).argvv; (*cmd).argvv++)
        {
            for (argv = *(*cmd).argvv; argv && *argv; argv++)
            {
                if(*argv){
                    free(*argv);
                    *argv = NULL;
                }
            }
        }
    }
    free((*cmd).args);
}

void store_command(char ***argvv, char filev[3][64], int in_background, struct command* cmd)
{
    int num_commands = 0;
    while(argvv[num_commands] != NULL){
        num_commands++;
    }

    for(int f=0;f < 3; f++)
    {
        if(strcmp(filev[f], "0") != 0)
        {
            strcpy((*cmd).filev[f], filev[f]);
        }
        else{
            strcpy((*cmd).filev[f], "0");
        }
    }

    (*cmd).in_background = in_background;
    (*cmd).num_commands = num_commands-1;
    (*cmd).argvv = (char ***) calloc((num_commands) ,sizeof(char **));
    (*cmd).args = (int*) calloc(num_commands , sizeof(int));

    for( int i = 0; i < num_commands; i++)
    {
        int args= 0;
        while( argvv[i][args] != NULL ){
            args++;
        }
        (*cmd).args[i] = args;
        (*cmd).argvv[i] = (char **) calloc((args+1) ,sizeof(char *));
        int j;
        for (j=0; j<args; j++)
        {
            (*cmd).argvv[i][j] = (char *)calloc(strlen(argvv[i][j]),sizeof(char));
            strcpy((*cmd).argvv[i][j], argvv[i][j] );
        }
    }
}


/**
 * Get the command with its parameters for execvp
 * Execute this instruction before run an execvp to obtain the complete command
 * @param argvv
 * @param num_command
 * @return
 */
void getCompleteCommand(char*** argvv, int num_command) {
	//reset first
	for(int j = 0; j < 8; j++)
		argv_execvp[j] = NULL;

	int i = 0;
	for ( i = 0; argvv[num_command][i] != NULL; i++)
		argv_execvp[i] = argvv[num_command][i];
}


/**
 * Main sheell  Loop  
 */
int main(int argc, char* argv[])
{
	/**** Do not delete this code.****/
	int end = 0; 
	int executed_cmd_lines = -1;
	char *cmd_line = NULL;
	char *cmd_lines[10];

    // isatty function call checks if the standard input is a terminal
    // FALSE -> redirection or pipe
	if (!isatty(STDIN_FILENO)) {
		cmd_line = (char*)malloc(100);
		while (scanf(" %[^\n]", cmd_line) != EOF){
			if(strlen(cmd_line) <= 0) return 0;
			cmd_lines[end] = (char*)malloc(strlen(cmd_line)+1);
			strcpy(cmd_lines[end], cmd_line);
			end++;
            // clear (flush) the output buffer and move buffered data to the console or the respective file
			fflush (stdin);
			fflush(stdout);
		}
	}

	/*********************************/

	char ***argvv = NULL;
	int num_commands;

    // history_size = 20
    // allocating the sizeof command
	history = (struct command*) malloc(history_size *sizeof(struct command));
	int run_history = 0;

	while (1) 
	{
		int status = 0;
		int command_counter = 0;
		int in_background = 0;
		signal(SIGINT, siginthandler);

		if (run_history)
        {
            run_history=0;
        }
        else{
            // Prompt 
            write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

            // Get command
            //********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
            executed_cmd_lines++;
            if( end != 0 && executed_cmd_lines < end) {
                command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
            }
            else if( end != 0 && executed_cmd_lines == end)
                return 0;
            else
                command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
		//************************************************************************************************

		/************************ STUDENTS CODE ********************************/

        // Check for mycalc and myhistory functions
        if (strcmp(argvv[0][0], "mycalc") == 0) {
            // 5.1 mycalc function
            mycalc(argvv[0]);
            continue;
        } else if (strcmp(argvv[0][0], "myhistory") == 0) {
            // 5.2 myhistory function
            myhistory(argvv[0]);
            continue;
        } else if (strcmp(argvv[0][0], "exit") == 0) {
            break;
        }

	    if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
            else {
                // Print command
                store_command(argvv, filev, in_background, &history[tail]);
                tail = (tail + 1) % history_size;
                if (n_elem < history_size) 
                {
                    n_elem++;
                } else {
                    head = (head + 1) % history_size; // Avanzar la cabeza si el buffer está lleno
                }
                // print_command(argvv, filev, in_background);
            }
        }
        
        // Check if there's any redirection specified
        bool redirection = false;
        for (int i = 0; i < 3; i++) {
            // If the file is not empty and not "0"
            if (filev[i][0] != '\0' && strcmp(filev[i], "0") != 0) 
            {
                redirection = true;
                break;
            }
        }
        

        // 1. Execution of simple commands
		// if there is only one single command, no pipes
        if (1 == command_counter)
        {
            // 4.1 Execution of simple commands with redirections
            if (redirection)
            {
                execute_single_command_redirection(argvv, filev, in_background);
            }
            // 1. and 2. Execution of simple commands and if in background
            else 
            {
                execute_single_command(argvv, in_background);
            }
        }
        // 3. Execution of sequences of commands connected through pipes
        else
        {
            // 4.2 Execution of sequences of commands connected through pipes with redirection
            if (redirection) 
            {
                execute_command_sequence_with_redirection(&argvv, filev, command_counter);
            }
            // 3. Execution of sequences of commands connected through pipes without redirection
            else 
            {
                execute_command_sequence(&argvv, command_counter);
            }
        }
                
    }
    for (int i = 0; i < history_size; i++) {
        free_command(&history[i]);
    }
    free(history);
	return 0;
};


// functions:
// 1. Execution of simple commands
void execute_single_command(char ***argvv, int in_background) {
    pid_t pid = fork();
    
    // Child process
    if (pid == 0) 
    {              
        // Execute the command       
        execvp(argvv[0][0], argvv[0]);

        // If execvp returns, an error occurred
        perror("execvp error");
        exit(EXIT_FAILURE);
    }
    // Parent process
    else if (pid > 0) 
    { 
        if (!in_background) 
        {
            // Wait for the child process to finish
            waitpid(pid, NULL, 0);
        }
        else 
        {
            // 2. single command in background running (parent) 
            printf("Process %d running in background\n", pid);
        }
    }
    // Handle fork failure
    else 
    {
        perror("fork error");
    }
}

// 4.1 Execution of simple commands with redirections
void execute_single_command_redirection(char ***argvv, char filev[3][64], int in_background) {
    pid_t pid = fork();
    
    // Child process
    if (pid == 0) 
    { 
        // Apply redirections
        if (filev[0][0] != '\0' && strcmp(filev[0], "0") != 0) {
            redirect_file(filev[0], NULL, NULL);
        }
        if (filev[1][0] != '\0' && strcmp(filev[1], "0") != 0) {
            redirect_file(NULL, filev[1], NULL);
        }
        if (filev[2][0] != '\0' && strcmp(filev[2], "0") != 0) {
            redirect_file(NULL, NULL, filev[2]);
        }
        // Execute the command       
        execvp(argvv[0][0], argvv[0]);
        // If execvp returns, an error occurred
        perror("execvp error");
        exit(EXIT_FAILURE);
    }
    // Parent process
    else if (pid > 0) 
    { 
        if (!in_background) 
        {
            // Wait for the child process to finish
            waitpid(pid, NULL, 0);
        }
        else 
        {
            // 2. single command in background running (parent) 
            printf("Process %d running in background\n", pid);
        }
    } 
    else 
    {
        // Handle fork failure
        perror("fork error");
    }
}


// 3. Execution of sequences of commands connected through pipes
void execute_command_sequence(char ****argvv, int num_commands) {
    int num_pipes = num_commands - 1;
    int pipe_fds[2 * num_pipes];

    // Create all necessary pipes
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fds + i * 2) == -1) 
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Execute each command
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        // Child process
        if (pid == 0) 
        { 
            // If not the first command, get input from the previous pipe
            if (i != 0) 
            {
                if (dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO) == -1) 
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            // If not the last command, output to the next pipe
            if (i != num_commands - 1) 
            {
                if (dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO) == -1) 
                {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipes (prevents hanging)
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fds[j]);
            }

            // Execute the command
            execvp((*argvv)[i][0], (*argvv)[i]);
            // If execvp returns, an error occurred
            perror("execvp");
            exit(EXIT_FAILURE);
        } 
        else if (pid < 0) 
        {
            // Handle fork failure
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Parent closes all pipes
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipe_fds[i]);
    }

    // Parent waits for all child processes to complete
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}


// redirections
void redirect_file(char *input_file, char *output_file, char *error_file) {
    // input file redirection
    if (input_file && *input_file) 
    {
        // read only
        int fd = open(input_file, O_RDONLY);
        if (fd == -1) 
        {
            perror("Failed to open input file");
            exit(EXIT_FAILURE);
        }
        // dup2() duplicates the file descriptor fd to the file descriptor STDIN_FILENO
        if (dup2(fd, STDIN_FILENO) == -1) 
        {
            perror("Failed to redirect standard input");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    // output file redirection
    if (output_file && *output_file) 
    {
        // write only, create, truncate
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) 
        {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }
        // dup2() duplicates the file descriptor fd to the file descriptor STDOUT_FILENO
        if (dup2(fd, STDOUT_FILENO) == -1) 
        {
            perror("Failed to redirect standard output");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
    // error file redirection
    if (error_file && *error_file) 
    {
        // write only, create, truncate
        int fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) 
        {
            perror("Failed to open error file");
            exit(EXIT_FAILURE);
        }
        // dup2() duplicates the file descriptor fd to the file descriptor STDERR_FILENO
        if (dup2(fd, STDERR_FILENO) == -1) 
        {
            perror("Failed to redirect standard error");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
}


// 3. Execution of sequences of commands connected through pipes with redirection
// Execute a sequence of commands with redirection and pipes
void execute_command_sequence_with_redirection(char ****argvv, char filev[3][64], int num_commands) {
    int num_pipes = num_commands - 1;
    // pipe_fds array to store all pipe file descriptors
    int pipe_fds[2 * num_pipes];

    // Create all necessary pipes
    for (int i = 0; i < num_pipes; i++) {
        // pipe() creates a pipe and returns two file descriptors
        if (pipe(pipe_fds + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Execute each command
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        // Child process
        if (pid == 0) 
        {
            // Redirect input if not the first command
            if (i != 0) 
            {
                // dup2() duplicates the file descriptor pipe_fds[(i - 1) * 2] to the file descriptor STDIN_FILENO
                dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO);
            }
            // Input redirection for the first command
            // If the input file is not empty and not "0"
            else if (filev[0][0] != '\0' && strcmp(filev[0], "0") != 0) {
                // Redirect input
                redirect_file(filev[0], NULL, NULL);
            }

            // Redirect output if not the last command
            if (i != num_commands - 1) 
            {
                // dup2() duplicates the file descriptor pipe_fds[i * 2 + 1] to the file descriptor STDOUT_FILENO
                dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO);
            }
            // Output redirection for the last command
            // If the output file is not empty and not "0"
            else if (filev[1][0] != '\0' && strcmp(filev[1], "0") != 0) {
                // Redirect output
                redirect_file(NULL, filev[1], NULL);
            }

            // Error redirection for each command if specified
            // If the error file is not empty and not "0"
            if (filev[2][0] != '\0' && strcmp(filev[2], "0") != 0) 
            {
                // Redirect error
                redirect_file(NULL, NULL, filev[2]);
            }

            // Close all pipes in the child to prevent hanging
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fds[j]);
            }

            // Execute the command
            execvp((*argvv)[i][0], (*argvv)[i]);
            // If execvp returns, an error occurred
            perror("execvp");
            exit(EXIT_FAILURE);
        } 
        else if (pid < 0) {
            // Handle fork failure
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }

    // Close all pipes in the parent
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipe_fds[i]);
    }

    // Wait for all child processes to complete
    for (int i = 0; i < num_commands; i++) {
        wait(NULL);
    }
}



// 5.1 mycalc function
void mycalc(char **args) {
    // Check if the command has the correct number of arguments
    if (!args[1] || !args[2] || !args[3]) {
        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
        return;
    }

    // Parse the arguments op1, operator, and op2
    int op1 = atoi(args[1]);
    int op2 = atoi(args[3]);
    char *operator = args[2];
    int result, remainder;

    // Read the current value of acc from the environment variable
    char *acc_env = getenv("Acc");
    int acc = (acc_env) ? atoi(acc_env) : 0;

    // Perform the operation
    if (strcmp(operator, "add") == 0) {
        result = op1 + op2; // Perform the addition
        acc += result; // Update the acc variable
    } else if (strcmp(operator, "mul") == 0) {
        result = op1 * op2; // Perform the multiplication
    } else if (strcmp(operator, "div") == 0) {
        // Not allowed to divide by zero
        if (op2 == 0) {
            printf("[ERROR] Division by zero is not allowed.\n");
            return;
        }
        result = op1 / op2; // Perform the division
        remainder = op1 % op2; // Calculate the remainder
        fprintf(stderr, "[OK] %d / %d = %d; Remainder %d\n", op1, op2, result, remainder); // Print the result of the division
        return;
    } else {
        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n"); // Print an error message if the operator is not valid
        return;
    }

    // Update the Acc environment variable with the new value
    char acc_str[20];
    sprintf(acc_str, "%d", acc); // Convert acc to string
    setenv("Acc", acc_str, 1); // Set the acc enviroment variable

    // Print the result
    if (strcmp(operator, "add") == 0){
        fprintf(stderr, "[OK] %d + %d = %d; Acc %d\n", op1,  op2, result, acc);
    } else{
        fprintf(stderr, "[OK] %d * %d = %d\n", op1, op2, result);

    }
}


// 5.2 myhistory function
void myhistory(char **args) {
    // Check if the command has one argument
    if (args[1] == NULL) {
        int start = n_elem < history_size ? 0 : head;
        int count = 0;
        // Print the last 20 commands in the history
        for (int i = start; i < start + n_elem && count < history_size; i++, count++) {
            int index = i % history_size; // Get the index of the command
            fprintf(stderr, "%d ", count); // Print the index
            // Print the command with its arguments
            for (int j = 0; j < history[index].num_commands; j++) {
                for (int k = 0; k < history[index].args[j]; k++) {
                    fprintf(stderr, "%s ", history[index].argvv[j][k]); // Print the argument
                }
                if (j < history[index].num_commands - 1) {
                    fprintf(stderr, "| "); // Print the pipe symbol if there are more commands
                }
            }
            fprintf(stderr, "\n");
        }
    } else {
        // Execute a command from the historial
        int index = atoi(args[1]); // Get the index of the command
        if (index < 0 || index >= n_elem || index >= history_size) {
            fprintf(stdout, "ERROR: Command not found\n"); // Print an error message if the index is out of bounds
        } else {
            int real_index = (head + index) % history_size; // Get the real index of the command
            fprintf(stderr, "Running command %d\n", index); // Print the index of the command being executed
            
            pid_t pid = fork();
            if (pid == 0) { // Child process
                execvp(history[real_index].argvv[0][0], history[real_index].argvv[0]); // Execute the command from the history of the index
                perror("execvp failed");
                exit(EXIT_FAILURE); // Exit if excvp fails
            } else if (pid > 0) { // Parent process
                wait(NULL); // Wait until child ends
            } else {
                perror("fork failed");
            }
        }
    }
}