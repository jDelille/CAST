#include "supabase.h"
#include "store.h"
#include <stdio.h>
#include <stdlib.h>

int download_template(const char *slug) {
    char query[256];
    snprintf(query, sizeof(query), "title=eq.%s", slug);

    char *json = supabase_get("templates", query);
    if (!json) {
        fprintf(stderr, "Failed to fetch template: %s\n", slug);
        return 1;
    }

    char *content = store_parse_json(json);
    free(json);

    if (!content) {
        fprintf(stderr, "Failed to parse template JSON\n");
        return 1;
    }

    store_save_template(slug, content);
    free(content);

    return 0;
}