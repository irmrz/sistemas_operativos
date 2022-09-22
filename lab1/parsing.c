#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <glib.h>
#include "parsing.h"
#include "parser.h"
#include "command.h"

/* Si no incluyo la definicion del struct tira error */
struct scommand_s {
    GQueue *list;
    char *redir_in;
    char *redir_out;
};

struct pipeline_s {
    GQueue *pipe;
    bool wait;
};


static scommand parse_scommand(Parser p) {
    /* Devuelve NULL cuando hay un error de parseo */
    assert(p != NULL);
    scommand result;
    result = scommand_new();
    arg_kind_t type;
    char *arg = NULL;
    bool next_not_null = true;
    parser_skip_blanks(p);
    
    while (next_not_null){
        arg = parser_next_argument(p, &type);
        parser_skip_blanks(p);
        
        if (arg != NULL ) {
            if (type == ARG_NORMAL) { 
                scommand_push_back(result,arg);
            }

            else if (type == ARG_INPUT){
                result->redir_in = arg;
            }
            else if (type == ARG_OUTPUT){
                result->redir_out = arg;
            }
        }
        
        else {
        next_not_null = false;
        }
    }
    
    return result;
}
pipeline parse_pipeline(Parser p) {
    assert(p != NULL);
    assert(!parser_at_eof(p));
    
    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error; 
    bool another_pipe= true;
    bool wait = pipeline_get_wait(result);
    parser_skip_blanks(p);
    cmd = parse_scommand(p);
    error = (cmd == NULL);


    while (another_pipe && !error) {
        pipeline_push_back(result,cmd);

        parser_op_pipe(p,&another_pipe);
        if (another_pipe) {
            cmd = parse_scommand(p);
        }
        else {
            parser_op_background(p, &wait);
            pipeline_set_wait(result,!wait);
            parser_garbage(p,&error);

        }
    }
    
    if (error)
    {
        printf("%s: not recognized",parser_last_garbage(p));
        pipeline_destroy(result);
        result = NULL;
    }
    
    /* Tolerancia a espacios posteriores */
    /* Consumir todo lo que hay inclusive el \n */
    /* Si hubo error, hacemos cleanup */

    return result; 
}
