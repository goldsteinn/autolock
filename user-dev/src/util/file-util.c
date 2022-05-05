#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util/error-util.h"
#include "util/file-util.h"

int32_t
I_safe_open2(char const * restrict path,
             int32_t flags,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    int32_t ret = open(path, flags);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno,
                 "Unable to open \"%s\" with [flags=0x%03x]\n", path, flags);
    }
    return ret;
}
int32_t
I_safe_open3(char const * restrict path,
             int32_t flags,
             int32_t mode,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    int32_t ret = open(path, mode);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno,
                 "Unable to open \"%s\" with [flags=0x%03x][mode=0x%03x])\n",
                 path, flags, mode);
    }
    return ret;
}
int64_t
I_safe_read(int32_t fd,
            void * restrict buf,
            size_t count,
            char const * restrict fn,
            char const * restrict func,
            uint32_t ln) {
    int64_t ret = read(fd, buf, count);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}
int64_t
I_safe_write(int32_t fd,
             void const * restrict buf,
             size_t count,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    int64_t ret = write(fd, buf, count);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}

int32_t
I_safe_stat(char const * restrict path,
            struct stat * restrict buf,
            char const * restrict fn,
            char const * restrict func,
            uint32_t ln) {
    int32_t ret = stat(path, buf);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno, "Unable to stat \"%s\"\n", path);
    }
    return ret;
}

int32_t
I_safe_fstat(int32_t fd,
             struct stat * restrict buf,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    int32_t ret = fstat(fd, buf);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}

int32_t
I_safe_access(char const * restrict path,
              int32_t mode,
              char const * restrict fn,
              char const * restrict func,
              uint32_t ln) {
    int32_t ret = access(path, mode);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno,
                 "Unable to access \"%s\" with [mode=0x%03x]\n", path, mode);
    }
    return ret;
}

int32_t
I_safe_close(int32_t fd,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    int32_t ret = close(fd);
    if (UNLIKELY(ret < 0)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}

int32_t
I_check_open2(const char * path,
              int32_t      flags,
              const char * fn,
              const char * func,
              uint32_t     ln,
              const char * msg,
              ...) {
    int32_t ret = open(path, flags);
    if (UNLIKELY(ret < 0)) {
        va_list ap;
        va_start(ap, msg);
        I_va_errdie(fn, func, ln, errno, msg, ap);
        va_end(ap);
    }
    return ret;
}

int32_t
I_check_open3(const char * path,
              int32_t      flags,
              int32_t      mode,
              const char * fn,
              const char * func,
              uint32_t     ln,
              const char * msg,
              ...) {
    int32_t ret = open(path, flags, mode);
    if (UNLIKELY(ret < 0)) {
        va_list ap;
        va_start(ap, msg);
        I_va_errdie(fn, func, ln, errno, msg, ap);
        va_end(ap);
    }
    return ret;
}

int32_t
I_check_access(const char * path,
               int32_t      mode,
               const char * fn,
               const char * func,
               uint32_t     ln,
               const char * msg,
               ...) {
    int32_t ret = access(path, mode);
    if (UNLIKELY(ret < 0)) {
        va_list ap;
        va_start(ap, msg);
        I_va_errdie(fn, func, ln, errno, msg, ap);
        va_end(ap);
    }
    return ret;
}

FILE *
I_safe_fopen(char const * restrict path,
             char const * restrict mode,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    FILE * fp = fopen(path, mode);
    if (UNLIKELY(fp == NULL)) {
        I_errdie(fn, func, ln, errno,
                 "Unable to open \"%s\" with [mode=%-8s]\n", path, mode);
    }
    return fp;
}

FILE *
I_safe_fdopen(int32_t fd,
              char const * restrict mode,
              char const * restrict fn,

              char const * restrict func,
              uint32_t ln) {
    FILE * fp = fdopen(fd, mode);
    if (UNLIKELY(fp == NULL)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return fp;
}

size_t
I_safe_fread(void * restrict ptr,
             size_t size,
             size_t nmemb,
             FILE * restrict stream,
             char const * restrict fn,
             char const * restrict func,
             uint32_t ln) {
    size_t ret = fread(ptr, size, nmemb, stream);
    if (UNLIKELY(ret != nmemb)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}

size_t
I_safe_fwrite(const void * restrict ptr,
              size_t size,
              size_t nmemb,
              FILE * restrict stream,
              char const * restrict fn,
              char const * restrict func,
              uint32_t ln) {
    size_t ret = fwrite(ptr, size, nmemb, stream);
    if (UNLIKELY(ret != nmemb)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}

FILE *
I_check_fopen(const char * restrict path,
              const char * restrict mode,
              const char * fn,
              const char * func,
              uint32_t     ln,
              const char * msg,
              ...) {
    FILE * fp = fopen(path, mode);
    if (UNLIKELY(fp == NULL)) {
        va_list ap;
        va_start(ap, msg);
        I_va_errdie(fn, func, ln, errno, msg, ap);
        va_end(ap);
    }
    return fp;
}

int32_t
I_safe_fclose(FILE * restrict stream,
              char const * restrict fn,
              char const * restrict func,
              uint32_t ln) {
    int32_t ret = fclose(stream);
    if (UNLIKELY(ret != 0)) {
        I_errdie(fn, func, ln, errno, NULL);
    }
    return ret;
}
