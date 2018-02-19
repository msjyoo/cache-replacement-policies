#include "../models/disk_model.h"

#include "../policies/dummy.h"
#include "../policies/lru.h"
#include "../policies/fifo.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

// Must come afterwards
#include <cmocka.h>

static void test_disk_model_put_get_1(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(25, 25, dummy_hooks);
        size_t page_id = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, NULL);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        ret = disk_model_get(model, page_id, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        // Repeat access should be paged
        ret = disk_model_get(model, page_id, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);

        assert_string_equal(buf, "This is test string 1.");

        disk_model_free(model);
}

static void test_policy_lru_simple_1(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(25, 2, lru_hooks);
        size_t page_id_0, page_id_1, page_id_2 = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, &page_id_0);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id_1);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 2.", strlen("This is test string 2.") + 1, &page_id_2);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        // Simple paging test
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);

        // Establish LRU access recency
        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_2, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        // Test page faults
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        disk_model_free(model);
}

static void test_policy_lru_simple_2(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(25, 2, lru_hooks);
        size_t page_id_0, page_id_1, page_id_2 = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, &page_id_0);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id_1);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 2.", strlen("This is test string 2.") + 1, &page_id_2);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        // Establish LRU access recency
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_2, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        // Test page faults
        ret = disk_model_get(model, page_id_2, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        disk_model_free(model);
}

/*
 * 1 2 3 2 2 1
 * LRU:
 * 1 X
 * 1 2
 * 2 3
 * 3 2 <--
 * 3 2 <--
 * 2 1 <--
 *
 * FIFO:
 * 1 X
 * 1 2
 * 2 3
 * 2 3 <--
 * 2 3 <--
 * 3 1 <--
 */
static void test_policy_lru_example_1(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(3, 2, lru_hooks);
        size_t page_id_0, page_id_1, page_id_2 = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, &page_id_0);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id_1);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 2.", strlen("This is test string 2.") + 1, &page_id_2);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        ret = disk_model_get(model, page_id_2, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_1), DM_P_PAGED);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_PAGED);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_PAGED);

        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        assert_int_equal(disk_model_page_status(model, page_id_1), DM_P_PAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_UNPAGED);

        disk_model_free(model);
}

static void test_policy_fifo_example_1(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(3, 2, fifo_hooks);
        size_t page_id_0, page_id_1, page_id_2 = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, &page_id_0);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id_1);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 2.", strlen("This is test string 2.") + 1, &page_id_2);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);

        ret = disk_model_get(model, page_id_2, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_1), DM_P_PAGED);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_PAGED);

        ret = disk_model_get(model, page_id_1, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);
        assert_int_equal(disk_model_page_status(model, page_id_0), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_PAGED);

        ret = disk_model_get(model, page_id_0, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 1);
        // Different from LRU
        assert_int_equal(disk_model_page_status(model, page_id_1), DM_P_UNPAGED);
        assert_int_equal(disk_model_page_status(model, page_id_2), DM_P_PAGED);

        disk_model_free(model);
}

int main(void)
{
        const struct CMUnitTest tests[] = {
                cmocka_unit_test(test_disk_model_put_get_1),
                cmocka_unit_test(test_policy_lru_simple_1),
                cmocka_unit_test(test_policy_lru_simple_2),
                cmocka_unit_test(test_policy_lru_example_1),
        };

        return cmocka_run_group_tests(tests, NULL, NULL);
}