#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "include/fiberlib.h"

int fib_ioctl(int ioctl_command, unsigned long ioctl_arguments)
{
    int ret;
    int fibers_device = open(FIBERS_DEVICE, O_RDWR);
    if(fibers_device < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fibers_device);

        return -1;
    }

    ret = ioctl(fibers_device, ioctl_command, ioctl_arguments);
    close(fibers_device);

    return ret;
}

fid_t Convert_thread_to_fiber(void)
{
    printf("Called ConvertThreadToFiber.\n");
    int ret;
    ret = fib_ioctl(FIB_CONVERT, 0);

    return ret;
}

fid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void * params)
{
    printf("Called CreateFiber.\n");
    int ret;
    struct fib_args args;

    args.stack_size = stack_size;
    args.entry_point = (unsigned long) entry_point;
    args.params = params;

    ret = fib_ioctl(FIB_CREATE, (unsigned long) &args);

    return ret;
}

void Switch_to_fiber(void *fib)
{
    printf("Called SwitchToFiber.\n");
    int ret;
    ret = fib_ioctl(FIB_SWITCH, (unsigned long) &fib);

}

long long Fls_get_value(long idx)
{
    printf("Called FlsGetValue.\n");
    int ret;
    struct fls_args args;

    args.idx = idx;

    ret = fib_ioctl(FLS_GET, (unsigned long) &args);
    return ret;
}

bool Fls_free(long idx)
{
    printf("Called FlsFree.\n");
    int ret;
    
    struct fls_args args;

    args.idx = idx;

    ret = fib_ioctl(FLS_FREE, (unsigned long) &args);

    return ret;
}

long Fls_alloc(void)
{
    printf("Called FlsAlloc.\n");
    int ret;
    ret = fib_ioctl(FLS_ALLOC, 0);

    return ret;
}

void Fls_set_value(long idx, long long value)
{
    printf("Called FlsSetValue.\n");
    int ret;

    struct fls_args args;

    args.idx = idx;
    args.value = value;

    ret = fib_ioctl(FLS_SET, (unsigned long) &args);
}