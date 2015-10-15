/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <utils/time.h>

#include <sel4/sel4.h>
#include <sos.h>

#include "test_constants.h"

void timer_errors() {
    /* We just shouldn't sleep. */
    int64_t t1 = sos_sys_time_stamp();
    sos_sys_usleep(0);
    sos_sys_usleep(-1);
    sos_sys_usleep(-234324);
    sos_sys_usleep(INT_MIN);
    assert(t1 - sos_sys_time_stamp() < 3000000);
}

void file_errors() {
    char path[2 * MAX_PATH_LENGTH];
    for (int i = 0; i < 2 * MAX_PATH_LENGTH; i++) {
        path[i] = 'a';
    }

    path[2 * MAX_PATH_LENGTH - 1] = '\0';
    int fd = sos_sys_open(path, O_RDONLY);
    assert(fd == -1);

    path[MAX_PATH_LENGTH] = '\0';
    fd = sos_sys_open(path, O_RDONLY);
    assert(fd == -1);

    path[0] = '\0';
    fd = sos_sys_open(path, O_RDONLY);
    assert(fd == -1);

    fd = sos_sys_open(NULL, O_RDONLY);
    assert(fd == -1);

    fd = sos_sys_open((char *)1000, O_RDONLY);
    assert(fd == -1);

    fd = sos_sys_open("a_new_file.txt", O_RDWR);
    assert(fd != -1);
    assert(sos_sys_close(fd) == 0);

    fd = sos_sys_open("a_new_file.txt", FM_READ | FM_WRITE);
    assert(fd == -1);

    fd = sos_sys_open("a_new_file_2.txt", 890244);
    assert(fd == -1);

    assert(sos_sys_close(-1) == -1);
    assert(sos_sys_close(-243243244) == -1);
    assert(sos_sys_close(1000) == -1);
    assert(sos_sys_close(543543545) == -1);

    char buff[3 * PAGE_SIZE];
    assert(sos_sys_write(-1, buff, 1) == -1);
    assert(sos_sys_write(-34234324, buff, 1000) == -1);
    assert(sos_sys_write(1000, buff, 3 * PAGE_SIZE) == -1);
    assert(sos_sys_write(454355455, buff, 234) == -1);
    assert(sos_sys_read(-1, buff, 1) == -1);
    assert(sos_sys_read(-34234324, buff, 1000) == -1);
    assert(sos_sys_read(1000, buff, 3 * PAGE_SIZE) == -1);
    assert(sos_sys_read(454355455, buff, 234) == -1);

    fd = sos_sys_open("a_new_file.txt", O_WRONLY);
    assert(fd != -1);
    /* Not really an error but rather a corner-case. */
    assert(sos_sys_write(fd, NULL, 0) == 0);
    assert(sos_sys_write(fd, NULL, 3242) == -1);
    assert(sos_sys_write(fd, (char *)1000, 1) == -1);
    assert(sos_sys_write(fd, (char *)~0, 1000) == -1);
    assert(sos_sys_write(fd, buff, ~0) == -1);
    assert(sos_sys_read(fd, buff, 1) == -1);
    /* Not really an error but rather a corner case. */
    assert(sos_sys_write(fd, buff, 0) == 0);
    assert(sos_sys_write(fd, buff, 1) == 1);
    assert(sos_sys_close(fd) == 0);
    assert(sos_sys_write(fd, buff, 1) == -1);

    fd = sos_sys_open("a_new_file.txt", O_RDONLY);
    assert(fd != -1);
    assert(sos_sys_read(fd, NULL, 0) == 0);
    assert(sos_sys_read(fd, NULL, 3242) == -1);
    assert(sos_sys_read(fd, (char *)1000, 1) == -1);
    assert(sos_sys_read(fd, (char *)~0, 1000) == -1);
    assert(sos_sys_read(fd, buff, ~0) == -1);
    assert(sos_sys_write(fd, buff, 1) == -1);
    /* Not really an error but rather a corner case. */
    assert(sos_sys_read(fd, buff, 0) == 0);
    assert(sos_sys_read(fd, "a_new_file.txt", 1) == -1);
    assert(sos_sys_close(fd) == 0);
    assert(sos_sys_read(fd, buff, 1) == -1);

    fd = sos_sys_open("console", O_RDONLY);
    assert(sos_sys_open("console", O_RDONLY) == -1);
    assert(sos_sys_close(fd) == 0);

    assert(sos_sys_open("swap", O_RDONLY) == -1);

    char name_buff[MAX_PATH_LENGTH];
    assert(sos_getdirent(-1, name_buff, MAX_PATH_LENGTH) == -1);
    assert(sos_getdirent(-34214, name_buff, MAX_PATH_LENGTH) == -1);
    assert(sos_getdirent(342423, name_buff, MAX_PATH_LENGTH) == -1);
    assert(sos_getdirent(0, name_buff, ~0) == -1);
    assert(sos_getdirent(0, "a_new_file.txt", 100) == -1);
    assert(sos_getdirent(0, NULL, 100) == -1);
    assert(sos_getdirent(0, (void *)~0, 1000) == -1);
    assert(sos_getdirent(0, name_buff, 0) == -1);

    sos_stat_t stat;
    assert(sos_stat("non_existant_file.wmv", &stat) == -1);
    assert(sos_stat(NULL, &stat) == -1);
    assert(sos_stat((void *)1000, &stat) == -1);
    assert(sos_stat("a_new_file.txt", NULL) == -1);
    assert(sos_stat("a_new_file.txt", (void *)~0) == -1);
    assert(sos_stat("a_new_file.txt", (void *)1000) == -1);
    assert(sos_stat("a_new_file.txt", (void *)"a_new_file.txt") == -1);

    int fd_buff[10000];
    printf("Opening files until failure. This might take a bit...\n");
    int i;
    for (i = 0; i < 10000; i++) {
        fd_buff[i] = sos_sys_open("a_new_file.txt", O_RDONLY);
        if (fd_buff[i] == -1) {
            break;
        }
    }

    assert(i < 10000);

    printf("Closing all open files. This might take a bit...\n");
    for (i = 0; i < 10000; i++) {
        if (fd_buff[i] == -1) {
            break;
        }

        assert(sos_sys_close(fd_buff[i]) == 0);
    }

    fd = sos_sys_open("a_new_file.txt", O_RDONLY);
    assert(fd != -1);
    assert(sos_sys_close(fd) == 0);
}

int main() {
    printf("Running timer error tests.\n");
    timer_errors();
    printf("Timer error tests passed.\n");

    printf("Running file error tests.\n");
    file_errors();
    printf("File error tests passed.\n");
}

