#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <libgen.h>    
#include <dirent.h>
#include <errno.h>

/**
 * @brief Reads a single character from standard input without waiting for the Enter key
 *        and without echoing it to the terminal.
 *
 * This function temporarily disables canonical mode and echoing on the terminal
 * to allow for capturing a single key press immediately.
 *
 * @return int (The character code for the key pressed)
 */
int getch()
{
    struct termios oldt, newt;
    int ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

void sanitize_path(char *path)
{
    if (!path)
        return;

    // Remove trailing newline
    path[strcspn(path, "\n")] = '\0';

    size_t length = strlen(path);
    if (length >= 2 && path[0] == '"' && path[length - 1] == '"')
    {
        memmove(path, path + 1, length - 2);
        path[length - 2] = '\0';
        length -= 2;
    }

    // Trim leading spaces
    char *start = path;
    while (*start == ' ')
    {
        start++;
    }

    // Trim trailing spaces
    char *end = path + strlen(path) - 1;
    while (end > start && *end == ' ')
    {
        end--;
    }
    *(end + 1) = '\0';

    // Move cleaned string to the beginning
    if (start != path)
    {
        memmove(path, start, strlen(start) + 1);
    }
}

void convert_windows_path_to_wsl(char *file_path)
{

    bool is_windows_path =
        strlen(file_path) >= 3 &&
        file_path[1] == ':' &&
        (file_path[2] == '\\' || file_path[2] == '/');
    ;

    if (is_windows_path)
    {
        char wsl_path[512];
        snprintf(wsl_path, sizeof(wsl_path), "/mnt/%c/%s", tolower(file_path[0]), file_path + 3);
        strncpy(file_path, wsl_path, 512);
    }

    for (char *current_char = file_path; *current_char; ++current_char)
    {
        if (*current_char == '\\')
        {
            *current_char = '/';
        }
    }
}

char *get_filename(const char *path)
{
    char *path_copy = strdup(path);
    char *base_name = basename(path_copy);
    char *result = strdup(base_name);
    free(path_copy);
    return result;
}

bool ensure_template_dir_exists(void)
{
    return access(".templates", F_OK) == 0 || mkdir(".templates", 0700) == 0;
}

bool copy_file(const char *src_path, const char *dst_path)
{

    FILE *src_file = fopen(src_path, "r");
    if (!src_file)
    {
        perror("Error opening source");
        return false;
    }

    FILE *dst_file = fopen(dst_path, "w");
    if (!dst_file)
    {
        perror("Error creating dest");
        fclose(src_file);
        return false;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        fwrite(buffer, 1, bytes_read, dst_file);
    }

    fclose(src_file);
    fclose(dst_file);
    return true;
}

bool file_exists(const char *file_path)
{
    return access(file_path, F_OK) == 0;
}

void trim_trailing_whitespace(char *string)
{
    size_t length = strlen(string);
    while (length > 0 && isspace((unsigned char)string[length - 1]))
    {
        string[--length] = '\0';
    }
}

void ensure_parent_dirs(const char *file_path)
{
      char tmp[512];
    strncpy(tmp, file_path, sizeof(tmp));
    tmp[sizeof(tmp) - 1] = '\0';

    char *dir = dirname(tmp); // parent directory

    char path[512] = "";
    char *segment = strtok(dir, "/"); // split by slash

    while (segment) {
        if (strlen(path) > 0) {
            strncat(path, "/", sizeof(path) - strlen(path) - 1);
        }
        strncat(path, segment, sizeof(path) - strlen(path) - 1);

        // Create this segment if it doesn't exist
        if (mkdir(path, 0755) != 0 && errno != EEXIST) {
            perror("mkdir failed");
        }

        segment = strtok(NULL, "/");
    }
}

bool directory_exists(const char *dir_name) {
    struct stat statbuf;
    return (stat(dir_name, &statbuf) == 0 && S_ISDIR(statbuf.st_mode));
}

char* find_templates_directory() {
    char current_dir[512];

    // Start from the root of the project, specify this directory explicitly
    const char *root_directory = "/mnt/c/Users/justi/Desktop/raft";  

    // Set the current directory to root directory
    strncpy(current_dir, root_directory, sizeof(current_dir));
    current_dir[sizeof(current_dir) - 1] = '\0';  // Ensure null termination

    // Look for the .templates folder in the root directory or its subdirectories
    DIR *d = opendir(current_dir);
    if (d) {
        struct dirent *entry = readdir(d);
        while (entry) {
            if (strcmp(entry->d_name, ".templates") == 0) {
                closedir(d);
                return strdup(current_dir); 
            }
            entry = readdir(d);
        }
        closedir(d);
    }

    // If not found, print error
    printf("Error: .templates directory not found.\n");
    return NULL;
}

void replace_placeholders(
    const char *input_line,
    const char **placeholder_keys,
    const char **replacement_values,
    int key_count,
    char *output,
    size_t output_size)
{
    output[0] = '\0';
    const char *cursor = input_line;

    while (*cursor)
    {
        const char *start_bracket = strchr(cursor, '[');
        if (!start_bracket)
        {
            strncat(output, cursor, output_size - strlen(output) - 1);
            break;
        }

        strncat(output, cursor, start_bracket - cursor);

        const char *end_bracket = strchr(start_bracket, ']');
        if (!end_bracket)
        {
            strncat(output, start_bracket, output_size - strlen(output) - 1);
            break;
        }

        char placeholder[128];
        char default_value[128] = "";

        size_t key_len = end_bracket - start_bracket - 1;
        if (key_len >= sizeof(placeholder))
            key_len = sizeof(placeholder) - 1;
        strncpy(placeholder, start_bracket + 1, key_len);
        placeholder[key_len] = '\0';

        char *equal_sign = strchr(placeholder, '=');

        if (equal_sign)
        {
            *equal_sign = '\0';
            strncpy(default_value, equal_sign + 1, sizeof(default_value));
        }

        // Find replacement from user input
        const char *replacement = default_value;

        for (int i = 0; i < key_count; i++)
        {
            if (strcmp(placeholder, placeholder_keys[i]) == 0)
            {
                replacement = replacement_values[i];
                break;
            }
        }

        strncat(output, replacement, output_size - strlen(output) - 1);
        cursor = end_bracket + 1;
    }
}