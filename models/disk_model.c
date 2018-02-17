#include "disk_model.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static struct disk_model_page {
        size_t page_id;
        enum disk_model_page_status status;
        char data[DISK_MODEL_PAGE_SIZE];
        void *replacement_metadata_page;
};

static struct disk_model {
        size_t max_page_count;
        size_t used_page_count;
        size_t max_paged_count;
        size_t currently_paged_count;
        struct disk_model_replacement_hooks replacement_hooks;
        void *replacement_metadata_disk;
        struct disk_model_page *pages;
};

static void disk_model_replace_or_unused(struct disk_model *model);

struct disk_model *disk_model_init(
        size_t max_page_count, size_t max_paged_count, struct disk_model_replacement_hooks hooks)
{
        if (max_page_count < 1) return NULL;

        if (hooks.hook_replacement_metadata_init_page == NULL) return NULL;
        if (hooks.hook_replacement_metadata_init_disk == NULL) return NULL;
        if (hooks.hook_replacement_metadata_free_page == NULL) return NULL;
        if (hooks.hook_replacement_metadata_free_disk == NULL) return NULL;
        if (hooks.hook_page_replace == NULL) return NULL;

        struct disk_model *model = calloc(1, sizeof(struct disk_model));
        if (model == NULL) return NULL;
        memset(model, 0, sizeof(struct disk_model));

        model->max_page_count = max_page_count;
        model->used_page_count = 0;
        model->max_paged_count = max_paged_count;
        model->currently_paged_count = 0;
        model->replacement_hooks = hooks;
        model->replacement_metadata_disk = hooks.hook_replacement_metadata_init_disk();

        model->pages = calloc(max_page_count, sizeof(struct disk_model_page));
        if (model->pages == NULL) {
                free(model);
                return NULL;
        }

        for (size_t i = 0; i < max_page_count; i++) {
                struct disk_model_page *p = (model->pages + i);
                p->page_id = i;
                p->replacement_metadata_page = hooks.hook_replacement_metadata_init_page();
        }

        return model;

}

void disk_model_free(struct disk_model *model)
{
        for (size_t i = 0; i < model->max_page_count; i++) {
                struct disk_model_page *p = (model->pages + i);
                model->replacement_hooks.hook_replacement_metadata_free_page(p->replacement_metadata_page);
        }

        model->replacement_hooks.hook_replacement_metadata_free_disk(model->replacement_metadata_disk);

        free(model->pages);
        free(model);
}

int disk_model_get(struct disk_model *model, size_t page_id, char *buf, size_t bufsize)
{
        if (page_id >= model->max_page_count) {
                return -1;
        }

        struct disk_model_page *p = (model->pages + page_id);
        switch (p->status) {
                case DM_P_UNUSED:
                        return -1;

                case DM_P_PAGED:
                        if (model->replacement_hooks.hook_page_access != NULL) {
                                model->replacement_hooks.hook_page_access(
                                        page_id, model->replacement_metadata_disk,
                                        p->replacement_metadata_page
                                );
                        }
                        memcpy(buf, p->data, bufsize <= DISK_MODEL_PAGE_SIZE ? bufsize : DISK_MODEL_PAGE_SIZE);
                        return 0;

                case DM_P_UNPAGED: {
                        // Result doesn't matter since we're not doing actual page replacement
                        disk_model_replace_or_unused(model);
                        assert(model->currently_paged_count < model->max_paged_count);

                        (model->pages + page_id)->status = DM_P_PAGED;
                        model->currently_paged_count += 1;

                        if (model->replacement_hooks.hook_page_fault != NULL) {
                                model->replacement_hooks.hook_page_fault(
                                        page_id, model->replacement_metadata_disk,
                                        p->replacement_metadata_page
                                );
                        }
                        if (model->replacement_hooks.hook_page_access != NULL) {
                                model->replacement_hooks.hook_page_access(
                                        page_id, model->replacement_metadata_disk,
                                        p->replacement_metadata_page
                                );
                        }
                        memcpy(buf, p->data, bufsize <= DISK_MODEL_PAGE_SIZE ? bufsize : DISK_MODEL_PAGE_SIZE);
                        return 1;
                }

                default:
                        assert(0);
        }
}

int disk_model_put(struct disk_model *model, const void *src, size_t n, size_t *page_id)
{
        if (n > DISK_MODEL_PAGE_SIZE) {
                return -1; // Invalid page size
        }

        if (model->used_page_count >= model->max_page_count) {
                return -1; // No space left
        }

        size_t free_page_id = 0;
        struct disk_model_page *free_page = NULL;

        // TODO: Implement unused page list for doing this without scan
        for (free_page_id = 0; free_page_id < model->max_page_count; free_page_id++) {
                if ((model->pages + free_page_id)->status == DM_P_UNUSED) {
                        free_page = (model->pages + free_page_id);
                        break;
                }
        }

        assert(free_page != NULL); // used_page_count different from actual; mismatch error

        model->used_page_count += 1;
        free_page->status = DM_P_UNPAGED;
        memcpy(free_page->data, src, n);

        if (page_id != NULL) {
                *page_id = free_page_id;
        }

        return 0;
}

static void disk_model_replace_or_unused(struct disk_model *model)
{
        assert(model->currently_paged_count <= model->max_paged_count);
        if (model->currently_paged_count == model->max_paged_count) {
                // Replace
                size_t replace_page_id = model->replacement_hooks.hook_page_replace(
                        model->replacement_metadata_disk);
                (model->pages + replace_page_id)->status = DM_P_UNPAGED;
                model->currently_paged_count -= 1;
        }
        assert(model->currently_paged_count < model->max_paged_count);
}

enum disk_model_page_status disk_model_page_status(struct disk_model *model, size_t page_id)
{
        assert(model != NULL);
        if (page_id >= model->max_page_count) {
                abort();
        }

        struct disk_model_page *p = (model->pages + page_id);
        return p->status;
}