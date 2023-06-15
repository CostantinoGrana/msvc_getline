#include "getline.h"

#include <stdlib.h>
#include <errno.h>

// MSVC specific implementation
static void fseterr(FILE *fp)
{
    struct file { // Undocumented implementation detail
        unsigned char *_ptr;
        unsigned char *_base;
        int _cnt;
        int _flag;
        int _file;
        int _charbuf;
        int _bufsiz;
    };
    #define _IOERR 0x10

    ((struct file *)fp)->_flag |= _IOERR;
}

ssize_t getdelim(char **restrict lineptr, size_t *restrict n, int delim, FILE *restrict stream)
{
    if (lineptr == NULL || n == NULL || stream == NULL || (*lineptr == NULL && *n != 0)) {
        errno = EINVAL;
        return -1;
    }
    if (feof(stream) || ferror(stream)) {
        return -1;
    }

    if (*lineptr == NULL) {
        *n = 256;
        *lineptr = malloc(*n);
        if (*lineptr == NULL) {
            fseterr(stream);
            errno = ENOMEM;
            return -1;
        }
    }
    ssize_t nread = 0;
    int c = EOF;
    while (c != delim) {
        c = fgetc(stream);
        if (c == EOF) {
            break;
        }
        if (nread >= (ssize_t)(*n - 1)) {
            size_t newn = *n * 2;
            char *newptr = realloc(*lineptr, newn);
            if (newptr == NULL) {
                fseterr(stream);
                errno = ENOMEM;
                return -1;
            }
            *lineptr = newptr;
            *n = newn;
        }
        (*lineptr)[nread++] = c;
    }
    if (c == EOF && nread == 0) {
        return -1;
    }
    (*lineptr)[nread] = 0;
    return nread;
}

ssize_t getline(char **restrict lineptr, size_t *restrict n, FILE *restrict stream)
{
    return getdelim(lineptr, n, '\n', stream);
}

