#include "store.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <json-c/json.h>

#include "../utils/selection.h"

void store_init(MemoryStore *store)
{
    store->data = malloc(1);
    store->size = 0;
}

size_t store_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    size_t total = size * nmemb;
    MemoryStore *store = (MemoryStore *)userdata;

    char *new_data = realloc(store->data, store->size + total + 1);
    if (!new_data)
        return 0;

    store->data = new_data;
    memcpy(store->data + store->size, ptr, total);
    store->size += total;
    store->data[store->size] = '\0';

    return total;
}

char *store_parse_json(const char *json_string)
{
    struct json_object *parsed_json = json_tokener_parse(json_string);
    if (!parsed_json)
        return NULL;

    if (!json_object_is_type(parsed_json, json_type_array))
    {
        json_object_put(parsed_json);
        return NULL;
    }

    struct json_object *template_obj = json_object_array_get_idx(parsed_json, 0);
    if (!template_obj)
    {
        json_object_put(parsed_json);
        return NULL;
    }

    struct json_object *content_obj;
    if (!json_object_object_get_ex(template_obj, "content", &content_obj))
    {
        json_object_put(parsed_json);
        return NULL;
    }

    const char *content = json_object_get_string(content_obj);
    char *copy = strdup(content);
    json_object_put(parsed_json);
    return copy;
}

void store_save_template(const char *slug, const char *content)
{
    mkdir(".templates", 0755);

    const char *yes_no[] = {"Yes", "No"};
    int rename_choice = (selection("Would you like to rename this template?", yes_no, 2) == 0);

    char filename[200];
    if (rename_choice) {
        printf("Enter new template name (without extension): ");
        if (fgets(filename, sizeof(filename), stdin)) {
            size_t len = strlen(filename);
            if (len > 0 && filename[len - 1] == '\n') {
                filename[len - 1] = '\0';
            }
        } else {
            snprintf(filename, sizeof(filename), "%s", slug);
        }
    } else {
        snprintf(filename, sizeof(filename), "%s", slug);
    }

    char path[256];
    snprintf(path, sizeof(path), ".templates/%s.tmpl", filename);

    FILE *fp = fopen(path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create file: %s\n", path);
        return;
    }

    fputs(content, fp);
    fclose(fp);
    printf("Template saved to %s\n", path);
}