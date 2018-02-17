#ifndef CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H
#define CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H

#include <stdlib.h>

#define DISK_MODEL_PAGE_SIZE (4096)

enum disk_model_page_status {
        DM_P_UNUSED = 0,
        DM_P_UNPAGED,
        DM_P_PAGED
};

struct disk_model_replacement_hooks {
        void *(*hook_replacement_metadata_init_page)();

        void (*hook_replacement_metadata_free_page)(void *metadata_page);

        void *(*hook_replacement_metadata_init_disk)();

        void (*hook_replacement_metadata_free_disk)(void *metadata_disk);

        size_t (*hook_page_replace)(void *metadata_disk);

        /**
         * Access implies that this page has just been paged, or was already paged prior.
         *
         * @param page_id
         * @param metadata_disk
         * @param metadata_page
         */
        void (*hook_page_access)(size_t page_id, void *metadata_disk, void *metadata_page);

        /**
         * Fault implies that this page has just been paged, and was NOT resident prior.
         *
         * @param page_id
         * @param metadata_disk
         * @param metadata_page
         */
        void (*hook_page_fault)(size_t page_id, void *metadata_disk, void *metadata_page);
};

struct disk_model;

struct disk_model *disk_model_init(
        size_t max_page_count, size_t max_paged_count, struct disk_model_replacement_hooks hooks);

void disk_model_free(struct disk_model *model);

int disk_model_get(struct disk_model *model, size_t page_id, char *buf, size_t bufsize);

int disk_model_put(struct disk_model *model, const void *src, size_t n, size_t *page_id);

enum disk_model_page_status disk_model_page_status(struct disk_model *model, size_t page_id);

#endif //CACHE_REPLACEMENT_POLICIES_DISK_MODEL_H
