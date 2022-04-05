#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parse.h"

#define MAXARGS 128
#define MAXLINE 2048
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);


int main()
{
    char cmdline[MAXLINE]; /* Command line */
    for(;;) {
        /* Read */
        printf("> ");
        fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
        exit(0);
        /* Evaluate */
        eval(cmdline);
    }
}

int builtin_command(char** argv){
    // strcmp returns 0 on equal
    if(!strcmp(argv[0], "quit")) 
        exit(0);
    if(!strcmp(argv[0], "&")) // ignore singleton &
        return 1;
    return 0; // not builtin
    
}

int parseline(char* buf, char** argv){
    char *delim; /* Points to first space delimiter */
    int argc; /* Number of args */
    int bg; /* Background job? */
    buf[strlen(buf)-1] = ' '; /* Replace '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;
    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    if (argc == 0)
        return 1; /* Ignore blank line */
    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;
    return bg;
}

void fillArg(std::vector<std::string>& cmd, char** argv){

    for(int i=0; i<cmd.size(); i++){
        //honestly i'm not sure if this is valid or not, i don't think it is
        //possible bug source right here

        argv[i] = &cmd[i].front();
    }
    argv[cmd.size()] = NULL;
}

void eval(char* cmdline){
    char* argv[MAXARGS]; // arg list execve()
    char buf[MAXLINE]; // modified cmd line
    int bg; // background or foreground

    parsed_t ret = parse(cmdline);
    if(ret.is_empty)
        return;
    if(ret.is_error)
        fprintf(stderr, ret.error.c_str());

    // case: only one command 
    if(ret.cmd.size() == 1){
        fillArg(ret.cmd[0], argv);
    }

    for(int i=0; i<ret.cmd.size(); i++){

    }
}

/*
// eval() returns nothing
//     argument: cmdline   // the string command line
// begin
//     set ret equal to the return value of a call to parse() on cmdline
//     if ret is empty
//         return
//     if ret is error
//         print error to stderr and return
//     if ret contains only one command
//         try to run it as a built in
//         if suscessfull
//             return
//     for each single_command in ret.cmd
//         create argv from single_command vector
//             // make sure it is zero terminated
//         declare next_pipe and prev_pipe
//         set is_next to false
//         set is_prev to false
//         if single_command is NOT the first command in ret.cmd
//             copy next_pipe to prev_pipe
//             set is_prev to true
//         if single_command is NOT the last command in ret.cmd
//             call pipe() on next_pipe
//             set is_next to true
//         set pid equal to the return value of a call of fork()
//         if pid is zero
//             // I am the child
//             if is_prev is true
//                 call dup2() on prev_pipe[0] and 0 // 0 is stdin
//                 call close() on prev_pipe[1]      // write end
//             if is_next is true
//                 call dup2() on next_pipe[1] and 1 // 1 is stdout
//                 call close() on next_pipe[0]      // read end
//             call exec on argv[0] and argv
//             exit
//         // parent
//         if is_prev is true
//             call close on prev_pipe[1]
//         push pid onto children vector
//     if ret is NOT background
//         wait for each child in children vector to finish
//     else
//         print pid and command line
        
// END
*/

int eval_line(char* cmdline){
    char* argv[MAXARGS]; // arg list execve()
    char buf[MAXLINE]; // modified cmd line
    int bg; // background or foreground
    pid_t pid; 
    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if(argv[0] == NULL)
        return; // ignore empty
    if(!builtin_command(argv)){
        pid = fork();
        if(pid == 0){
            // child runs job
            if(execve(argv[0], argv, environ) < 0){
                printf("%s: command not found.\n", argv[0]);
                exit(0);
            }
        }

        // Parent waits for foreground job to terminate
        if (!bg) {
            int status;
            if (waitpid(pid, &status, 0) < 0)
                fprintf(stderr, "waitfg: waitpid error");
        }else{
            printf("%d %s", pid, cmdline);
        }
    }
    return;
}