#include "lru.h"

#include "../models/disk_model.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

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

        /*
         * Delete from doubly-linked list
         */

        /*
         * Cases
         * First:
         *      - Remove reference from next, if exists
         *      - Remove as first, set next as first
         *      - If last, set next as last
         * Middle:
         *      - Remove reference from prev, if exists
         *      - Remove reference from next, if exists
         * Last:
         *      - Remove reference from prev, if exists
         *      - Remove as last, set prev as next
         *      - If first, set prev as first
         *
         * In short:
         *      - If exists, remove reference from prev, set prev->next to next
         *      - If exists, remove reference from next, set next->prev to prev
         *      - If first, set prev as first
         *      - If last, set next as last
         */

        if (m_page->prev != NULL) m_page->prev->next = m_page->next;
        if (m_page->next != NULL) m_page->next->prev = m_page->prev;

        if (m_page == m_disk->lru_page) m_disk->lru_page = m_page->prev;
        if (m_page == m_disk->mru_page) m_disk->mru_page = m_page->next;

        m_page->prev = NULL;
        m_page->next = NULL;

        /*
         * Insert to doubly-linked list
         */

        /*
         * Cases:
         * Empty:
         *      - Set LRU to m_page
         *      - Set MRU to m_page
         *      - Leave prev, next = NULL
         * One+:
         *      - Set MRU->next to m_page
         *      - Set MRU to m_page
         *      - Set prev to previous MRU
         *
         * In short:
         *      - If LRU == NULL, set LRU = m_page
         *      - If MRU?, MRU->next = m_page
         *      - m_page->prev = MRU
         *      - Set MRU = m_page
         */

        if (m_disk->lru_page == NULL) m_disk->lru_page = m_page;
        if (m_disk->mru_page != NULL) m_disk->mru_page->next = m_page;
        m_page->prev = m_disk->mru_page;
        m_disk->mru_page = m_page;
}

size_t lru_hook_page_replace(void *metadata_disk)
{
        struct lru_metadata_disk *m_disk = metadata_disk;
        assert(m_disk != NULL);

        assert(m_disk->lru_page != NULL);
        size_t replace_page_id = m_disk->lru_page->page_id;
        struct lru_metadata_page *tmp = m_disk->lru_page;

        /*
         * Pop from head of doubly-linked list
         *
         * Cases
         * Empty: Impossible
         * One:
         *      - Set LRU to NULL (also LRU->next)
         *      - Set MRU to NULL
         * Two+:
         *      - Set LRU to LRU->next
         *      - Set the new LRU's ->prev to NULL
         *
         * In short:
         *      - If MRU, set MRU to NULL
         *      - If LRU->next?, set LRU->next->prev to NULL
         *      - Set LRU to LRU->next
         */

        if (m_disk->mru_page == m_disk->lru_page) m_disk->mru_page = NULL;
        if (m_disk->lru_page->next != NULL) m_disk->lru_page->next->prev = NULL;
        m_disk->lru_page = m_disk->lru_page->next;

        // Zero out unused metadata
        memset(tmp, 0, sizeof(struct lru_metadata_page));

        return replace_page_id;
}

void lru_dump(void *metadata_disk)
{
        struct lru_metadata_disk *m_disk = metadata_disk;
        struct lru_metadata_page *m_page = m_disk->lru_page;

        int i = 0;

        fprintf(stderr, "START LRU DUMP;\n");

        while (m_page != NULL) {
                fprintf(stderr, "LRU DUMP; [%d] page_id = %d, prev = %p, next = %p\n", i, m_page->page_id,
                        m_page->prev, m_page->next);

                m_page = m_page->next;
        }

        fprintf(stderr, "END LRU DUMP;\n");
}