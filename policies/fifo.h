#ifndef CACHE_REPLACEMENT_POLICIES_FIFO_H
#define CACHE_REPLACEMENT_POLICIES_FIFO_H

#include "../models/disk_model.h"

#include <stdlib.h>

extern const struct disk_model_replacement_hooks fifo_hooks;

void *fifo_hook_replacement_metadata_init_page();

void fifo_hook_replacement_metadata_free_page(void *metadata_page);

void *fifo_hook_replacement_metadata_init_disk();

void fifo_hook_replacement_metadata_free_disk(void *metadata_disk);

void fifo_hook_page_fault(size_t page_id, void *metadata_disk, void *metadata_page);

size_t fifo_hook_page_replace(void *metadata_disk);

#endif //CACHE_REPLACEMENT_POLICIES_FIFO_H
