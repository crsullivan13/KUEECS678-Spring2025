> # EECS 678 - Quash Shell

## Introduction

In this project, you will complete the Quite a Shell (quash) program using the
UNIX system calls. You may work in groups of 2. The purpose of this project is
as follows:

- Getting familiar with the Operating System (UNIX) interface.
- Exercising UNIX system calls.
- Understanding the concept of a process from the user point of view.

A skeleton has been provided, but it lacks most of the core functionality one
would expect from a shell program. Quash should behave similar to csh, bash or
other popular shell programs.

## Installation

To build Quash use:
> `make`

To generate this documentation in HTML and LaTeX use:
> `make doc`

To clean quash use:
> `make clean`

## Usage

To run Quash use:
> `./quash`
or
> `make test`

## Features

<em><b>The main file you will modify is src/execute.c. You may not use or modify
files in the src/parsing directory, with the notable exception of the
destroy_parser() function in src/parsing/parse_interface.c if necessary. Your
output should match our example result files exactly.</b></em>

The following features should be implemented in Quash:

- Quash should be able to run executables (the basic function of a shell) with
  command line parameters

- If the executable is not specified in the absolute or relative path format
  (starting with sequences of ‘/’, './', or '../'), quash should search the
  directories in the environment variable PATH (see below). If no executable
  file is found, quash should print an error message to standard error. Quash
  should allow both foreground and background executions. Character ‘&’ is used
  to indicate background execution. Commands without ‘&’ are assumed to run in
  foreground.

    - When a command is run in the background, quash should print: "Background
      job started: [JOBID]    PID    COMMAND"

    - When a background command finishes, quash should print: "Completed:
      [JOBID]    PID    COMMAND"

```bash
[QUASH]$ program1 &
Background job started: [1]    2342    program1 &
[QUASH]$ ls
Documents Downloads
Completed: [1]    2342    program1 &
```

- Quash should implement I/O redirection. The `<` character is used to redirect
  the standard input from a file. The `>` character is used to redirect the
  standard output to a file while truncating the file. The `>>` string is used
  to redirect the standard output to a file while appending the output to the
  end of the file.

```bash
[QUASH]$ echo Hello Quash! > a.txt  # Write "Hello Quash!\n" to a file
[QUASH]$ cat a.txt
Hello Quash!
[QUASH]$ echo Hey Quash! > a.txt  # Truncate the previous contents of a.txt and write "Hey Quash!\n" to the file
[QUASH]$ cat a.txt          # Print file contents. If we didn't actually truncate we would see "Hey Quash!h!\n" as the output of this command.
Hey Quash!
[QUASH]$ cat < a.txt        # Make cat read from a.txt via standard in
Hey Quash!
[QUASH]$ cat < a.txt > b.txt  # Multiple redirect. Read from a.txt and write to b.txt.
[QUASH]$ cat b.txt
Hey Quash!
[QUASH]$ cat a.txt >> b.txt  # Append output of a.txt to b.txt
[QUASH]$ cat b.txt
Hey Quash!
Hey Quash!
[QUASH]$
```

- Quash should support pipes `|`.

```bash
[QUASH]$ cat src/quash.c | grep running
// Check if loop is running
bool is_running() {
  return state.running;
  state.running = false;
  while (is_running()) {
[QUASH]$ cat src/quash.c | grep running | grep return
  return state.running;
```

### Built-in Functions

All built-in commands should be implemented in quash itself. They cannot be
external programs of any kind. Quash should support the following built-in
functions:

- `echo` - Print a string given as an argument. The output format should be
  the same as bash (a string followed by new line '\\n')

```bash
[QUASH]$ echo Hello world! 'How are you today?'
Hello world! How are you today?
[QUASH]$ echo $HOME/Development
/home/jrobinson/Development
[QUASH]$
```

- export - Sets the value of an environment variable. Quash should support
  reading from and writing to environment variables.

```bash
[QUASH]$ export PATH=/usr/bin:/bin  # Set the PATH environment variable
[QUASH]$ echo $PATH                 # Print the current value of PATH
/usr/bin:/bin
[QUASH]$ echo $HOME
/home/jrobinson
[QUASH]$ export PATH=$HOME  # Set the PATH environment variable to the value of HOME
[QUASH]$ echo $PATH         # Print the current value of PATH
/home/jrobinson
[QUASH]$
```

- `cd` - Change current working directory. This updates both the actual working
  directory and the PWD environment variable.

```bash
[QUASH]$ echo $PWD
/home/jrobinson
[QUASH]$ cd ..              # Go up one directory
[QUASH]$ echo $PWD
/home
[QUASH]$ cd $HOME           # Go to path in the HOME environment variable
/home/jrobinson
[QUASH]$
```

- `pwd` - Print the absolute path of the current working directory. Make sure
  you are printing out the actual working directory and not just the PWD
  environment variable.

```bash
[QUASH]$ pwd                # Print the working directory
/home/jrobinson
[QUASH]$ echo $PWD          # Print the PWD environment variable
/home/jrobinson
[QUASH]$ export PWD=/usr    # Change the PWD environment variable
[QUASH]$ pwd
/home/jrobinson
[QUASH]$ echo $PWD
/usr
[QUASH]$
```

- `quit` & `exit` - Use these to terminate quash. These are already implemented
  for you.

```bash
[BASH]$ ./quash
Welcome...
[QUASH]$ exit
[BASH]$ ./quash
Welcome...
[QUASH]$ quit
[BASH]$
```

- `jobs` - Should print all of the currently running background processes in the
  format: "[JOBID] PID COMMAND" where JOBID is a unique positive integer quash
  assigns to the job to identify it, PID is the PID of the child process used
  for the job, and COMMAND is the command used to invoke the job.

```bash
[QUASH]$ find -type f | grep '*.c' > out.txt &
Background job started: [1]    2342    find / -type f | grep '*.c' > out.txt &
[QUASH]$ sleep 15 &
Background job started: [2]    2343    sleep 15 &
[QUASH]$ jobs               # List currently running background jobs
[1]    2342    find / -type f | grep '*.c' > out.txt &
[2]    2343    sleep 15 &
[QUASH]$
```

## Useful Functions in the Quash Skeleton

The following are some funtions outside of src/execute.c that you may want to
use in your implementation:

- @a get_command_string()

- @a destroy_parser()

- Things in src/deque.h. Quash can make due with only implementations of this
  data structure (i.e. background jobs list, process id list). These are
  preprocessor macro definitions of the data structure and will expand and
  specialize to a type when used (similar to templates in c++). See example
  usage at the bottom of src/deque.h. <b>You don't have to use this deque
  implementation if you would rather write your own list data structure.</b>

  - @a IMPLEMENT_DEQUE_STRUCT() - Generates the double ended queue structure
    specialized to a type

  - @a PROTOTYPE_DEQUE() - Generates the function prototypes for the functions
    that are generated by the @a IMPLEMENT_DEQUE() macro

  - @a IMPLEMENT_DEQUE() - Generates functions for use with the structure
    generated by @a IMPLEMENT_DEQUE_STRUCT().

    - NOTE: The following functions were generated with a call to @a
      IMPLEMENT_DEQUE(Example, Type). Each function is named after the contents
      passed into the first argment of this macro (in this case @a Example).

    - @a new_Example()

    - @a new_destructable_Example()

    - @a destroy_Example()

    - @a empty_Example()

    - @a is_empty_Example()

    - @a length_Example()

    - @a as_array_Example()

    - @a apply_Example()

    - @a push_front_Example()

    - @a push_back_Example()

    - @a pop_front_Example()

    - @a pop_back_Example()

    - @a peek_front_Example()

    - @a peek_back_Example()

    - @a update_front_Example()

    - @a update_back_Example()

## Useful System Calls and Library Functions

The following is a list and brief description of some system calls and library
functions you may want to use and their respective man page entries. Note that
this list may not be exhaustive, but be sure what ever library functions you use
will run on the lab machines:

- atexit(3) - Enroll functions that should be called when exit(3) is called

- chdir(2) - Changes the current working directory

- close(2) - Closes a file descriptor

- dup2(2) - Copies a file descriptor into a specified entry in the file
  descriptor table

- execvp(3) - Replaces the current process with a new process

- exit(3) - Imediately terminate the current process with an exit status

- fork(2) - Creates a new process by duplicating the calling process

- getenv(3) - Reads an environment variable from the current process environment

- getwd(3) - Gets the current working directory as a string
  (get_current_dir_name(3) may be easier to use)

- get_current_dir_name(3) - Gets the current working directory and stores it in
  a newly allocated string

- kill(2) - Sends a signal to a process with a given pid

- open(2) - Opens a file descriptor with an entry at the specified path in the
  file system

- pipe(2) - Creates a unidirectional communication pathway between two processes

- printf(3) - Prints to the standard out (see also fprintf)

- setenv(3) - Sets an environment variable in the current process environment

- waitpid(2) - Waits or polls for a process with a given pid to finish

You may NOT use the system(3) function anywhere in your project

## Project Hints and Comments

In Quash, a job is defined as a single command or a list of commands separated
by pipes. For example the following are each one job:

cat file.txt       # A job with a single process running under it

find | grep *.qsh  # A job with two processes running under it

A job may contain more than one process and should have a unique id for the
current list of jobs in Quash, a knowledge of all of the pids for processes that
run under it, and an expanded string depicting what was typed in on the command
line to create that job. When passing the pid to the various print job functions
you just need to give one pid associated with the job. The job id should also be
assigned in a similar manner as bash assigns them. That is the job id of a new
background job is one greater than the maximum job id in the background job
list. Experiment with background jobs in Bash for more details on the id
assignment.

The structure of the functions in src/execute.c will be explained in the
following paragraphs.

The entry level function for execution in quash is @a run_script(). This
function is responsible for calling @a create_process() on an array of @a
CommandHolders. After all of the processes have been created for a job,
run_script() should either wait on all processes inside a foreground job to
complete or add it to the background job list without waiting if it is a
background job.

The @a create_process() function is intended to be the place where you fork
processes, handle pipe creation, and file redirection. You should not call
execvp(3) from this function. Instead you should call derivatives of the @a
example_run_command() function. Also you can determine whether you should use
the boolean variables at the top of this function to determine if pipes and
redirects should be setup. It may be necessary to keep a global execution state
structure so that different calls to create process can view important
information created in previous invocations of create_process() (i.e. the file
descriptors for open pipes of previous processes).

When implementing the run_<command type> functions in src/execute.c, the command
structures usually hold everything needed to pass to the corresponding function
calls. For example, the @a GenericCommand structure contains an args field fully
formatted and ready to pass into the argv argument of the @a execvp(3) library
function. The one exception to this rule is @a run_cd(). In the @a CDCommand
structure, the field dir is the path that the user typed. This needs to be
expanded to an absolute path with @a realpath(3) before you use it.

You should not have to search for environment variables ($) in your functions.
The parser uses your implementation of @a lookup_env() function to expand the
environment variables for you.

## Testing

There is an automated testing script written for this project in run_tests.bash.
Using the "make test" target in the make file will run the ./run-tests.bash
command present in the Makefile. If you want more control you may run this
script directly from the command line with "./run_tests.bash [-cdstuv]". The
various options are listed below or with "./run_tests.bash -h".

- -c Clean up all files created during testing.

- -d If there is a difference between what is seen and what is expected this
   option will immediately print out the diff file rather than telling you where
   you can find the diff file.

- -p Allow all tests to be run in parallel as background processes. This will
   greatly speed up the testing process but output to the command line will be
   scattered.

- -s Keep the sandbox directory around after testing (this is a debug option for
   the tester and probably not of much use unless modifying run-tests.bash).

- -t Keep the the temporary directory "test-cases/<test-name-dir>/.tmp" around
   after tests are complete (this is a debug option for the tester and probably
   not of much use unless modifying run-tests.bash).

- -v Print out all output from the test case if diff picked up any differences
   between the test output and expected output.

## Grading Policy

Partial credit will be given for incomplete programs. However, a program that
cannot compile will get 0 points. The feature tests are placed into multiple
tiers of completeness. The output to standard out from your code must match our
output exactly, except for whitespace, for the next tier of grading to be
accessible. This is due to reliance of previous tiers in subsequent tier tests.
If we cannot run your code in one tier then it becomes far more difficult test
later tiers. The point breakdown for features is below:

<table>
    <tr>
        <th> Description </th>
        <th> Score </th>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Tier 0 </li>
                <ul>
                    <li> Quash compiles </li>
                </ul>
            </ul>
        </td>
        <td>
            10%
        </td>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Tier 1 </li>
                <ul>
                    <li> Single commands without arguments (ls) </li>
                    <li> Simple built-in commands </li>
                    <ul>
                        <li> pwd </li>
                        <li> echo with a single argument </li>
                    </ul>
                </ul>
            </ul>
        </td>
        <td>
            30%
        </td>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Tier 2 </li>
                <ul>
                    <li> Single commands with arguments (ls -a /) </li>
                    <li> Built-in commands </li>
                    <ul>
                        <li> echo with multiple arguments </li>
                        <li> cd </li>
                        <li> export </li>
                    </ul>
                    <li> Environment Variables </li>
                    <ul>
                        <li> echo with environment variables (echo $HOME) </li>
                        <li> Execution with environment variables (du -H $PWD/..)
                    </ul>
                </ul>
            </ul>
        </td>
        <td>
            30%
        </td>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Tier 3 </li>
                <ul>
                    <li> Built-in commands </li>
                    <ul>
                        <li> jobs </li>
                        <li> kill </li>
                    </ul>
                    <li> Piping output between one command and another (find -type f \| grep '*.c') </li>
                    <li> Redirect standard input to any command from file (cat < a.txt) </li>
                    <li> Redirect standard output from a command to a file (cat b.txt > a.txt) </li>
                    <li> Background processes </li>
                    <ul>
                        <li> Job completion notification </li>
                    </ul>
                </ul>
            </ul>
        </td>
        <td>
            30%
        </td>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Tier 4 (extra credit) </li>
                <ul>
                    <li> Pipes and redirects can be mixed (cat < a.txt \| grep -o World \| cat > b.txt) </li>
                    <li> Pipes and redirects work with built-in commands </li>
                    <li> Append redirection (cat a.txt \| grep -o World >> b.txt) </li>
                </ul>
            </ul>
        </td>
        <td>
            10%
        </td>
    </tr>
    <tr>
        <td>
            <ul>
                <li> Valgrind Memory Errors </li>
                <ul>
                    <li> While not ideal, you will not lose any points for "still reachable" blocks </li>
                    <li> Unacceptable Memory Leaks </li>
                    <ul>
                        <li> Definately lost </li>
                        <li> Indirectly lost </li>
                        <li> Possibly lost </li>
                    </ul>
                    <li> Unacceptable Access Violations </li>
                    <ul>
                        <li> Invalid Read </li>
                        <li> Invalid Write </li>
                        <li> Invalid free </li>
                        <li> Use of uninitialized values </li>
                    </ul>
                </ul>
            </ul>
        </td>
        <td>
            -5% from tier grade down to 0% for each tier with violations
        </td>
    </tr>
</table>

## Submission

Each group should submit the project to your TA via Blackboard. Create a zip
file of your code using "make submit". You should also check that the zipped
project still builds and runs correctly after building the submit target with
"make unsubmit". Your TA will be using this command to extract and build your
project so make sure it works correctly. If you modify either of these targets,
please ensure all file extensions are renamed to ".txt" in the submission and
extract correctly with the unsubmit target.

## Miscellaneous
Start early!
You need to use C language to implement this project.
