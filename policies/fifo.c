#include "fifo.h"

#include "../models/disk_model.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct fifo_metadata_page {
        size_t page_id;
        struct fifo_metadata_page *prev;
        struct fifo_metadata_page *next;
};

struct fifo_metadata_disk {
        struct fifo_metadata_page *fi_page; // First in
        struct fifo_metadata_page *li_page; // Last in
};

const struct disk_model_replacement_hooks fifo_hooks = {
        .hook_replacement_metadata_init_page = fifo_hook_replacement_metadata_init_page,
        .hook_replacement_metadata_free_page = fifo_hook_replacement_metadata_free_page,
        .hook_replacement_metadata_init_disk = fifo_hook_replacement_metadata_init_disk,
        .hook_replacement_metadata_free_disk = fifo_hook_replacement_metadata_free_disk,
        .hook_page_replace = fifo_hook_page_replace,
        .hook_page_fault = fifo_hook_page_fault,
};

void *fifo_hook_replacement_metadata_init_page()
{
        struct fifo_metadata_page *m = calloc(1, sizeof(struct fifo_metadata_page));
        assert(m != NULL);

        return m;
}

void fifo_hook_replacement_metadata_free_page(void *metadata_page)
{
        free(metadata_page);
}

void *fifo_hook_replacement_metadata_init_disk()
{
        struct fifo_metadata_disk *m = calloc(1, sizeof(struct fifo_metadata_disk));
        assert(m != NULL);

        return m;
}

void fifo_hook_replacement_metadata_free_disk(void *metadata_disk)
{
        free(metadata_disk);
}

void fifo_hook_page_fault(size_t page_id, void *metadata_disk, void *metadata_page)
{
        struct fifo_metadata_disk *m_disk = metadata_disk;
        struct fifo_metadata_page *m_page = metadata_page;

        assert(m_disk != NULL);
        assert(m_page != NULL);

        m_page->page_id = page_id;

        /*
         * Delete from doubly-linked list (based on LRU doubly-linked list)
         */

        if (m_page->prev != NULL) m_page->prev->next = m_page->next;
        if (m_page->next != NULL) m_page->next->prev = m_page->prev;

        if (m_page == m_disk->fi_page) m_disk->fi_page = m_page->prev;
        if (m_page == m_disk->li_page) m_disk->li_page = m_page->next;

        m_page->prev = NULL;
        m_page->next = NULL;

        /*
         * Insert to doubly-linked list (based on LRU doubly-linked list)
         */

        if (m_disk->fi_page == NULL) m_disk->fi_page = m_page;
        if (m_disk->li_page != NULL) m_disk->li_page->next = m_page;
        m_page->prev = m_disk->li_page;
        m_disk->li_page = m_page;
}

size_t fifo_hook_page_replace(void *metadata_disk)
{
        struct fifo_metadata_disk *m_disk = metadata_disk;
        assert(m_disk != NULL);

        assert(m_disk->fi_page != NULL);
        size_t replace_page_id = m_disk->fi_page->page_id;
        struct fifo_metadata_page *tmp = m_disk->fi_page;

        if (m_disk->li_page == m_disk->fi_page) m_disk->li_page = NULL;
        if (m_disk->fi_page->next != NULL) m_disk->fi_page->next->prev = NULL;
        m_disk->fi_page = m_disk->fi_page->next;

        // Zero out unused metadata
        memset(tmp, 0, sizeof(struct fifo_metadata_page));

        return replace_page_id;
}
