#ifndef CACHE_REPLACEMENT_POLICIES_LRU_H
#define CACHE_REPLACEMENT_POLICIES_LRU_H

#include "../model/disk_model.h"

extern const struct disk_model_replacement_hooks lru_hooks;

void *lru_hook_replacement_metadata_init_page();

void lru_hook_replacement_metadata_free_page(void *metadata_page);

void *lru_hook_replacement_metadata_init_disk();

void lru_hook_replacement_metadata_free_disk(void *metadata_disk);

void lru_hook_page_access(size_t page_id, void *metadata_disk, void *metadata_page);

size_t lru_hook_page_replace(void *metadata_disk);

#endif //CACHE_REPLACEMENT_POLICIES_LRU_H
