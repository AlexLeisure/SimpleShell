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

/**
 * @brief Use a parsed command list to fill in argv. Inserts a null terminator.
 * This is incredibly iffy code, but i'm not entirely sure if there's a better way of doing it without dynamically allocating new cstrings.
 * Thought of making a local cstring and giving a pointer to that, but once the function is out of scope it'll be invalid.
 * string.front() depends on c++11 so make sure to compile with the -std=c++11 flag
 * @param cmd 
 * @param argv 
 */
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
    pid_t pid; 
    std::vector<pid_t> children;

    parsed_t ret = parse(cmdline);
    if(ret.is_empty) 
        return;
    if(ret.is_error) {
        fprintf(stderr, ret.error.c_str());
        return;
    }

    // case: only one command 
    if(ret.cmd.size() == 1){
        fillArg(ret.cmd[0], argv);
        if(builtin_command(argv)) 
            return;
    }

    // 0  ->  read end
    // 1  ->  write end
    int nextPipe[2] = {0};
    int prevPipe[2] = {0};
    for(int i=0; i<ret.cmd.size(); i++){
        fillArg(ret.cmd[i], argv);

        bool isNext = false;
        bool isPrev = false;

        if(i != 0){
            prevPipe[0] = nextPipe[0];
            prevPipe[1] = nextPipe[1];
            isPrev = true;
        }
        if(i != (ret.cmd.size()-1)){
            if(!pipe(nextPipe)){
                //success
                isNext = true;
            }else{
                //error
                //pipe() sets errno, so would have to go through process of getting errno and using strerror()
                fprintf(stderr, "Unable to create pipe\n");
                return;
            }
        }
        pid = fork();
        if(pid == 0){ // child
            // TODO: make sure this is right
            if(isPrev){
                dup2(prevPipe[0], 0);
                close(prevPipe[1]);
            }
            if(isNext){
                dup2(nextPipe[1], 1);
                close(nextPipe[0]);
            }
            // call exec on argv[0] and argv
            if(execve(argv[0], argv, environ) < 0){
                printf("%s: command not found.\n", argv[0]);
                exit(0);
            }
        }else if(pid > 0){ // parent
            if(isPrev){
                close(prevPipe[1]);
            }
            children.push_back(pid);
        }else{
            //failure
            fprintf(stderr, "Failed to fork\n");
            exit(1);
        }
    }

    if(!ret.is_bg){
        int status = 0;
        for(auto child : children){
            waitpid(child, &status, 0);
            if(status == 1) 
                fprintf(stderr, "Child process [%i] finished with an error.\n", child);
        }
    }else{
        fprintf(stdout, "[%i]\n", getpid());
    }
}

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