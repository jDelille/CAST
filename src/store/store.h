#ifndef STORE_H
#define STORE_H

#include <stddef.h>

typedef struct {
    char *data;
    size_t size;
} MemoryStore;

void store_init(MemoryStore *store);
size_t store_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata);
char* store_parse_json(const char *json_string);
void store_save_template(const char *slug, const char *content);

#endif