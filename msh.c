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

// myhistory function

// mycalc function
void mycalc(char **args) {
    int x = atoi(args[1]);
    int y = atoi(args[3]);
    char *operator = args[2];

    if (strcmp(operator, "add") == 0) {
        printf("%d\n", x + y);
    } else if (strcmp(operator, "mul") == 0) {
        printf("%d\n", x * y);
    } else if (strcmp(operator, "div") == 0) {
        if (y != 0) {
            printf("%d\n", x / y);
        } else {
            printf("Error: Division by zero\n");
        }
    } else {
        printf("Invalid operation\n");
    }
}

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

        /*
		char input[1024];
        char *commands[128];

        while (1) {
            printf("MSH>> ");
            fflush(stdout);

            if (!fgets(input, sizeof(input), stdin)) break; // Termina si se detecta EOF
            input[strcspn(input, "\n")] = 0; // Remueve el carácter de nueva línea

            bool runInBackground = 0;
            if (input[strlen(input) - 1] == '&') {
                runInBackground = 1;
                input[strlen(input) - 1] = 0; // Elimina el '&' del final
            }

            int n_commands = 0;
            commands[n_commands] = strtok(input, "|");
            while (commands[n_commands] != NULL) {
                n_commands++;
                commands[n_commands] = strtok(NULL, "|");
            }

            int fd[2], in_fd = STDIN_FILENO;
            for (int i = 0; i < n_commands; i++) {
                if (i < n_commands - 1) { // No es el último comando, crea una tubería
                    pipe(fd);
                }


                char *argv[128];
                int argc = 0;

                argv[argc] = strtok(commands[i], " ");
                while (argv[argc] != NULL) {
                    // Detén la búsqueda si encuentras símbolos de redirección
                    if (strcmp(argv[argc], "<") == 0 || strcmp(argv[argc], ">") == 0 || strcmp(argv[argc], "!>") == 0) {
                        break;
                    }
                    argc++;
                    argv[argc] = strtok(NULL, " ");
                }

                pid_t pid = fork();

                if (pid == 0) { // Proceso hijo
                    // Aplica la redirección de entrada si es necesario
                    if (in_fd != STDIN_FILENO) {
                        dup2(in_fd, STDIN_FILENO);
                        close(in_fd);
                    }
                    // Aplica la redirección de salida si no es el último comando
                    if (i != n_commands - 1) {
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                    }

                    // Manejo de redirecciones dentro del comando
                    for (int i = 0; argv[i] != NULL; i++) {
                        if (strcmp(argv[i], "<") == 0 && argv[i + 1] != NULL) {
                            int fd = open(argv[i + 1], O_RDONLY);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        } else if (strcmp(argv[i], ">") == 0 && argv[i + 1] != NULL) {
                            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        } else if (strcmp(argv[i], "!>") == 0 && argv[i + 1] != NULL) {
                            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                            dup2(fd, STDERR_FILENO);
                            close(fd);
                        }
                        argv[i] = NULL; // Elimina el resto de los argumentos después de la redirección
                    }

                    if (execvp(argv[0], argv) == -1) {
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    }

                    close(fd[0]);
                    close(fd[1]);

                } 
                else if (pid < 0) {
                    perror("fork");
                    exit(EXIT_FAILURE);
                }

                if (i != n_commands - 1) {
                    close(fd[1]);
                }

                if (!runInBackground || i == n_commands - 1) {
                    wait(NULL); // Espera al proceso hijo si no es en segundo plano o es el último comando
                } else {
                    printf("[PID] %d running in background\n", pid);
                }

                
            }
        
            if (i < n_commands - 1) {
            in_fd = fd[0];
            } 
            else {
                // Si es el último comando, no hay necesidad de mantener el último in_fd abierto
                if (in_fd != STDIN_FILENO) {
                    close(in_fd);
                }
            }
        } */


		/************************ STUDENTS CODE ********************************/
        
	    if (command_counter > 0) {
			if (command_counter > MAX_COMMANDS){
				printf("Error: Maximum number of commands is %d \n", MAX_COMMANDS);
			}
			else {
				// Print command
				print_command(argvv, filev, in_background);

                // read_command (char ***argvv, char **filev, int *bg);
                // ctrl + c -> 0
                // error -> -1
                // num of commands -> n
                /* for (int n = 1; n < sizeof(argvv); n++)
                {
                    // command in position i
                    printf("Command %i: %s\n", n, argvv[n][0]);

                    // its first argumment
                    printf("Argumment %i: %s\n", n, argvv[n][1]);

                    // (<) filev[0] string contains name of input file 
                    // (>) filev[1] string contains name of output file 
                    // (!>) filev[2] string contains name of error output file

                    printf("File %i: %s\n", n, filev[0]);
                    // in_background used for checking if is executed in background
                    // 0 = yes
                    // 1 = no
                    printf("Background %i: %d\n", n, in_background);
                }
                */
                /*printf("%c", argvv);
                for (int n = 1; n < sizeof(argvv); n++)
                {
                    if (argvv[n][0] == "ls -l")
                    {
                        execl("/bin/ls", "ls", "-l", (char *)NULL);
                        perror("execl");
                    }

                }*/
			}
            // Process each command
            for (int i = 0; i < command_counter; i++) {
                // Fork a new process
                pid_t pid = fork();
                if (pid == 0) { // Child process
                    // Handle redirection if necessary
                    
                    // Execute the command
                    execvp(argvv[i][0], argvv[i]);
                    // If execvp returns, an error occurred
                    perror("execvp");
                    exit(EXIT_FAILURE);
                } else if (pid > 0) { // Parent process
                    if (!in_background) {
                        // Wait for the child process to finish
                        waitpid(pid, NULL, 0);
                    }
                } else {
                    // Handle fork failure
                    perror("fork");
                }
            }
		}
	}
	
	return 0;
};
