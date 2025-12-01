#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <errno.h>
#include <dirent.h>

#include "../scaffold/scaffold.h"
#include "../template/template.h"
#include "../utils/utils.h"
#include "../utils/selection.h"

void create_project_cmd()
{
    scaffold();
}

void install_template_cmd(const char *path)
{
    printf("Installing template: %s\n", path);
    install_template(path);
}

void copy_template_cmd() {
    copy_template();
}

void delete_template_cmd() {
    delete_template();
}

void clear_terminal_cmd() {
    system("clear");
    return;
}

void create_directory(const char *dir_name) {
    if (mkdir(dir_name, 0755) == -1) {
        if (errno != EEXIST) {  
            perror("Error creating directory");
            return;
        }
    } else {
        printf("Directory created: %s\n", dir_name);
    }
}

void change_directory(const char *dir_name) {
    if (chdir(dir_name) == -1) {
        perror("Error changing directory");
        return;  
    } else {
        printf("Changed directory to: %s\n", dir_name);
    }
}

void cd_cmd(const char *project_folder) {
  if (strcmp(project_folder, ".") != 0 && strcmp(project_folder, "..") != 0) {
        if (!directory_exists(project_folder)) {
            const char *yes_no[] = {"Yes", "No"};
            int install_choice = (selection("Directory does not exist. Do you want to create one?", yes_no, 2) == 0);
            if (install_choice) {
                create_directory(project_folder); 
            } else {
                return;
            }
        }
    }

    change_directory(project_folder);
}

void view_cmd(const char *path) {
    DIR *d = opendir(path);
    if (!d) {
        perror("view");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        printf("%s  ", dir->d_name);
    }
    printf("\n");
    closedir(d);
}


/**
 * Add download & upload template commands
 * This will download a template from the docs website database
 * users can upload and download templates on the docs website
 * upload <template-path>
 * download <template-path> 
 */
