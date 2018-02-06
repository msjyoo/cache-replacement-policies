#include "../model/disk_model.h"

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

static void test_disk_model_put_get_1(void **state)
{
        int ret = -1;

        struct disk_model *model = disk_model_init(25);
        size_t page_id = 0;

        ret = disk_model_put(model, "This is test string 0.", strlen("This is test string 0.") + 1, NULL);
        assert_int_equal(ret, 0);
        ret = disk_model_put(model, "This is test string 1.", strlen("This is test string 1.") + 1, &page_id);
        assert_int_equal(ret, 0);

        char buf[DISK_MODEL_PAGE_SIZE] = {0};

        ret = disk_model_get(model, page_id, buf, DISK_MODEL_PAGE_SIZE);
        assert_int_equal(ret, 0);

        assert_string_equal(buf, "This is test string 1.");

        disk_model_free(model);
}

int main(void)
{
        const struct CMUnitTest tests[] = {
                cmocka_unit_test(test_disk_model_put_get_1),
        };

        return cmocka_run_group_tests(tests, NULL, NULL);
}