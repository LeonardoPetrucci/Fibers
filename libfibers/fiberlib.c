#include "include/fiberlib.h"

#define QUIET

int fd;

pid_t Convert_thread_to_fiber(void)
{
    long ret;
    fd = open(FIBERS_DEVICE, O_RDWR);
    if(fd < 0)
    {
        #ifdef VERBOSE
        printf("Critical error: cannot open fibers device.\n");
        #endif
        close(fd);
        exit(-ENOENT);
    }
    ret = ioctl(fd, IOCTL_CONVERT_THREAD_TO_FIBER, 0);
    #ifdef VERBOSE
    printf("Thread %d converted into fiber with id %d.\n", gettid(), ret);
    #endif
    return (pid_t) ret;
}

pid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void * params)
{
    long ret;
    fib_args_t fib_args = {
        .stack_size = stack_size,
        .entry_point = (unsigned long) entry_point,
        .params = params,
    };

    if(posix_memalign(&(fib_args.stack_base), 16, stack_size))
    {   
        #ifdef VERBOSE
        printf("Critical error: cannot initialize a separate stack\n");
        #endif
        exit(-ECANCELED);
    }

    bzero(fib_args.stack_base, stack_size);
    ret = ioctl(fd, IOCTL_CREATE_FIBER, (unsigned long) &fib_args);
    #ifdef VERBOSE
    printf("Created fiber with id %d\n", ret);
    #endif
    return (pid_t) ret;
}

int Switch_to_fiber(void * fib)
{
    long ret;

    ret = ioctl(fd, IOCTL_SWITCH_TO_FIBER, (unsigned long) &fib);
    if(ret < 0)
    {
        return -EAGAIN;
    }
    #ifdef VERBOSE
    printf("Thread %d switching to fiber %d...done.\n", gettid(), (int) fib);
    #endif
    return (int) ret;
}

long Fls_alloc(void)
{
    long ret;
    ret = ioctl(fd, IOCTL_FLS_ALLOC, 0);
    if(ret < 0)
    {
        return -EAGAIN;
    }
    #ifdef VERBOSE
    printf("Thread %d allocated fls at index %li.\n", gettid(), idx);
    #endif
    return ret;
}

long long Fls_get_value(long idx)
{
   long ret;
   long long read_value = 0;
    fls_args_t fls_args = {
        .idx = idx,
        .value = (unsigned long) &read_value,
    };
    
    ret = ioctl(fd, IOCTL_FLS_GET_VALUE, &fls_args);
    if(ret < 0)
    {
        return -EAGAIN;
    }
    #ifdef VERBOSE
    printf("Thread %d read value %lli at index %li.\n", gettid(), read_value, idx);
    #endif
    return read_value;
}

int Fls_free(long idx)
{
    int ret;
    fls_args_t fls_args = {
        .idx = idx,
    };
    ret = ioctl(fd, IOCTL_FLS_FREE, (unsigned long) &fls_args);
    if(ret < 0)
    {
        return -EAGAIN;
    }
    return ret;
}

long Fls_set_value(long idx, long long value)
{
    long ret;
    fls_args_t fls_args = {
        .idx = idx,
        .value = value,
    };
    ret = ioctl(fd, IOCTL_FLS_SET_VALUE, (unsigned long) &fls_args);
    if(ret < 0)
    {
        return -EAGAIN;
    }
    #ifdef VERBOSE
    printf("Thread %d set value %lli at index %li.\n", gettid(), value, idx);
    #endif
    return ret;
}