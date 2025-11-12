#ifndef COMMANDS_H
#define COMMANDS_H

void create_project_cmd();
void install_template_cmd(const char *path);
void generate_proj_from_template(const char *templateName, const char *projectName);
void copy_template_cmd();
void delete_template_cmd();
#endif 