#include "supabase.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct memory {
    char *buffer;
    size_t size;
};

static size_t write_callback(void *data, size_t size, size_t count, void *user_data) {
    size_t total = size * count;
    struct memory *mem = user_data;

    if (!mem) return 0;

    char *new_buffer = realloc(mem->buffer, mem->size + total + 1);
    if (!new_buffer) return 0; 

    mem->buffer = new_buffer;
    memcpy(mem->buffer + mem->size, data, total);
    mem->size += total;
    mem->buffer[mem->size] = '\0';

    return total;
}

char *supabase_get(const char *table, const char *query) {
    const char *url = getenv("SUPABASE_URL");
    if (!url) {
        fprintf(stderr, "Error: SUPABASE_URL not set\n");
        return NULL;
    }

    const char *key = getenv("SUPABASE_API_KEY");
    if (!key) {
        fprintf(stderr, "Error: SUPABASE_API_KEY not set\n");
        return NULL;
    }

    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    char full_url[512];
    snprintf(full_url, sizeof(full_url), "%s/%s?select=*&%s", url, table, query);

    struct memory response = { .buffer = NULL, .size = 0 };

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char api_header[256];
    snprintf(api_header, sizeof(api_header), "apikey: %s", key);
    headers = curl_slist_append(headers, api_header);

    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", key);
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, full_url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        free(response.buffer);
        fprintf(stderr, "CURL failed: %s\n", curl_easy_strerror(res));
        return NULL;
    }

    return response.buffer; // remember to free!
}