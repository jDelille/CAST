#ifndef COMMANDS_H
#define COMMANDS_H

void create_project_cmd();
void install_template_cmd(const char *path);
void copy_template_cmd();
void delete_template_cmd();
void clear_terminal_cmd();
void cd_cmd(const char *project_folder);
void view_cmd(const char *path);
void delete_file_cmd(char *filename);
void recover_file_cmd(const char *filename);
void empty_trash(); 
void print_prompt();
void show_help();
void copy_project_cmd();
void create_template_cmd();
void rename_project_cmd();
void view_projects_cmd();

#endif 