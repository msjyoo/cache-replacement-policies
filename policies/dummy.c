#include "dummy.h"

#include <stdlib.h>

const struct disk_model_replacement_hooks dummy_hooks = {
        .hook_replacement_metadata_init_page = dummy_hook_replacement_metadata_init_page,
        .hook_replacement_metadata_free_page = dummy_hook_replacement_metadata_free_page,
        .hook_replacement_metadata_init_disk = dummy_hook_replacement_metadata_init_disk,
        .hook_replacement_metadata_free_disk = dummy_hook_replacement_metadata_free_disk,
        .hook_page_access = dummy_hook_page_access,
        .hook_page_replace = dummy_hook_page_replace,
};

void *dummy_hook_replacement_metadata_init_page()
{
        return NULL;
}

void dummy_hook_replacement_metadata_free_page(void *metadata_page)
{
        return; // NOLINT
}

void *dummy_hook_replacement_metadata_init_disk()
{
        return NULL;
}

void dummy_hook_replacement_metadata_free_disk(void *metadata_disk)
{
        return; // NOLINT
}

void dummy_hook_page_access(size_t page_id, void *metadata_disk, void *metadata_page)
{
        return; // NOLINT
}

size_t dummy_hook_page_replace(void *metadata_disk)
{
        return 0;
}