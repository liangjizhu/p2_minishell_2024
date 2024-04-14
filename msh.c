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



void execute_command_sequence(char ****argvv, int num_commands);
void redirect_io(char *input_file, char *output_file, char *error_file);

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
 // myhistory function
void myhistory(char **args) {
    if (args[1] == NULL) {
        // No se proporcionó un argumento, imprimir historial
        int start = n_elem < history_size ? 0 : head;
        int count = 0;
        fprintf(stderr, "History of commands:\n");
        for (int i = start; i < start + n_elem && count < history_size; i++, count++) {
            int index = i % history_size;
            fprintf(stderr, "%d ", count);
            for (int j = 0; j < history[index].num_commands; j++) {
                for (int k = 0; k < history[index].args[j]; k++) {
                    fprintf(stderr, "%s ", history[index].argvv[j][k]);
                }
                if (j < history[index].num_commands - 1) {
                    fprintf(stderr, "| ");
                }
            }
            fprintf(stderr, "\n");
        }
    } else {
        // Se proporcionó un argumento, intentar ejecutar ese comando del historial
        int index = atoi(args[1]);
        if (index < 0 || index >= n_elem || index >= history_size) {
            fprintf(stdout, "ERROR: Command not found\n");
        } else {
            int real_index = (head + index) % history_size;
            fprintf(stderr, "Running command %d\n", index);
            // Re-ejecutar el comando como si fuera recién introducido
            execvp(history[real_index].argvv[0][0], history[real_index].argvv[0]);
            perror("execvp failed");
        }
    }
}


// mycalc function
int acc = 0; // Variable global para mantener el acumulador para la suma.
void mycalc(char **args) {
    if (!args[1] || !args[2] || !args[3]) {
        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
        return;
    }

    int op1 = atoi(args[1]);
    int op2 = atoi(args[3]);
    char *operator = args[2];
    int result, remainder;

    if (strcmp(operator, "add") == 0) {
        result = op1 + op2;
        acc += result;
        fprintf(stderr, "[OK] %d + %d = %d; Acc %d\n", op1, op2, result, acc);
    } else if (strcmp(operator, "mul") == 0) {
        result = op1 * op2;
        fprintf(stderr, "[OK] %d * %d = %d\n", op1, op2, result);
    } else if (strcmp(operator, "div") == 0) {
        if (op2 == 0) {
            printf("[ERROR] Division by zero is not allowed.\n");
            return;
        }
        result = op1 / op2;
        remainder = op1 % op2;
        fprintf(stderr, "[OK] %d / %d = %d; Remainder %d\n", op1, op2, result, remainder);
    } else {
        printf("[ERROR] The structure of the command is mycalc <operand_1> <add/mul/div> <operand_2>\n");
    }
}


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


        if (strcmp(argvv[0][0], "mycalc") == 0) {
            mycalc(argvv[0]); // Paso el primer arreglo de argumentos a mycalc
            continue;
        } else if (strcmp(argvv[0][0], "myhistory") == 0) {
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
                if (n_elem < history_size) {
                    n_elem++;
                } else {
                    head = (head + 1) % history_size; // Avanzar la cabeza si el buffer está lleno
                }
                //print_command(argvv, filev, in_background);
            }
        }

        

        // 1. Execution of simple commands
								// if there is only one single command, no pipes
        if (1 == command_counter)
        {  
            pid_t pid = fork();
            // Child process
            if (pid == 0) {
                // 4. Execution of simple commands and sequence of commands with redirections (input, output
                // and error) and in background.         
                // Execute the command*/
                if (filev[0][0] != '\0' && strcmp(filev[0], "0") != 0) {
                    redirect_io(filev[0], NULL, NULL);
                }
                if (filev[1][0] != '\0' && strcmp(filev[1], "0") != 0) {
                    redirect_io(NULL, filev[1], NULL);
                }
                if (filev[2][0] != '\0' && strcmp(filev[2], "0") != 0) {
                    redirect_io(NULL, NULL, filev[2]);
                }
                execvp(argvv[0][0], argvv[0]);
                // If execvp returns, an error occurred
                perror("execvp error");
                exit(EXIT_FAILURE);
            }
            // Parent process
            else if (pid > 0) { 
                if (!in_background) {
                    // Wait for the child process to finish
                    waitpid(pid, NULL, 0);
                }
                else {
                    // 2. single command in background running (parent) 
                    printf("Process %d running in background\n", pid);
                }
            } 
            else {
                // Handle fork failure
                perror("fork error");
            }
        }
        else{
            // 3. Execution of sequences of commands connected through pipes
            execute_command_sequence(&argvv, command_counter);
        }
                
    }
        for (int i = 0; i < history_size; i++) {
        free_command(&history[i]);
    }
    free(history);
	return 0;
};

// 3. Execution of sequences of commands connected through pipes
void execute_command_sequence(char ****argvv, int num_commands) {
    int num_pipes = num_commands - 1;
    int pipe_fds[2 * num_pipes];

    // Create all necessary pipes
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipe_fds + i * 2) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Execute each command
    for (int i = 0; i < num_commands; i++) {
        pid_t pid = fork();
        if (pid == 0) { // Child process
            // If not the first command, get input from the previous pipe
            if (i != 0) {
                if (dup2(pipe_fds[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            // If not the last command, output to the next pipe
            if (i != num_commands - 1) {
                if (dup2(pipe_fds[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipes (very important! prevents hanging)
            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipe_fds[j]);
            }

            // Execute the command
            execvp((*argvv)[i][0], (*argvv)[i]);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
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

// 4. Execution of simple commands and sequence of commands with redirections (input, output
// and error) and in background.
/*void redirect_io(char *input_file, char *output_file, char *error_file) {
    if (input_file) {
        int in_fd = open(input_file, O_RDONLY);
        dup2(in_fd, STDIN_FILENO);
        close(in_fd);
    }
    if (output_file) {
        int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(out_fd, STDOUT_FILENO);
        close(out_fd);
    }
    if (error_file) {
        int err_fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        dup2(err_fd, STDERR_FILENO);
        close(err_fd);
    }
}*/

void redirect_io(char *input_file, char *output_file, char *error_file) {
    if (input_file) {
        int in_fd = open(input_file, O_RDONLY);
        if (in_fd != -1) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
    }
    if (output_file) {
        int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd != -1) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
    }
    if (error_file) {
        int err_fd = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (err_fd != -1) {
            dup2(err_fd, STDERR_FILENO);
            close(err_fd);
        }
    }
}

