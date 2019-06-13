#include "include/fiberlib.h"

int fd;

pid_t Convert_thread_to_fiber(void)
{
    int ret;
    fd = open(FIBERS_DEVICE, O_RDWR);
    if(fd < 0)
    {
        close(fd);
        return -1;
    }
    ret = ioctl(fd, IOCTL_CONVERT_THREAD_TO_FIBER, 0);
    return (pid_t) ret;
}

pid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void * params)
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
    ret = ioctl(fd, IOCTL_CREATE_FIBER, (unsigned long) &fib_args);
    return (pid_t) ret;
}

int Switch_to_fiber(void * fib)
{
    int ret;

    ret = ioctl(fd, IOCTL_SWITCH_TO_FIBER, (unsigned long) &fib);
    if(ret < 0)
    {
        return -1;
    }
    return ret;
}

long Fls_alloc(void)
{
    long ret;
    ret = ioctl(fd, IOCTL_FLS_ALLOC, 0);
    return ret;
}

long long Fls_get_value(long idx)
{
   long long ret;
   long long read_value = 0;
    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = (unsigned long) &read_value;
    ret = ioctl(fd, IOCTL_FLS_GET_VALUE, &fls_args);
    return read_value;
}

int Fls_free(long idx)
{
    int ret;
    fls_args_t fls_args;
    fls_args.idx = idx;
    ret = ioctl(fd, IOCTL_FLS_FREE, (unsigned long) &fls_args);
    return ret;
}

long Fls_set_value(long idx, long long value)
{
    long ret;
    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = value;
    ret = ioctl(fd, IOCTL_FLS_SET_VALUE, (unsigned long) &fls_args);
    return ret;
}