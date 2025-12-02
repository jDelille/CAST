#include <stdio.h>
#include "cli.tab.h"  
#include "commands/commands.h"

int yyparse(void);

int main(void) {
    clear_terminal_cmd();
    while (1) {
        print_prompt();
        yyparse();
    }
    return 0;
}