#include <stdio.h>
#include "cli.tab.h"  
#include "commands/commands.h"

int yyparse(void);

int main(void) {
    while (1) {
        printf("> ");
        yyparse();
    }
    return 0;
}