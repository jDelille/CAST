#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <ftw.h>
#include <limits.h>

#include "../scaffold/scaffold.h"
#include "../template/template.h"
#include "../project/project.h"
#include "../utils/utils.h"
#include "../utils/selection.h"
#include "../utils/defs.h"
#include "../api/download_template.h"

void create_project_cmd()
{
    scaffold();
}

void install_template_cmd(const char *path)
{
    install_template(path);
}

void copy_template_cmd()
{
    copy_template();
}

void delete_template_cmd()
{
    delete_template();
}

void copy_project_cmd()
{
    copy_project();
}

void rename_project_cmd()
{
    rename_project();
}

void clear_terminal_cmd()
{
    system("clear");
    return;
}

void create_template_cmd()
{
    create_template();
}

void view_projects_cmd()
{
    char projects_list[64][256];
    int num_projects = list_projects(projects_list, 64);

    if (num_projects == 0) {
        printf("No projects found.\n");
        return;
    }

    printf("Projects:\n");
    for (int i = 0; i < num_projects; i++) {
        printf("%s\n", projects_list[i]);
    }
}

void rename_template_cmd() {
    rename_template();
}

void show_usage(const char *command) {
    if (strcmp(command, "CREATE") == 0) {
        printf("Usage: create project | create template\n");
    } else if (strcmp(command, "COPY") == 0) {
        printf("Usage: copy project | copy template\n");
    } else if (strcmp(command, "DELETE") == 0) {
        printf("Usage: delete project | delete template | delete <filename>\n");
    } else if (strcmp(command, "VIEW") == 0) {
        printf("Usage: view projects | view templates | view trash\n");
    } else if (strcmp(command, "EMPTY") == 0) {
        printf("Usage: empty trash\n");
    } else if (strcmp(command, "GOTO") == 0) {
        printf("Usage: goto <directory>\n");
    } else if (strcmp(command, "INSTALL") == 0) {
        printf("Usage: install template <template_path>\n");
    } else if (strcmp(command, "RENAME") == 0){
        printf("Usage: rename project | rename template\n");
    } else if (strcmp(command, "DOWNLOAD") == 0){
        printf("Usage: download template '<template_name>'\n");
    } else {
        printf("Unknown command. Type HELP for a list of commands.\n");
    }
}

void create_directory(const char *dir_name)
{
    if (mkdir(dir_name, 0755) == -1)
    {
        if (errno != EEXIST)
        {
            perror("Error creating directory");
            return;
        }
    }
    else
    {
        printf("Directory created: %s\n", dir_name);
    }
}

void change_directory(const char *dir_name)
{
    if (chdir(dir_name) == -1)
    {
        perror("Error changing directory");
        return;
    }
}

void cd_cmd(const char *input_path)
{
    char path[512];
    strncpy(path, input_path, sizeof(path));
    path[sizeof(path) - 1] = '\0';

    convert_windows_path_to_wsl(path);

    if (!directory_exists(path))
    {
        if (path[0] != '/')
        {
            const char *yes_no[] = {"Yes", "No"};
            int install_choice = (selection(
                                      "Directory does not exist. Do you want to create one?",
                                      yes_no, 2) == 0);
            if (install_choice)
            {
                create_directory(path);
            }
            else
            {
                return;
            }
        }
        else
        {
            printf("Directory does not exist: %s\n", path);
            return;
        }
    }

    change_directory(path);
}

void view_cmd(const char *path)
{
    DIR *d = opendir(path);
    if (!d)
    {
        perror("view");
        return;
    }

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        printf("%s  ", dir->d_name);
    }
    printf("\n");
    closedir(d);
}

void delete_cmd(const char *name) {
    mkdir(".trash", 0700);

    char trash_path[512];
    snprintf(trash_path, sizeof(trash_path), ".trash/%s", name);

    struct stat st;
    if (stat(name, &st) != 0) {
        perror("Error: file or folder not found");
        return;
    }

    if (rename(name, trash_path) == 0) {
        printf("Moved '%s' to the trash\n", name);
    } else {
        perror("Error moving to trash");
    }
    fflush(stdout);
}

void delete_project()
{
    while (1)
    {
        char projects_list[64][256];
        int num_projects = list_projects(projects_list, 64);

        int filtered_count = 0;
        const char *project_names[65];
        for (int i = 0; i < num_projects; i++) {
            if (strcmp(projects_list[i], ".trash") != 0) {
                project_names[filtered_count++] = projects_list[i];
            }
        }

        if (filtered_count == 0)
        {
            printf("No projects found to delete.\n");
            return;
        }

        project_names[filtered_count] = "Quit";

        int selected = selection("Which project do you want to delete?", project_names, filtered_count + 1);
        if (selected == filtered_count || selected < 0)
        {
            break;
        }

        const char *selected_project = project_names[selected];
        const char *yes_no[] = {"Yes", "No"};
        int confirm = selection("Are you sure you want to delete this project?", yes_no, 2);

        if (confirm != 0)
        {
            printf("Deletion canceled.\n");
            continue;
        }

        mkdir(".trash", 0700); 

        char old_path[512];
        snprintf(old_path, sizeof(old_path), "projects/%s", selected_project);

        char trash_path[512];
        snprintf(trash_path, sizeof(trash_path), ".trash/%s", selected_project);

        if (rename(old_path, trash_path) == 0)
        {
            printf("Project '%s' moved to trash successfully.\n", selected_project);
        }
        else
        {
            perror("Failed to delete project");
        }
    }
}

void recover_file_cmd(const char *filename)
{
    char trash_path[512];
    snprintf(trash_path, sizeof(trash_path), ".trash/%s", filename);

    if (access(trash_path, F_OK) != 0)
    {
        printf("File %s not found in .trash\n", filename);
        return;
    }

    if (rename(trash_path, filename) == 0)
    {
        printf("Recovered: %s\n", filename);
        fflush(stdout);
    }
    else
    {
        perror("Error recovering file.\n");
    }
}

int remove_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb;
    (void)ftwbuf;

    if (remove(fpath) != 0)
    {
        perror(fpath);
        return -1;
    }
    return 0;
}

void empty_trash()
{
    if (nftw(".trash", remove_callback, 64, FTW_DEPTH | FTW_PHYS) != 0)
    {
        perror("nftw");
    }
    else
    {
        puts("Trash emptied.");
    }
}

void print_prompt()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        if (strncmp(cwd, ROOT_FOLDER, strlen(ROOT_FOLDER)) == 0)
        {
            const char *relative = cwd + strlen(DESKTOP_PATH); 
            printf("%s%s> %s", STYLE_CYAN_BLUE, relative, STYLE_RESET);
        }
        else
        {
            printf("%s%s> %s", STYLE_CYAN_BLUE, cwd, STYLE_RESET);
        }
    }
    else
    {
        perror("getcwd failed");
        printf("> ");
    }
    fflush(stdout);
}

void download_template_cmd(const char *path) {
    download_template(path);
}