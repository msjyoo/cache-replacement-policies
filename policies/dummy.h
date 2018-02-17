#ifndef CACHE_REPLACEMENT_POLICIES_DUMMY_H
#define CACHE_REPLACEMENT_POLICIES_DUMMY_H

#include "../models/disk_model.h"

#include <stdlib.h>

extern const struct disk_model_replacement_hooks dummy_hooks;

void *dummy_hook_replacement_metadata_init_page();

void dummy_hook_replacement_metadata_free_page(void *metadata_page);

void *dummy_hook_replacement_metadata_init_disk();

void dummy_hook_replacement_metadata_free_disk(void *metadata_disk);

void dummy_hook_page_access(size_t page_id, void *metadata_disk, void *metadata_page);

size_t dummy_hook_page_replace(void *metadata_disk);

#endif //CACHE_REPLACEMENT_POLICIES_DUMMY_H
