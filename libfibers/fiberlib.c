#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "include/fiberlib.h"

#define STACK_SIZE 8192

int fd;

fid_t Convert_thread_to_fiber(void)
{
    int ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        close(fd);
        return -1;
    }
    ret = ioctl(fd, FIB_CONVERT, 0);
    return (fid_t) ret;
}

fid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void * params)
{
    int ret;
    fib_args_t fib_args;
    fib_args.stack_size = stack_size;
    fib_args.entry_point = (unsigned long) entry_point;
    fib_args.params = params;
    if(posix_memalign(&(fib_args.stack_base), 16, stack_size))
    {
        return -1;
    }

    bzero(fib_args.stack_base, stack_size);
    ret = ioctl(fd, FIB_CREATE, (unsigned long) &fib_args);
    return (fid_t) ret;
}

fid_t Switch_to_fiber(void * fib)
{
    int ret;
    ret = ioctl(fd, FIB_SWITCH, (unsigned long) &fib);
    return (fid_t) ret;
}

long Fls_alloc(void)
{
    long ret;
    ret = ioctl(fd, FLS_ALLOC, 0);
    return ret;
}

long long Fls_get_value(long idx)
{
   long long ret;
   long long read_value = 0;
    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = (unsigned long) &read_value;
    ret = ioctl(fd, FLS_GET, &fls_args);
    return read_value;
}

int Fls_free(long idx)
{
    int ret;
    fls_args_t fls_args;
    fls_args.idx = idx;
    ret = ioctl(fd, FLS_FREE, (unsigned long) &fls_args);
    return ret;
}

long Fls_set_value(long idx, long long value)
{
    long ret;
    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = value;
    ret = ioctl(fd, FLS_SET, (unsigned long) &fls_args);
    return ret;
}