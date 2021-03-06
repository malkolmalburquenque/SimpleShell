/*
   Malkolm Alburquenque
   McGill University
   260562740
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


//global variables needed
int pid, canKill;

//create the jobs struct
struct jobs{
    int jobpid;
    int number;
    int completed;
};

int jobslistsize=0;
struct jobs jobslist[128];

//anytime a background process is called, we need to create it as a Job
void createJob(int jpid){
    //printf("job created");
    jobslist[jobslistsize].number = 1+jobslistsize;
    jobslist[jobslistsize].jobpid = jpid;
    jobslist[jobslistsize].completed = 0;
    jobslistsize++;
}

//iterate thrugh the current list of jobs to find one with matching "number" variable
//returns negative 1 if job is not found
int jobSearch(int jnumber){
    for(int i =0; i< jobslistsize; i++){
        if(jobslist[i].number == jnumber){
            if(jobslist[i].completed == 0){
                jobslist[i].completed = 1;
                return jobslist[i].jobpid;
            }
            else{
                return -1;
            }
        }
    }
    return -1;
}

//checks for all jobs
void jobChecker(){
    int pid, tempstatus;
    for(int i =0; i< jobslistsize; i++){
        pid = waitpid(jobslist[i].jobpid, &tempstatus, WNOHANG);
        if(pid!=0)
            jobslist[i].completed = 1;
    }
}

//extraCommands is called in main to check if the command entered is manually implemented
// if it is a manually implemented command, we return one. Else return 0.
int extraCommands( char* args[]){
    char cwd[PATH_MAX + 1];

    //check for cd
    if(strcmp(args[0], "cd")==0){
        chdir(args[1]);
        return 1;
    }

    //check for pwd
    else if(strcmp(args[0], "pwd")==0){
        getcwd(cwd, PATH_MAX+1);
        printf("%s\n", cwd);
        return 1;
    }

    //check for exit
    else if(strcmp(args[0], "exit")==0){
        exit(0);
        return 1;
    }

    //check for jobs
    else if(strcmp(args[0], "jobs")==0){
        printf("Current Jobs:\n");
        for(int i=0; i<jobslistsize; i++){
            if(jobslist[i].completed==0){
                printf("[%d] \t %d \n", jobslist[i].number, jobslist[i].jobpid);
            }
        }
        return 1;
    }

    //check for fg
    else if(strcmp(args[0], "fg")==0){
        int tempstatus;
        //return as int
        int fgpid = jobSearch(atoi(args[1]));
        canKill = fgpid;
        if(fgpid > 0){
            waitpid(fgpid, &tempstatus, 0);
            if(!WIFEXITED(tempstatus)){
                perror("FG: Error");
                exit(EXIT_FAILURE);
            }
            else{
                printf("Child was forgrounded\n");
                return 1;
            }
        }
        else {
            printf("FG: Could not find job");
            return 1;
        }
        return 1;
    }

    //a return of 0 signifies the command was not manually implemented above
    else
        return 0;
}


//the following 4 methods are called in the main to check their status.

//signal handler
void signalHandler(int sig){
        printf("Ctrl-C recognized\n");
        //kill Ctrl-C signals
        kill(pid, SIGKILL);
}

//this method empties args to help display the prompt to the user
void emptyArgs(char *args[]){
    for (size_t i = 0; i < 20; i++) {
        args[i]=NULL;
    }
}

//check if it is a redirect
int isRedirect(char* args[]){
    int index;
    for(index=0; args[index]!=NULL; index++){
        if(strcmp(args[index], ">")==0){
            return index;
        }
    }
    return 0;
}

//check if it is a pipe
int isPipe(char* args[]){
    int index;
    for(index=0; args[index]!=NULL; index++){
        if(strcmp(args[index], "|")==0){
            return index;
        }
    }
    return 0;
}

char * extrapointer;
char extraline[512];
int getcmd(char *prompt, char *args[], int *background){
    int length, i=0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;
    //emptyArgs is called here to clear args
    emptyArgs(args);

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    //fix memory leak
    strcpy(extraline, line);
    extrapointer = extraline;
    free(line);

    if (length <= 0) {
        exit(-1);
    }

    if ((loc = index(extraline, '&')) != NULL){
        *background = 1;
        *loc = ' ';
    }else
        *background = 0;

    while ((token = strsep(&extrapointer, " \t\n")) != NULL){
        for (int j=0; j<strlen(token); j++)
            if (token[j] <=32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    return i;
}

int main (void){
    char *args[20];
    int bg, status;

    //index's representing the location of the '|' or '>'
    int redirectindex=0;
    int pipeindex=0;
    //for the redirect command
    int fd;

    //signals
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, signalHandler);


    while (1) {
        bg =0;
        int cnt = getcmd("\n¯\\_(ツ)_/¯ : ", args, &bg);
        if (cnt==0){
            continue;
        }
        //determine the index of args where the '>' is located
        redirectindex=isRedirect(args);
        //determine the index of args where the '|' is located
        pipeindex=isPipe(args);
        //check for jobs list
        jobChecker();
        //if extraCommands returns true, we do not iterate through the rest of the while loop.
        //if it returns 0, ignore.
       if(extraCommands(args)){
            continue;
        }

        //traditional comands
        pid = fork();
        int canKill = pid;
        //not background
        if(bg==0){
            if (pid < 0){
                perror("NON BG: PID Error\n");
                exit (EXIT_FAILURE);
            }
            //parent
            if (pid > 0){
                //printf("NON BG: Parent\n");
                waitpid (pid, &status, 0);
            }
            //child
            else{
                //is a redirect
                if(redirectindex){
                    close(1);
                    fd =  open(args[redirectindex+1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
                    args[redirectindex]= NULL;
                }
                if(pipeindex){
                    int argsindex;
                    //split args into two separate args at the pipe location
                    char * firstargs[20];
                    char * secondargs[20];

                    //fill the first half of the args (before the pipe)
                    for(argsindex = 0; argsindex<pipeindex; argsindex++){
                        firstargs[argsindex] = args[argsindex];
                    }
                    firstargs[argsindex] = NULL;
                    //fill the second half of the args (post pipe)
                    for(argsindex = 0; args[argsindex]!=NULL; argsindex++){
                        secondargs[argsindex] = args[argsindex + pipeindex + 1];
                    }
                    secondargs[argsindex] = NULL;

                    //create the pipe ends
                    int pipeEnds[2];
                    //create the pipe
                    if(pipe(pipeEnds) == -1){
                        //failed in creating the pipe
                        perror("Failed in creating the pipe");
                        exit(EXIT_FAILURE);
                    }
                    //fork
                    int pipepid = fork();
                    if (pipepid < 0){
                        perror("PIPE: PID Error\n");
                        exit (EXIT_FAILURE);
                    }
                    //parent
                    if (pipepid > 0){
                        //connect the pipe
                        close(pipeEnds[1]);
                        dup2(pipeEnds[0], 0);
                        close(pipeEnds[0]);
                        execvp(secondargs[0], secondargs);
                    }
                    else{
                        //connect the pipe
                        close(pipeEnds[0]);
                        dup2(pipeEnds[1], 1);
                        execvp(firstargs[0], firstargs);
                    }
                }
                //printf("NON BG: Child\n");
                int j = execvp(args[0], args);
                if (j < 0){
                    perror("NON BG: Error in execvp\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        //is background
        else{
            createJob(pid);
            if (pid < 0){
                perror("BG: PID Error\n");
                exit (EXIT_FAILURE);
            }
            //parent
            if (pid > 0){
                //printf("BG: Parent\n");
            }
            //child
            else{
                //printf("BG: Child\n");
                int j = execvp(args[0], args);
                if (j < 0){
                    perror("BG: Error in execvp\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}
