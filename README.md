# p2_minishell_2024

This repository contains the implementation of a mini shell for the P2-SSOO-23/24 course. The mini shell supports the execution of simple commands, sequences of commands connected through pipes, and commands with input/output/error redirection. Additionally, it includes custom commands `mycalc` and `myhistory`.

## Features

* Execution of simple commands
* Execution of sequences of commands connected through pipes
* Input/output/error redirection
* Background execution of commands
* Custom `mycalc` command for basic arithmetic operations
* Custom `myhistory` command to view and execute command history

## Files

* `msh.c`: Main source code file for the mini shell.
* `Makefile`: Makefile for compiling the mini shell.
* `checker_os_p2.sh`: Script for testing the mini shell.
* `authors.txt`: List of authors for the project.
* `default/msh.c`: Default implementation of the mini shell.
* `default/Makefile`: Default Makefile for compiling the mini shell.
* `default/checker_os_p2.sh`: Default script for testing the mini shell.
* `default/authors.txt`: Default list of authors for the project.
* `grade.txt`: Grade file for the project.
* `output.txt`: Output file for testing.
* `test.txt`: Test file for testing.

## Usage

1. Clone the repository:
   ```sh
   git clone <repository_url>
   cd p2_minishell_2024
   ```

2. Compile the mini shell:
   ```sh
   make
   ```

3. Run the mini shell:
   ```sh
   ./msh
   ```

## Custom Commands

### mycalc

The `mycalc` command performs basic arithmetic operations (addition, multiplication, and division) and maintains an accumulator for the sum.

Usage:
```sh
mycalc <operand_1> <add/mul/div> <operand_2>
```

### myhistory

The `myhistory` command displays the last 20 commands executed in the mini shell and allows re-execution of a specific command from the history.

Usage:
```sh
myhistory
myhistory <command_index>
```

## Authors

* Alba Tello Marcos
* Carmen Rodríguez García
* LiangJi Zhu
