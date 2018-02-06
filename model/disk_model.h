#ifndef CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H
#define CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H

#include <stdlib.h>

#define DISK_MODEL_PAGE_SIZE (4096)

struct disk_model;

struct disk_model* disk_model_init(size_t max_page_count);
void disk_model_free(struct disk_model *model);
int disk_model_get(struct disk_model *model, size_t page_id, char *buf, size_t bufsize);
int disk_model_put(struct disk_model *model, const void *src, size_t n, size_t *page_id);


#endif //CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H
