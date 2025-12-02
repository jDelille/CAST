#include <stdio.h>
#include <string.h>

typedef struct {
    const char *command;
    const char *description;
} CommandHelp;

CommandHelp commands[] = {
    {"create project", "Create a new project from a template"},
    {"view [path]", "List all files and folders | optionally specify a path"},
    {"install template <template_path>", "Install a new template"},
    {"copy template", "Copy an existing template to a new name"},
    {"delete template", "Delete an existing template"},
    {"delete <file_name/folder_name>", "Delete a file or folder"},
    {"recover <file_name>", "Recover a deleted file from trash"},
    {"empty trash", "Empty the trash folder"},
    {"clear", "Clear the terminal"},
    {"goto <directory>", "Change directory to the specified folder"},
    {"help", "Show this help message"},
};


void show_help() {
    // Determine the maximum length of command strings
    int max_len = 0;
    for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        int len = strlen(commands[i].command);
        if (len > max_len) max_len = len;
    }

    printf("\nAvailable commands:\n");
    for (int i = 0; i < sizeof(commands)/sizeof(commands[0]); i++) {
        // Align descriptions based on the longest command
        printf("  %-*s - %s\n", max_len, commands[i].command, commands[i].description);
    }
    printf("\n");
}

void handle_command(const char *input) {
    if (strcmp(input, "help") == 0 || strcmp(input, "?") == 0) {
        show_help();
    } else if (strcmp(input, "exit") == 0) {
        printf("Exiting CLI...\n");
        // exit logic here
    } else {
        printf("Unknown command: %s\n", input);
    }
}