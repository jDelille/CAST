#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <ftw.h>

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

void copy_template_cmd()
{
    copy_template();
}

void delete_template_cmd()
{
    delete_template();
}

void clear_terminal_cmd()
{
    system("clear");
    return;
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
    else
    {
        printf("Changed directory to: %s\n", dir_name);
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

void delete_file_cmd(char *filename)
{
    mkdir(".trash", 0700);

    char trash_path[512];
    snprintf(trash_path, sizeof(trash_path), ".trash/%s", filename);

    if (rename(filename, trash_path) == 0)
    {
        printf("Moved %s to the trash\n", filename);
        fflush(stdout);
    }
    else
    {
        perror("Error deleting file\n");
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

/**
 * Add download & upload template commands
 * This will download a template from the docs website database
 * users can upload and download templates on the docs website
 * upload <template-path>
 * download <template-path>
 */
