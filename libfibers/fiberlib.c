#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>

#include "include/fiberlib.h"

//#define STACK_SIZE 8192 //Value given by the tests
/*
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
/*
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
    fib_args_t args;

    args.stack_size = stack_size;
    args.entry_point = (unsigned long) entry_point;
    args.params = params;
    
    if (posix_memalign(&args.stack_base, 16, STACK_SIZE)){
        return -1;
    }
    bzero(args.stack_base, STACK_SIZE);

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
    
    long long ret = 0;
    fls_args_t args;

    args.idx = idx;
    args.value = (unsigned long) &ret;

    ret = fib_ioctl(FLS_GET, (unsigned long) &args);
    return ret;
    
   return 0;
}

bool Fls_free(long idx)
{
    
    printf("Called FlsFree.\n");
    
    int ret;
    
    fls_args_t args;

    args.idx = idx;

    ret = fib_ioctl(FLS_FREE, (unsigned long) &args);

    return ret;
    
   return 0;
}

long Fls_alloc(void)
{
    
    printf("Called FlsAlloc.\n");
    
    int ret;
    ret = fib_ioctl(FLS_ALLOC, 0);

    return ret;
    
   return 0;
}

void Fls_set_value(long idx, long long value)
{
    
    printf("Called FlsSetValue.\n");
    
    int ret;

    fls_args_t args;

    args.idx = idx;
    args.value = value;

    ret = fib_ioctl(FLS_SET, (unsigned long) &args);
    
   return 0;
}
*/

int fd;

fid_t Convert_thread_to_fiber(void)
{
    printf("Called Convert_thread_to_fiber.\n");
    int ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    ret = ioctl(fd, FIB_CONVERT, 0);
    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return (fid_t) ret;
}

fid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void * params)
{
    printf("Called Create_fiber.\n");
    int ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    fib_args_t fib_args;
    fib_args.stack_size = stack_size;
    fib_args.entry_point = (unsigned long) entry_point;
    fib_args.params = params;

    if(posix_memalign(&(fib_args.stack_base), 16, stack_size))
    {
        printf("[ERROR] Cannot obtain a memory-aligned stack base.\n");
    }
    bzero(fib_args.stack_base, stack_size);

    ret = ioctl(fd, FIB_CREATE, (unsigned long) &fib_args);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return (fid_t) ret;
}

fid_t Switch_to_fiber(void * fib)
{
    printf("Called Switch_to_fiber.\n");
    int ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    ret = ioctl(fd, FIB_SWITCH, (unsigned long) &fib);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return (fid_t) ret;
}

long Fls_alloc(void)
{
    printf("Called Fls_alloc.\n");
    long ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    ret = ioctl(fd, FLS_ALLOC, 0);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return ret;
}

long long Fls_get_value(long idx)
{
    printf("Called Fls_get_value.\n");
    long ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = 0;

    ret = ioctl(fd, FLS_GET, (unsigned long) &fls_args);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
        return ret;
    }

    close(fd);
    printf("Finish.\n");
    return fls_args.value;
}

int Fls_free(long idx)
{
    printf("Called Fls_free.\n");
    int ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    fls_args_t fls_args;
    fls_args.idx = idx;

    ret = ioctl(fd, FLS_FREE, (unsigned long) &fls_args);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return ret;
}

long Fls_set_value(long idx, long long value)
{
    printf("Called Fls_set_value.\n");
    long ret;
    fd = open(FIBERS_DEVICE, O_RDONLY);
    if(fd < 0)
    {
        printf("[ERROR] Cannot open device file %s. Exiting...\n", FIBERS_DEVICE);
        close(fd);
        return -1;
    }

    fls_args_t fls_args;
    fls_args.idx = idx;
    fls_args.value = value;

    ret = ioctl(fd, FLS_SET, (unsigned long) &fls_args);

    if(ret == -1)
    {
        printf("[ERROR] Something wrong has occurred. See dmesg for more details.\n");
    }

    close(fd);
    printf("Finish.\n");
    return ret;
}