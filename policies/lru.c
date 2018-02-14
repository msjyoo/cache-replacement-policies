#include "lru.h"

#include "../model/disk_model.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct lru_metadata_page {
        size_t page_id;
        time_t last_access_time;
        struct lru_metadata_page *prev;
        struct lru_metadata_page *next;
};

struct lru_metadata_disk {
        struct lru_metadata_page *lru_page;
        struct lru_metadata_page *mru_page;
};

const struct disk_model_replacement_hooks lru_hooks = {
        .hook_replacement_metadata_init_page = lru_hook_replacement_metadata_init_page,
        .hook_replacement_metadata_free_page = lru_hook_replacement_metadata_free_page,
        .hook_replacement_metadata_init_disk = lru_hook_replacement_metadata_init_disk,
        .hook_replacement_metadata_free_disk = lru_hook_replacement_metadata_free_disk,
        .hook_page_access = lru_hook_page_access,
        .hook_page_replace = lru_hook_page_replace,
};

void *lru_hook_replacement_metadata_init_page()
{
        struct lru_metadata_page *m = calloc(1, sizeof(struct lru_metadata_page));
        assert(m != NULL);

        return m;
}

void lru_hook_replacement_metadata_free_page(void *metadata_page)
{
        free(metadata_page);
}

void *lru_hook_replacement_metadata_init_disk()
{
        struct lru_metadata_disk *m = calloc(1, sizeof(struct lru_metadata_disk));
        assert(m != NULL);

        return m;
}

void lru_hook_replacement_metadata_free_disk(void *metadata_disk)
{
        free(metadata_disk);
}

void lru_hook_page_access(size_t page_id, void *metadata_disk, void *metadata_page)
{
        struct lru_metadata_disk *m_disk = metadata_disk;
        struct lru_metadata_page *m_page = metadata_page;

        assert(m_disk != NULL);
        assert(m_page != NULL);

        m_page->page_id = page_id;
        m_page->last_access_time = time(0);

        // Detach from list
        if (m_page->prev != NULL) m_page->prev->next = m_page->next;
        if (m_page->next != NULL) m_page->next->prev = m_page->prev;

        // Set neighbours, prevent circular list
        if (m_disk->mru_page != m_page) m_page->prev = m_disk->mru_page;
        m_page->next = NULL;

        if (m_disk->mru_page != NULL) {
                assert(m_disk->mru_page->next == NULL);
                // Prevent circular list
                if (m_disk->mru_page != m_page) m_disk->mru_page->next = m_page;
        }

        m_disk->mru_page = m_page;
        if (m_disk->lru_page == NULL) m_disk->lru_page = m_page;
}

size_t lru_hook_page_replace(void *metadata_disk)
{
        struct lru_metadata_disk *m_disk = metadata_disk;
        assert(m_disk != NULL);

        assert(m_disk->lru_page != NULL);
        size_t replace_page_id = m_disk->lru_page->page_id;
        struct lru_metadata_page *tmp = m_disk->lru_page;
        m_disk->lru_page = tmp->next;
        m_disk->lru_page->prev = NULL;
        tmp->prev = NULL;
        tmp->next = NULL;

        memset(tmp, 0, sizeof(struct lru_metadata_page));

        return replace_page_id;
}