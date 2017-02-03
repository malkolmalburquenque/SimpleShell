/*
Malkolm Alburquenque
McGill University
260562740
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>



int extraCommands( char* args[]){
    
    //check for output redirection
    if(strcmp(args[1], ">")==0){
        args[1]=args[2];
        args[2]=NULL;
        return 2;
    }
    //check for command piping
    if(strcmp(args[1], "|")==0){
        args[1]=args[2];
        args[2]=NULL;
        return 3;
    }
    else
        return 0;
}


//this method empties args
void emptyArgs(char *args[]){
	for (size_t i = 0; i < 20; i++) {
		args[i]=NULL;
	}
}

int getcmd(char *prompt, char *args[], int *background){
	int length, i=0;
	char *token, *loc;
	char *line = NULL;
	size_t linecap = 0;
	emptyArgs(args);

	printf("%s", prompt);
	length = getline(&line, &linecap, stdin);

	if (length <= 0) {
		exit(-1);
	}

	if ((loc = index(line, '&')) != NULL){
		*background = 1;
		*loc = ' ';
	}else
		*background = 0;

	while ((token = strsep(&line, " \t\n")) != NULL){
		for (int j=0; j<strlen(token); j++)
			if (token[j] <=32)
				token[j] = '\0';
		if (strlen(token) > 0)
			args[i++] = token;
	}
    //need to free memory
    free(line);
	return i;
}

int main (void){
	char *args[20];
	int bg, status, extraCmdReturn;

	while (1) {
		bg =0;
		int cnt = getcmd("\n++ ", args, &bg);
        if (cnt==0){
            continue;
        }
        
         
    switch(extraCommands(args)){
        //all other extraCommands not listed below
        case(1){
            continue; 
        }
        //redirect
        case(2){

        }
        //pipe
        case(3){
            
            if(pipe(args)==-1){
                perror("Error in creating pipe"\n);
                exit (EXIT_FAILURE);
            }
            int pipe_pid=fork();
                if (pid < 0){
                    perror("PIPE: PID Error\n");
                    exit (EXIT_FAILURE);
                }
                if (pipe_pid > 0){
                    
                }

        }
        //all input commands not dealt with in extraCommands
        default;{
            printf("Standard command issued");
        }
            

        /* the steps can be..:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
         * (3) if background is not specified, the parent will wait,
         * otherwise parent starts the next command... */


		 int pid = fork();
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
                 if(!WIFEXITED(status)){
					 perror("NON BG: Child did not exit\n");
					 exit(EXIT_FAILURE);
				 }
			 }
			 //child
			 else{
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
               // printf("BG: Child\n");
			 	int j = execvp(args[0], args);
				if (j < 0){
				 	perror("BG: Error in execvp\n");
				 	exit(EXIT_FAILURE);
				}
			 }
		 }
	}
}
