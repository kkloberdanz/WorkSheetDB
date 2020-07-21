#ifndef KT_FILE_H
#define KT_FILE_H

#include <stdio.h>

struct kt_file {
    int fd;
    char *dat;
    size_t map_size;
    char *fname;
    int col;
    char *table;
    size_t row_begin;
    size_t row_end;
};

int kt_file_free(struct kt_file *file);

int kt_open(const char *fname);

struct kt_file *kt_mmap(const char *fname);

struct kt_file *kt_find_file(const char *table, int col, int row);

#endif /* KT_FILE_H */
