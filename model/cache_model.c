#include "disk_model.h"
#include "cache_model.h"

#include <stdlib.h>
#include <string.h>

struct cache_model_page {
        size_t disk_page_id;
        struct disk_model_page *disk_model_page;
};

struct cache_model {
        size_t max_pages;
        size_t cached_pages;
        struct cache_model_page *cache_pages;
};

struct cache_model* cache_model_init(size_t max_pages)
{
        struct cache_model *cache_model = malloc(sizeof(cache_model));
        if (cache_model == NULL) return NULL;

        cache_model->max_pages = max_pages;

        cache_model->cache_pages = malloc(sizeof(struct cache_model_page) * max_pages);
}


int cache_model_get(struct cache_model *cache_model, size_t disk_page_id, char *buf, size_t bufsize)
{
        // page_id 0..*, cache_page_size 1..*
        if (page_id >= cache_model->cache_page_size) {
                return -1;
        }

        memset(buf, 0, bufsize);

        if (cache_model->cached[page_id] == 1) {
                // TODO: stat cache hit
                memset(buf, 1, bufsize);
                return 0;
        } else {
                // TODO: stat cache miss
                memset(buf, 0, bufsize);
                return 1;
        }
}