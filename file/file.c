#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "file.h"
#include "../globals.h"

int kt_file_free(struct kt_file *file) {
    if (file) {
        close(file->fd);
        if (msync(file->dat, file->map_size, MS_SYNC) == -1) {
            perror("failed to write to disk");
            return -1;
        }

        if (munmap(file->dat, file->map_size)) {
            perror("failed to unmap memory");
            return -2;
        }

        free(file);
    }
    return 0;
}

int kt_open(const char *fname) {
    int fd;
    if ((fd = open(fname, O_RDWR|O_CREAT, 0600)) == -1) {
        char buf[200];
        snprintf(buf, 200, "kt_open failed to open file: '%s'", fname);
        perror(buf);
    }
    return fd;
}

struct kt_file *kt_mmap(const char *fname) {
    char *dat;
    struct stat statbuf;
    size_t map_size;
    int fd = kt_open(fname);
    struct kt_file *file;

    if (fd == -1) {
        goto return_fail;
    }

    fstat(fd, &statbuf);
    map_size = statbuf.st_size > KT_MAP_SIZE ? statbuf.st_size : KT_MAP_SIZE;

    if (lseek(fd, map_size - 1, SEEK_SET) == -1) {
        perror("error calling lseek() to 'stretch' the file");
        goto close_fd;
    }

    if (write(fd, "", 1) == -1) {
        perror("error writing last byte of the file");
        goto close_fd;
    }

    dat = mmap(
        NULL,
        map_size,
        PROT_READ|PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    if (dat == MAP_FAILED) {
        perror("failed to map file");
        goto close_fd;
    }

    file = malloc(sizeof(*file));
    file->fd = fd;
    file->dat = dat;
    file->map_size = map_size;
    return file;

close_fd:
    close(fd);

return_fail:
    return NULL;
}

#ifdef KT_TEST_MMAP
int main(void) {
    int i;
    struct kt_file *file;
    const char *str = "this is some data";
    size_t len = strlen(str);
    int status = 0;

    file = kt_mmap("tmp.dat");
    if (!file) {
        return -1;
    }

    for (i = 0; i < 10; i++) {
        printf("%c", (file->dat + 40)[i]);
    }
    puts("\nEND\n");

    memcpy(file->dat + 10, str, len);

    if (kt_file_free(file)) {
        const char *msg = "*** Very bad things have happened, data lost ***";
        fprintf(stderr, "%s\n", msg);
        status = -1;
    }
    return status;
}
#endif
