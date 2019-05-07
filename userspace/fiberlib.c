#include <linux/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

#include "include/kernel_device.h"
#include "include/fiberlib.h"

int fib_ioctl(int ioctl_command, unsigned long ioctl_arguments)
{
    int ret;
    int fibers_device = open(FIBERS_DEVICE, O_RDONLY); //should I use the variant with mode parameter ?
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

void *fib_convert(void)
{
    int ret;
    ret = fib_ioctl(FIBER_CONVERT, 0);

    return ret;
}

void *fib_creat(size_t stack_size, void (*entry_point)(void *), void *params)
{
    int ret;
    fib_args_t args;

    args.stack_size = stack_size;
    args.entry_point = (unsigned long) entry_point;
    args.params = params;

    ret = fib_ioctl(IOCTL_CREATE_FIBER, (unsigned long) &args);

    return ret;
}

void fib_switch_to(void *fib)
{
    int ret;
    ret = fib_ioctl(IOCTL_SWITCH_TO_FIBER, (unsigned long) &fib);

    return;
}

long long fib_fls_get(long idx)
{
    int ret;
    fls_args_t args;

    args.idx = idx;

    ret = fib_ioctl(IOCTL_FLS_GET_VALUE, (unsigned long) &args);
    return ret;
}

bool fib_fls_free(long idx)
{
    int ret;
    
    fls_args_t args;

    args.idx = idx;

    ret = fib_ioctl(IOCTL_FLS_FREE, (unsigned long) &args);

    return ret;
}

long fib_fls_alloc(void)
{
    int ret;
    
    fls_args_t args;

    ret = fib_ioctl(IOCTL_FLS_ALLOC, 0);

    return ret;
}

void fib_fls_set(long idx, long long value)
{
    int ret;

    fls_args_t args;

    args.idx = idx;
    args.value = value;

    ret = fib_ioctl(IOCTL_FLS_SET_VALUE, (unsigned long) &args);

    return ret;
}