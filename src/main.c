#include <stdio.h>
#include "cli.tab.h"  
#include "commands/commands.h"

int yyparse(void);

int main(void) {
    while (1) {
        printf("> ");
        fflush(stdout);

         if (yyparse() != 0) {
            // parse error
        }
        
    }
    return 0;
}