#include <stdio.h>
#include <string.h>

typedef struct {
    const char *command;
    const char *description;
} CommandHelp;

CommandHelp commands[] = {
    {"create project", "Create a new project from a template"},
    {"copy project", "Copy an existing project to a new name"},
    {"rename project", "Rename a project"},
    {"delete project", "Delete a project"},
  
    {"install template <template_path>", "Install a new template"},
    {"create template", "Create a new template"},
    {"rename template", "Rename a template"},
    {"copy template", "Copy an existing template to a new name"},
    {"delete template", "Delete an existing template"},

    {"view [path]", "List all files and folders | optionally specify a path"},
    {"clear", "Clear the terminal"},
    {"goto <directory>", "Change directory to the specified folder"},

    {"delete <file_name/folder_name>", "Delete a file or folder (must be in same directory)"},
    {"recover <file_name/folder_name>", "Recover a deleted file from trash"},
    {"empty trash", "Empty the trash folder"},

    {"help", "Show this help message"},
};


void show_help() {
    int max_len = 0;
    for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        int len = strlen(commands[i].command);
        if (len > max_len) max_len = len;
    }

    printf("\nAvailable commands:\n");
    for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        printf("  %-*s - %s\n", max_len, commands[i].command, commands[i].description);
    }
    printf("\n");
}

void handle_command(const char *input) {
    if (strcmp(input, "help") == 0 || strcmp(input, "?") == 0) {
        show_help();
    }
}