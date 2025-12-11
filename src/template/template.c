#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>

#include "../utils/defs.h"
#include "../utils/utils.h"
#include "../utils/selection.h"
#include "../scaffold/scaffold.h"

#define MAX_GLOBAL_PLACEHOLDERS 128
#define MAX_PENDING_FILES 256

typedef struct
{
    char path[512];
    long section_start;
} PendingFile;

typedef struct
{
    char key[128];
    char value[128];
    char default_value[128];
    bool filled;
    char file_name[256];
} Placeholder;

PendingFile files[MAX_PENDING_FILES];
int file_count = 0;

void install_template(const char *template_path)
{
    char converted_path[512];
    snprintf(converted_path, sizeof(converted_path), "%s", template_path);
    convert_windows_path_to_wsl(converted_path);

    if (!ensure_template_dir_exists())
        return;

    if (!file_exists(converted_path))
    {
        printf("Error: The file %s does not exist\n", converted_path);
        return;
    }

    char *filename = get_filename(converted_path);

    char destination_path[512];
    snprintf(destination_path, sizeof(destination_path), ".templates/%s", filename);
    free(filename);

    if (file_exists(destination_path))
    {
        printf("Template already exists: %s\n", destination_path);
        return;
    }

    if (copy_file(converted_path, destination_path))
        printf("Installed template: %s\n", destination_path);
}

const char *get_placeholder(
    const char *placeholder_name,
    const char *placeholder_keys[],
    const char *placeholder_values[],
    int num_placeholders)
{
    for (int i = 0; i < num_placeholders; i++)
    {
        if (strcmp(placeholder_name, placeholder_keys[i]) == 0)
        {
            return placeholder_values[i];
        }
    }
    return "";
}

void create_dir_from_line(const char *line, const char *projectName)
{
    while (*line == ' ' || *line == '/')
    {
        line++;
    }

    if (*line == '\0')
    {
        return;
    }

    size_t len = strlen(line);

    if (len == 0 || line[len - 1] != '/')
    {
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%.*s", projectName, (int)(len - 1), line); // remove trailing '/'

    mkdir(path, 0755);
}

void create_file_from_line(
    FILE *template_fp,
    char *output_path,
    const char **placeholder_keys,
    const char **replacement_values,
    int key_count)
{
    trim_trailing_whitespace(output_path);
    ensure_parent_dirs(output_path);

    FILE *output_fp = fopen(output_path, "w");
    if (!output_fp)
    {
        perror("File create failed");
        return;
    }

    char input_line[1024];
    char processed_line[4096];

    while (fgets(input_line, sizeof(input_line), template_fp))
    {
        if (strncmp(input_line, "-- ", 3) == 0)
        {
            fseek(template_fp, -strlen(input_line), SEEK_CUR);
            break;
        }

        replace_placeholders(
            input_line,
            placeholder_keys,
            replacement_values,
            key_count,
            processed_line,
            sizeof(processed_line));

        fputs(processed_line, output_fp);
    }

    fclose(output_fp);

    chmod(output_path, 0644);
}

int extract_placeholders_from_template(
    const char *template_name,
    char placeholder_keys[][128],
    char placeholder_defaults[][128],
    int max_placeholders)
{
    char template_path[512];
    snprintf(template_path, sizeof(template_path), "/mnt/c/Users/justi/Desktop/beam/.templates/%s", template_name);

    FILE *file = fopen(template_path, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Couldn't open template file %s\n", template_path);
        return 0;
    }

    char line[1024];
    int placeholder_count = 0;

    while (fgets(line, sizeof(line), file))
    {
        const char *cursor = line;

        while ((cursor = strchr(cursor, '[')) != NULL)
        {
            const char *end_bracket = strchr(cursor, ']');
            if (!end_bracket) {
                break;
            }
                
            char content[256];
            size_t content_len = end_bracket - cursor - 1;
            if (content_len >= sizeof(content)) {
                content_len = sizeof(content) - 1;
            }
                
            strncpy(content, cursor + 1, content_len);
            content[content_len] = '\0';

            char *equal_sign = strchr(content, '=');
            char key[128] = "";
            char default_value[128] = "";

            if (equal_sign)
            {
                *equal_sign = '\0';
                strncpy(key, content, sizeof(key));
                strncpy(default_value, equal_sign + 1, sizeof(default_value));
            }
            else
            {
                strncpy(key, content, sizeof(key));
                default_value[0] = '\0';
            }

            key[strcspn(key, " \r\n\t")] = 0;
            default_value[strcspn(default_value, " \r\n\t")] = 0;

            if (strlen(key) == 0)
            {
                cursor = end_bracket + 1;
                continue;
            }

            bool already_exists = false;
            for (int i = 0; i < placeholder_count; i++)
            {
                if (strcmp(placeholder_keys[i], key) == 0)
                {
                    already_exists = true;
                    break;
                }
            }

            if (!already_exists && placeholder_count < max_placeholders)
            {
                strncpy(placeholder_keys[placeholder_count], key, sizeof(placeholder_keys[0]));
                strncpy(placeholder_defaults[placeholder_count], default_value, sizeof(placeholder_defaults[0]));
                placeholder_count++;
            }

            cursor = end_bracket + 1;
        }
    }

    fclose(file);
    return placeholder_count;
}

void copy_template()
{
    char templates_list[64][256];
    int num_templates = list_templates(templates_list, 64);

    if (num_templates <= 0) {
        return;
    }
        
    const char *template_names[64];
    for (int i = 0; i < num_templates; i++) {
        template_names[i] = templates_list[i];
    }
        
    int template_selection = selection("Which template do you want to copy?", template_names, num_templates);
    if (template_selection < 0) {
        return;
    }

    const char *filename = template_names[template_selection];
    char src_path[512];
    snprintf(src_path, sizeof(src_path), ".templates/%s", filename);

    char new_name[512];
    printf("Enter a new filename for the copied template: ");
    if (scanf("%255s", new_name) != 1)
    {
        printf("Invalid input.\n");
        return;
    }

    new_name[sizeof(new_name) - 1] = '\0';

    char dest_path[1024];
    snprintf(dest_path, sizeof(dest_path), ".templates/%s.tmpl", new_name);

    if (file_exists(dest_path))
    {
        printf("Error: A template named '%s' already exists.\n", new_name);
        return;
    }

    if (copy_file(src_path, dest_path))
        printf("Template copied successfully to %s\n", dest_path);
    else
        printf("Failed to copy template.\n");
}

void delete_template()
{
    while (1)
    {
        char templates_list[64][256];
        int num_templates = list_templates(templates_list, 64);

        if (num_templates == 0)
        {
            printf("No templates found to delete.\n");
            return;
        }

        const char *template_names[65];
        for (int i = 0; i < num_templates; i++) {
            template_names[i] = templates_list[i];
        }
            
        template_names[num_templates] = "Quit";

        int selected = selection("Which template do you want to delete?", template_names, num_templates + 1);
        if (selected == num_templates || selected < 0)
        {
            break;
        }

        const char *selected_template = template_names[selected];
        const char *yes_no[] = {"Yes", "No"};
        int confirm = selection("Are you sure you want to delete this template?", yes_no, 2);

        if (confirm != 0)
        {
            printf("Deletion canceled.\n");
            continue;
        }

        char path[512];
        snprintf(path, sizeof(path), ".templates/%s", selected_template);

        if (remove(path) == 0)
        {
            printf("Template '%s' deleted successfully.\n", selected_template);
        }
        else
        {
            perror("Failed to delete template");
        }
    }
}

void rename_template()
{
    char templates_list[64][256];
    int num_templates = list_templates(templates_list, 64);

    if (num_templates <= 0)
    {
        printf("No templates found in the .templates folder.\n");
        return;
    }

    const char *template_names[64];
    for (int i = 0; i < num_templates; i++) {
        template_names[i] = templates_list[i];
    }
        

    int selection_index = selection("Which template do you want to rename?", template_names, num_templates);
    if (selection_index < 0) {
        return;
    }
        
    const char *selected_template = template_names[selection_index];

    char new_name[256];
    printf("Enter a new name for the template '%s': ", selected_template);
    if (scanf("%255s", new_name) != 1)
    {
        printf("Invalid input.\n");
        return;
    }
    new_name[255] = '\0';

    char base_name[256];
    strncpy(base_name, selected_template, 255);
    base_name[255] = '\0';

    char *ext = strstr(base_name, ".tmpl");
    if (ext) {
        *ext = '\0';
    }

    char old_path[512], new_path[512];
    snprintf(old_path, sizeof(old_path), ".templates/%s.tmpl", base_name);
    snprintf(new_path, sizeof(new_path), ".templates/%s.tmpl", new_name);

    if (access(new_path, F_OK) == 0)
    {
        printf("Error: A template named '%s' already exists.\n", new_name);
        return;
    }

    if (rename(old_path, new_path) == 0)
    {
        printf("Template '%s' successfully renamed to '%s'.\n", selected_template, new_name);
    }
    else
    {
        perror("Failed to rename template");
    }
}

void create_template()
{
    char template_name[256];
    char folder[] = ".templates";
    char filepath[512];

    printf("What is your template named? » ");
    fflush(stdout);

    if (!fgets(template_name, sizeof(template_name), stdin))
    {
        printf("Failed to read template name.\n");
        return;
    }

    template_name[strcspn(template_name, "\n")] = 0;

    if (strlen(template_name) == 0)
    {
        printf("Template name cannot be empty.\n");
        return;
    }

    if (mkdir(folder, 0755) != 0 && errno != EEXIST)
    {
        perror("Error creating .templates folder");
        return;
    }

    snprintf(filepath, sizeof(filepath), "%s/%s.tmpl", folder, template_name);

    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        perror("Error creating template file");
        return;
    }

    fprintf(file, "// %s.tmpl\n", template_name);
    fprintf(file, "// Sample template showing placeholders and structure\n\n");

    fprintf(file,
            "-- app/Page.tsx\n"
            "export default function [PAGE_COMPONENT=Page]() {\n"
            "    return (\n"
            "        <div className=\"[PAGE_CLASSNAME=page]\">\n"
            "            <[NAVBAR_COMPONENT=Navbar] />\n"
            "        </div>\n"
            "    );\n"
            "}\n\n");

    fprintf(file,
            "-- app/components/reusable/button/Button.tsx\n"
            "const [BUTTON_COMPONENT=Button] = ({ text, onClick }) => (\n"
            "    <button onClick={onClick}>{text}</button>\n"
            ");\n\n"
            "export default [BUTTON_COMPONENT];\n");

    fclose(file);
    printf("Template '%s' created successfully in '%s'!\n", template_name, folder);
}

void generate_project_from_template(const char *templateName, const char *projectName, bool customize)
{
    char templatePath[512];
    snprintf(templatePath, sizeof(templatePath), "/mnt/c/Users/justi/Desktop/beam/.templates/%s", templateName);

    FILE *templateFile = fopen(templatePath, "r");
    if (!templateFile)
    {
        printf("Template open failed: %s\n", templatePath);
        return;
    }

    char project_path[512];

    char cwd[512];
    getcwd(cwd, sizeof(cwd));

    bool in_projects_dir = false;

    const char *last = strrchr(cwd, '/');
    if (last && strcmp(last + 1, "projects") == 0)
    {
        in_projects_dir = true;
    }

    if (in_projects_dir)
    {
        snprintf(project_path, sizeof(project_path), "%s", projectName);
    }
    else
    {
        snprintf(project_path, sizeof(project_path), "projects/%s", projectName);
    }

    if (!in_projects_dir)
    {
        struct stat st;
        if (stat("projects", &st) != 0)
        {
            mkdir("projects", 0755);
        }
    }

    mkdir(project_path, 0755);

    char line[1024];
    Placeholder global_placeholders[MAX_GLOBAL_PLACEHOLDERS];
    int global_count = 0;
    file_count = 0;

    while (fgets(line, sizeof(line), templateFile))
    {
        if (strncmp(line, "//", 2) == 0) {
            continue;
        }
            

        if (strncmp(line, "--", 2) != 0) {
            continue;
        }
            

        const char *file_path = line + 2;
        while (*file_path == ' ') {
            file_path++;
        }
            
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", project_path, file_path);
        full_path[strcspn(full_path, "\r\n")] = 0;

        ensure_parent_dirs(full_path);

        long section_start = ftell(templateFile);
        char section_line[1024];

        strncpy(files[file_count].path, full_path, sizeof(files[file_count].path));
        files[file_count].section_start = section_start;
        file_count++;

        while (fgets(section_line, sizeof(section_line), templateFile))
        {
            if (strncmp(section_line, "--", 2) == 0)
            {
                fseek(templateFile, -strlen(section_line), SEEK_CUR);
                break;
            }

            const char *cursor = section_line;
            while ((cursor = strchr(cursor, '[')) != NULL)
            {
                const char *end_bracket = strchr(cursor, ']');
                if (!end_bracket) {
                    break;
                }

                char content[256];
                size_t len = end_bracket - cursor - 1;
                if (len >= sizeof(content)) {
                    len = sizeof(content) - 1;
                }
                    
                strncpy(content, cursor + 1, len);
                content[len] = '\0';

                char *equal_sign = strchr(content, '=');
                char key[128] = "";
                char default_val[128] = "";

                if (equal_sign)
                {
                    *equal_sign = '\0';
                    strncpy(key, content, sizeof(key));
                    strncpy(default_val, equal_sign + 1, sizeof(default_val));
                }
                else
                {
                    strncpy(key, content, sizeof(key));
                    default_val[0] = '\0';
                }

                key[strcspn(key, " \r\n\t")] = 0;
                default_val[strcspn(default_val, " \r\n\t")] = 0;

                if (strlen(key) == 0)
                {
                    cursor = end_bracket + 1;
                    continue;
                }

                bool exists = false;
                for (int i = 0; i < global_count; i++)
                {
                    if (strcmp(global_placeholders[i].key, key) == 0)
                    {
                        exists = true;
                        break;
                    }
                }

                if (!exists && global_count < MAX_GLOBAL_PLACEHOLDERS)
                {
                    strncpy(global_placeholders[global_count].key, key, sizeof(global_placeholders[0].key));
                    strncpy(global_placeholders[global_count].default_value, default_val, sizeof(global_placeholders[0].default_value));
                    strncpy(global_placeholders[global_count].value, default_val, sizeof(global_placeholders[0].value));
                    global_placeholders[global_count].filled = false;
                    global_count++;
                }

                cursor = end_bracket + 1;
            }
        }
    }

    if (customize)
    {
        printf("\n");
        for (int i = 0; i < global_count; i++)
        {
            if (!global_placeholders[i].filled)
            {
                printf("%s [%s]: ", global_placeholders[i].key, global_placeholders[i].default_value);
                char input[128];
                fgets(input, sizeof(input), stdin);
                input[strcspn(input, "\n")] = 0;
                if (strlen(input) > 0)
                    strncpy(global_placeholders[i].value, input, sizeof(global_placeholders[i].value));
                global_placeholders[i].filled = true;
            }
        }
    }

    const char *key_ptrs[MAX_GLOBAL_PLACEHOLDERS];
    const char *replacement_ptrs[MAX_GLOBAL_PLACEHOLDERS];
    for (int i = 0; i < global_count; i++)
    {
        key_ptrs[i] = global_placeholders[i].key;
        replacement_ptrs[i] = global_placeholders[i].value;
    }

    printf("\n");

    for (int i = 0; i < file_count; i++)
    {
        printf("Creating file: %s\n", files[i].path);

        fseek(templateFile, files[i].section_start, SEEK_SET);

        create_file_from_line(
            templateFile,
            files[i].path,
            key_ptrs,
            replacement_ptrs,
            global_count);
    }

    printf("\n🚀 Project %s created using the %s template\n\n", projectName, templateName);
    fclose(templateFile);
}