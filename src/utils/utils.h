#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

int getch();

void trim_trailing_whitespace(char *string);
void ensure_parent_dirs(const char *file_path);
void replace_placeholders(
    const char *input_line,
    const char **placeholder_keys,
    const char **replacement_values,
    int key_count,
    char *output,
    size_t output_size);
    
void sanitize_path(char *path);
void convert_windows_path_to_wsl(char *file_path);

bool file_exists(const char *file_path);
bool ensure_template_dir_exists(void);
bool ensure_projects_dir_exists(void);
bool copy_file(const char *src_path, const char *dst_path);
bool directory_exists(const char *dir_name) ;

char *get_filename(const char *path);
char* find_templates_directory();

#endif