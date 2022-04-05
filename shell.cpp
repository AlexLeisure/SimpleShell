#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

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

// returns x | x = 0   =>  not background
//             x != 0  =>  background
int parseline(char* buf, char** argv){
    char* delim;
    int argc;
    int bg;

    buf[strlen(buf)-1] = ' '; // replaces \n with space

    // interesting algorithm, seems like it's adding the 
    // whole string to argv but placing in a \0 so any
    // string operations stop early? not entirely sure
    while((delim = strchr(buf, ' '))){
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while(*buf && (*buf == ' ')) //ignores spaces
            buf++;
    }
    argv[argc] = NULL;
    if(argc == 0) return 1;
    if((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;
    return bg;    
}

void eval(char* cmdline){
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
            if(execve(argv[0], environ) < 0){
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