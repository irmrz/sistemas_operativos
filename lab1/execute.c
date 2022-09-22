#include "execute.h"
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "command.h"
#include "builtin.h"
#include "strextra.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tests/syscall_mock.h"
#define READ 0 
#define WRITE 1 

char *invalid_cmd = "Ingrese un comando valido \n";
char *pipe_error = "Error abriendo el pipe \n";
char *fork_error = "Error ejecutando fork \n";
char *dup2_error = "Error en dup2 \n";
char *close_pipe_error = "Error cerrando el pipe \n";

static char **exec_cmd_parsing(unsigned int length, scommand cmd){
    char **argument_list = calloc(length + 1, sizeof(char));

    //Parseo los argumentos del cmd en la forma que execvp() los necesita
    for (unsigned int i = 0u; i < length; i++) {
        argument_list[i] = strdup(scommand_front(cmd));
        scommand_pop_front(cmd);
    }

    // Agrega NULL como ultimo elemento del array
    argument_list[length] = NULL;
    return argument_list;
}

static void execute_scommand(scommand cmd) {
    assert(cmd != NULL);
    unsigned int length = scommand_length(cmd);
    if (builtin_is_internal(cmd)) {
        builtin_run(cmd);
        exit(EXIT_SUCCESS);
    } else {
        char **argument_list = exec_cmd_parsing(length,cmd);
        /* Redirectores */
        char *redirector_out = scommand_get_redir_out(cmd);
        char *redirector_in = scommand_get_redir_in(cmd);

        if (redirector_in != NULL)
        {
            int fd_redir_in = open(redirector_in,O_RDONLY, S_IWUSR);
            if (close(STDIN_FILENO) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
            if (dup2(fd_redir_in,STDIN_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
            if (close(fd_redir_in) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error)); }
        }
        if (redirector_out != NULL)
        {
            int fd_redit_out = open(redirector_out,O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
            if (close(STDOUT_FILENO) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
            if (dup2(fd_redit_out,STDOUT_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
            if (close(fd_redit_out) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
        }

        free(redirector_out);
        free(redirector_in);
        if (execvp(argument_list[0], argument_list) == -1) { write(STDOUT_FILENO,invalid_cmd,strlen(invalid_cmd)); exit(EXIT_FAILURE); } 
    }
}


void execute_pipeline(pipeline apipe) {
    assert(apipe != NULL);

    if (pipeline_is_empty(apipe)) {
        write(STDOUT_FILENO,invalid_cmd,strlen(invalid_cmd)); 
        if(apipe != NULL) { apipe = NULL; }
    } else if (scommand_is_empty(pipeline_front(apipe))) {
        pipeline_pop_front(apipe);
    } else if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe));
        pipeline_pop_front(apipe);

    
    } else {
        unsigned int length = pipeline_length(apipe); //cantidad de comandos simples
        scommand cmd = pipeline_front(apipe);
        bool have2_wait = pipeline_get_wait(apipe);
        pid_t pid1, pid2, pid3;
        int fd1[2],fd2[2];
        int status;
        if (length >= 2) {
            if (pipe(fd1) == -1) { write(STDOUT_FILENO,pipe_error,strlen(pipe_error));} //Comunicar primer y segundo comando
        }

        pid1 = fork();

        /* Hijo: ejecuta primer comando */
        if (pid1 == 0){
            if (length >= 2){
                /* Preparar la entrada/salida */
                if (close(fd1[READ]) == -1) { exit(EXIT_FAILURE);}
                if (dup2(fd1[WRITE],STDOUT_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
                if (close(fd1[WRITE]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
            }
            
            /* Ejecuta */
            execute_scommand(cmd);
        }

        else if( pid1 == -1 ){ write(STDOUT_FILENO,fork_error,strlen(fork_error));}
        
        /*Padre: sigue con la ejecucion*/
        else{
            if (length == 1) {
                if (have2_wait) {waitpid(pid1,&status,WCONTINUED);}
            } else if (length >= 2){ 
                if (close(fd1[WRITE]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}  
                
                pipeline_pop_front(apipe);
                cmd = pipeline_front(apipe);
                if (have2_wait) {waitpid(pid1,&status,WCONTINUED);}
                if (length >= 3) {
                    if (pipe(fd2) == -1) { write(STDOUT_FILENO,pipe_error,strlen(pipe_error)); }  
                }
                pid2 = fork();

                /* Hijo: ejecuta segundo comando */
                if (pid2 == 0){
                    /* Preaparar la entrada/salida */

                    if (dup2(fd1[READ],STDIN_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
                    if (close(fd1[READ]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}

                    if (length >= 3){
                        if (close(fd2[READ]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
                        if (dup2(fd2[WRITE],STDOUT_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
                        if (close(fd2[WRITE]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
                    }
                    /* Ejecuta */
                    execute_scommand(cmd);
                }

                else if ( pid2 == -1 ){ write(STDOUT_FILENO,fork_error,strlen(fork_error)); }
                
                else{
                    if (close(fd1[READ]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
                    if (length == 2 && have2_wait) {
                        waitpid(pid2,&status,WCONTINUED);
                    } else if (length >= 3){
                        if (close(fd2[WRITE]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
                            
                        pipeline_pop_front(apipe);
                        cmd = pipeline_front(apipe);        
                        if (have2_wait) {waitpid(pid2,&status,WCONTINUED);}
                        pid3 = fork();
                        /* Hijo: ejecuta segundo comando */
                        if (pid3 == 0){
                            /* Preaparar la entrada/salida */
                            if (dup2(fd2[READ],STDIN_FILENO) == -1) { write(STDOUT_FILENO,dup2_error,strlen(dup2_error));}
                            if (close(fd2[READ]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}

                            /* Ejecuta */
                            execute_scommand(cmd);
                        }
                        else if ( pid3 == -1 ){ write(STDOUT_FILENO,fork_error,strlen(fork_error)); }
                        
                        else {
                            if (have2_wait) {waitpid(pid3,&status,WCONTINUED);} 
                        }
                    }  
                }

            }
        }
        if (length >= 3) {
            if (close(fd2[READ]) == -1) { write(STDOUT_FILENO,close_pipe_error,strlen(close_pipe_error));}
        }
        /*
        if (have2_wait) {
            if (length == 1) {waitpid(pid1,&status,WCONTINUED);}
            if (have2_wait) {waitpid(pid2,&status,WCONTINUED);}
            if (length == 3) {waitpid(pid3,&status,WCONTINUED);}
         }*/
    }
}