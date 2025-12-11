#include "supabase.h"
#include "store.h"
#include <stdio.h>
#include <stdlib.h>
#include <json-c/json.h>
#include <string.h>

// parse Supabase JSON array and get the content field
static char *parse_template_json(const char *json_str) {
    struct json_object *parsed_json = json_tokener_parse(json_str);
    if (!parsed_json) return NULL;

    if (json_object_get_type(parsed_json) != json_type_array) {
        json_object_put(parsed_json);
        return NULL;
    }

    struct json_object *first_item = json_object_array_get_idx(parsed_json, 0);
    if (!first_item) {
        json_object_put(parsed_json);
        return NULL;
    }

    struct json_object *content_obj = NULL;
    if (!json_object_object_get_ex(first_item, "content", &content_obj)) {
        json_object_put(parsed_json);
        return NULL;
    }

    const char *content_str = json_object_get_string(content_obj);
    char *result = strdup(content_str); 

    json_object_put(parsed_json);
    return result;
}

// Download a template by name from Supabase and save it locally
int download_template(const char *template_name) {
    char query[256];
    snprintf(query, sizeof(query), "title=eq.%s", template_name);

    char *json = supabase_get("templates", query);
    if (!json) {
        fprintf(stderr, "Failed to fetch template: %s\n", template_name);
        return 1;
    }

    char *content = parse_template_json(json);
    free(json);

    if (!content) {
        fprintf(stderr, "Failed to parse template JSON\n");
        return 1;
    }

    store_save_template(template_name, content);
    free(content);

    return 0;
}