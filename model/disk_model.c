#include "disk_model.h"

#include <stdlib.h>
#include <string.h>

enum disk_model_page_status {
        UNUSED = 0,
        USED,
        PAGED
};

struct disk_model_page {
        enum disk_model_page_status status;
        char data[DISK_MODEL_PAGE_SIZE];
};

struct disk_model {
        size_t max_page_count;
        size_t used_page_count;
        struct disk_model_page *pages;
};

struct disk_model* disk_model_init(size_t max_page_count)
{
        struct disk_model *model = malloc(sizeof(struct disk_model));
        if (model == NULL) return NULL;

        model->max_page_count = max_page_count;

        model->pages = calloc(model->max_page_count, sizeof(struct disk_model_page));
        if (model->pages == NULL) {
                free(model);
                return NULL;
        }
        memset(model->pages, 0, model->max_page_count);

        return model;

}

void disk_model_free(struct disk_model *model)
{
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
                case UNUSED:
                        return -1;

                case PAGED:
                        abort();
                        return 0;

                case USED:
                        memcpy(buf, p->data, bufsize <= DISK_MODEL_PAGE_SIZE ? bufsize : DISK_MODEL_PAGE_SIZE);
                        return 0;

                default:
                        abort();
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

        for (free_page_id = 0; free_page_id < model->max_page_count; free_page_id++) {
                if ((model->pages + free_page_id)->status == UNUSED) {
                        free_page = (model->pages + free_page_id);
                        break;
                }
        }

        if (free_page == NULL) {
                // TODO: Page count mismatch error
                return -1; // No space left;
        }

        model->used_page_count += 1;
        free_page->status = USED;
        memcpy(free_page->data, src, n);

        if (page_id != NULL) {
                *page_id = free_page_id;
        }

        return 0;
}