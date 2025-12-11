%{
#include <stdio.h>
#include <stdlib.h>
#include "cli.tab.h"
#include "commands/commands.h"
#include "utils/utils.h"

int yylex(void);
void yyerror(const char *s);
%}

%union {
    char *string;
}

%token CREATE PROJECT PROJECTS INSTALL TEMPLATE TEMPLATES COPY DELETE CLEAR GOTO VIEW RECOVER EMPTY TRASH HELP RENAME DOWNLOAD
%token <string> IDENTIFIER STRING
%token NEWLINE

%%

commands:
    /* empty */ 
  | command { print_prompt(); fflush(stdout); } commands
  | partial_command { print_prompt(); fflush(stdout); } commands
  | NEWLINE commands
;

command:
      create_project NEWLINE
    | install_template NEWLINE
    | copy_template NEWLINE
    | delete_template NEWLINE
    | clear_terminal NEWLINE
    | cd_command NEWLINE
    | view_command NEWLINE
    | delete_file_command NEWLINE 
    | recover_file_command NEWLINE
    | empty_trash NEWLINE
    | help_command NEWLINE
    | copy_project NEWLINE
    | create_template NEWLINE
    | rename_project NEWLINE
    | view_projects NEWLINE
    | view_trash NEWLINE
    | view_templates NEWLINE
    | rename_template NEWLINE
    | download_template NEWLINE
    | delete_project_command NEWLINE
;

partial_command:
      CREATE NEWLINE   { show_usage("CREATE"); }
    | COPY NEWLINE     { show_usage("COPY"); }
    | DELETE NEWLINE   { show_usage("DELETE"); }
    | EMPTY NEWLINE    { show_usage("EMPTY"); }
    | GOTO NEWLINE     { show_usage("GOTO"); }
    | INSTALL NEWLINE  { show_usage("INSTALL"); }
    | RENAME NEWLINE   { show_usage("RENAME"); }
    | DOWNLOAD TEMPLATE NEWLINE { show_usage("DOWNLOAD"); }
;

install_template: 
    INSTALL TEMPLATE STRING { install_template_cmd($3);  }
;

create_project:
    CREATE PROJECT { create_project_cmd(); }
;

copy_template:
    COPY TEMPLATE { copy_template_cmd(); }
;

delete_template: 
    DELETE TEMPLATE { delete_template_cmd(); }
;

clear_terminal:
    CLEAR { clear_terminal_cmd(); }
;

cd_command: 
    GOTO IDENTIFIER { cd_cmd($2); }
  | GOTO STRING     { cd_cmd($2); }
  | GOTO PROJECTS   { cd_cmd("projects"); }
;

view_command:
    VIEW { view_cmd("."); }
  | VIEW IDENTIFIER { view_cmd($2); }
;

delete_file_command:
    DELETE IDENTIFIER { delete_cmd($2); }
;

delete_project_command:
    DELETE PROJECT { delete_project_cmd(); }
;

recover_file_command:
    RECOVER IDENTIFIER { recover_file_cmd($2); }
;

empty_trash:
    EMPTY TRASH { empty_trash(); }
;

help_command:
    HELP { show_help(); }
;

copy_project:
    COPY PROJECT { copy_project_cmd(); }
;

rename_project:
    RENAME PROJECT { rename_project_cmd(); }
;

rename_template:
    RENAME TEMPLATE { rename_template_cmd(); }
;

create_template:
    CREATE TEMPLATE { create_template_cmd(); }
;

view_projects:
    VIEW PROJECTS { view_projects_cmd(); }
;

view_trash:
    VIEW TRASH { view_cmd(".trash"); }
;

view_templates:
    VIEW TEMPLATES { view_cmd(".templates"); }
;

download_template:
    DOWNLOAD TEMPLATE STRING { download_template_cmd($3); }
;

%%



void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    fflush(stderr);
    fflush(stdout);
}