#ifndef TEMPLATES_H
#define TEMPLATES_H

#include <stdbool.h>

void install_template(const char *templatePath);

bool file_exists(const char *path);

void convert_windows_path_to_wsl(char *path);

void copy_template();

void delete_template();

void generate_project_from_template(
    const char *templateName,
    const char *projectName,
    bool customize);

int extract_placeholders_from_template(
    const char *templateName,
    char placeholder_keys[][128],
    char placeholder_defaults[][128],
    int max_placeholders);

#endif /* TEMPLATES_H */