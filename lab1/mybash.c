#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"
#include <unistd.h>

/* Colores prompt */
#define COLOR_GREEN   "\x1b[92m"
#define COLOR_CYAN    "\x1b[96m"
#define COLOR_RESET   "\x1b[0m"     

static void show_prompt(void) {
    char *user = malloc(256);
    char *dir = malloc(256);
    gethostname(user,256);
    dir = getcwd(dir,256);

    printf (COLOR_GREEN "%s@mybash:",user);
    printf (COLOR_CYAN"~%s",dir);
    printf (COLOR_RESET"$ ");
    
    free(user);
    free(dir);
    user = NULL;
    dir = NULL;
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    pipe = pipeline_new(); 
    Parser input;
    
    while (true) { /* Hay que salir luego de ejecutar? -> NO, se deberia usar exit o algo por el estilo*/
        show_prompt();
        input = parser_new(stdin);
        pipe = parse_pipeline(input);
        execute_pipeline(pipe);
        pipe = pipeline_destroy(pipe);
        parser_destroy(input);
    }
    
    if (input != NULL) {parser_destroy(input); input = NULL;}
    if (pipe != NULL) {pipeline_destroy(pipe);}
    return EXIT_SUCCESS;
}