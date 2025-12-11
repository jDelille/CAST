#include "supabase.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SUPABASE_URL "https://jqtqviwsnmxgjzglrhbt.supabase.co/rest/v1"
#define API_KEY "sb_publishable_noNFgBeg1X9CbPUItrokYA_4z5wQ6wW"

struct memory {
    char *response;
    size_t size;
};

static size_t write_mem(void *ptr, size_t size, size_t nmemb, void *userdata) {
    size_t total = size * nmemb;
    struct memory *mem = userdata;

    char *new_resp = realloc(mem->response, mem->size + total + 1);
    if (!new_resp) return 0;

    mem->response = new_resp;
    memcpy(mem->response + mem->size, ptr, total);
    mem->size += total;
    mem->response[mem->size] = '\0';

    return total;
}

char *supabase_get(const char *table, const char *query) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    char url[512];
    snprintf(url, sizeof(url), "%s/%s?select=*&%s", SUPABASE_URL, table, query);

    struct memory mem = {0};

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "apikey: " API_KEY);
    headers = curl_slist_append(headers, "Authorization: Bearer " API_KEY);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mem);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        free(mem.response);
        return NULL;
    }

    return mem.response; // caller frees this
}