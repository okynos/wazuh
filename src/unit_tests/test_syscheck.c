/*
 * Copyright (C) 2015-2019, Wazuh Inc.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <string.h>

#include "../syscheckd/syscheck.h"


/* redefinitons/wrapping */

void __wrap__mwarn(const char * file, int line, const char * func, const char *msg, ...)
{
    char formatted_msg[OS_MAXSTR];
    va_list args;

    va_start(args, msg);
    vsnprintf(formatted_msg, OS_MAXSTR, msg, args);
    va_end(args);

    check_expected(formatted_msg);
}

void __wrap__merror_exit(const char * file, int line, const char * func, const char *msg, ...)
{
    char formatted_msg[OS_MAXSTR];
    va_list args;

    va_start(args, msg);
    vsnprintf(formatted_msg, OS_MAXSTR, msg, args);
    va_end(args);

    check_expected(formatted_msg);
}

fdb_t *__wrap_fim_db_init(int memory) {
    check_expected(memory);
    return mock_type(fdb_t*);
}

int __wrap_getDefine_Int() {
    return mock();
}

/* setup/teardowns */
static int setup_group(void **state) {
    fdb_t *fdb = calloc(1, sizeof(fdb_t));

    if(fdb == NULL)
        return -1;

    *state = fdb;

    return 0;
}

static int teardown_group(void **state) {
    fdb_t *fdb = *state;

    free(fdb);

    return 0;
}

/* tests */

void test_fim_initialize(void **state)
{
    fdb_t *fdb = *state;

    expect_value(__wrap_fim_db_init, memory, 0);
    will_return(__wrap_fim_db_init, fdb);

    fim_initialize();

    assert_ptr_equal(syscheck.database, fdb);
}

void test_fim_initialize_error(void **state)
{
    expect_value(__wrap_fim_db_init, memory, 0);
    will_return(__wrap_fim_db_init, NULL);

    expect_string(__wrap__merror_exit, formatted_msg, "(6698): Creating Data Structure: sqlite3 db. Exiting.");

    fim_initialize();

    assert_null(syscheck.database);
}

void test_read_internal(void **state)
{
    (void) state;

    will_return_always(__wrap_getDefine_Int, 1);

    read_internal(0);
}

void test_read_internal_debug(void **state)
{
    (void) state;

    will_return_always(__wrap_getDefine_Int, 1);

    read_internal(1);
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_fim_initialize),
        cmocka_unit_test(test_fim_initialize_error),
        cmocka_unit_test(test_read_internal),
        cmocka_unit_test(test_read_internal_debug),
    };

    return cmocka_run_group_tests(tests, setup_group, teardown_group);
}
