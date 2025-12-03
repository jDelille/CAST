#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdlib.h>
#include "../utils/selection.h"

bool is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0)
        return false;
    return S_ISDIR(st.st_mode);
}

int list_projects(char projects_list[][256], int max_projects) {
    DIR *d = opendir("projects");
    if (!d) return 0;

    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(d)) != NULL && count < max_projects) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char full_path[512];
        snprintf(full_path, sizeof(full_path), "projects/%s", entry->d_name);

        if (is_directory(full_path)) {
            strncpy(projects_list[count], entry->d_name, 255);
            projects_list[count][255] = '\0';
            count++;
        }
    }
    closedir(d);
    return count;
}

bool copy_folder(const char *src_path, const char *dest_path) {
    struct stat st = {0};
    if (stat(dest_path, &st) == -1) {
        mkdir(dest_path, 0755);
    }

    DIR *dir = opendir(src_path);
    if (!dir) return false;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char src_file[512], dest_file[512];
        snprintf(src_file, sizeof(src_file), "%s/%s", src_path, entry->d_name);
        snprintf(dest_file, sizeof(dest_file), "%s/%s", dest_path, entry->d_name);

        struct stat st_entry;
        stat(src_file, &st_entry);

        if (S_ISDIR(st_entry.st_mode)) {
            if (!copy_folder(src_file, dest_file)) {
                closedir(dir);
                return false;
            }
        } else {
            FILE *src = fopen(src_file, "rb");
            FILE *dst = fopen(dest_file, "wb");
            if (!src || !dst) {
                if (src) fclose(src);
                if (dst) fclose(dst);
                closedir(dir);
                return false;
            }

            char buffer[4096];
            size_t bytes;
            while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, bytes, dst);
            }

            fclose(src);
            fclose(dst);
        }
    }

    closedir(dir);
    return true;
}

void copy_project() {
    char projects_list[64][256];
    int num_projects = list_projects(projects_list, 64);

    if (num_projects <= 0) {
        printf("No projects found in the projects folder.\n");
        return;
    }

    const char *project_names[64];
    for (int i = 0; i < num_projects; i++)
        project_names[i] = projects_list[i];

    int selection_index = selection("Which project do you want to copy?", project_names, num_projects);
    if (selection_index < 0) return;

    const char *selected_project = project_names[selection_index];

    char new_name[256];
    printf("Enter a new name for the copied project: ");
    if (scanf("%255s", new_name) != 1) {
        printf("Invalid input.\n");
        return;
    }
    new_name[255] = '\0';

    char src_path[512];
    char dst_path[512];
    snprintf(src_path, sizeof(src_path), "projects/%s", selected_project);
    snprintf(dst_path, sizeof(dst_path), "projects/%s", new_name);

    if (is_directory(dst_path)) {
        printf("Error: A project named '%s' already exists.\n", new_name);
        return;
    }

    if (copy_folder(src_path, dst_path)) {
        printf("Project copied successfully to '%s'\n", dst_path);
    } else {
        printf("Failed to copy project.\n");
    }
}

void rename_project() {
    char projects_list[64][256];
    int num_projects = list_projects(projects_list, 64);

    if (num_projects <= 0) {
        printf("No projects found in the projects folder.\n");
        return;
    }

     const char *project_names[64];
    for (int i = 0; i < num_projects; i++)
        project_names[i] = projects_list[i];

    int selection_index = selection("Which project do you want to rename?", project_names, num_projects);
    if (selection_index < 0) return;

    const char *selected_project = project_names[selection_index];

    char new_name[256];
    printf("Enter a new name for the project '%s': ", selected_project);
    if (scanf("%255s", new_name) != 1) {
        printf("Invalid input.\n");
        return;
    }
    new_name[255] = '\0';

    char old_path[512], new_path[512];
    snprintf(old_path, sizeof(old_path), "projects/%s", selected_project);
    snprintf(new_path, sizeof(new_path), "projects/%s", new_name);

    if (is_directory(new_path)) {
        printf("Error: A project named '%s' already exists.\n", new_name);
        return;
    }

    if (rename(old_path, new_path) == 0) {
        printf("Project '%s' successfully renamed to '%s'.\n", selected_project, new_name);
    } else {
        perror("Failed to rename project");
    }

}