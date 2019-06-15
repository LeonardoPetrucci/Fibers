#include <unistd.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <strings.h>
#include <sys/syscall.h>
#include <errno.h>

#define gettid() syscall(SYS_gettid)
#define STACK_SIZE 8192
#define FIBERS_DEVICE "/dev/fibers"

#include "../../fibers/include/device.h"

/*******************************************************************************/
/*                             From device.h                                   */
/*******************************************************************************/

/* 
#define IOCTL_NUMBER
#define IOCTL_CONVERT_THREAD_TO_FIBER
#define IOCTL_CREATE_FIBER
#define IOCTL_SWITCH_TO_FIBER
#define IOCTL_FLS_ALLOC
#define IOCTL_FLS_GET_VALUE
#define IOCTL_FLS_FREE
#define IOCTL_FLS_SET_VALUE
*/

#include "../../fibers/include/fiber.h"

/*******************************************************************************/
/*                              From fiber.h                                   */
/*******************************************************************************/

/*
typedef struct fib_args_s {
    void * stack_base;
    size_t stack_size;
    unsigned long entry_point;
    void * params;
}fib_args_t;

typedef struct fls_args_s {
    long idx;
    long long value;
}fls_args_t;
*/

pid_t Convert_thread_to_fiber(void);
pid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void *param);
int Switch_to_fiber(void *fib);
long Fls_alloc(void);
long long Fls_get_value(long idx);
int Fls_free(long idx);
long Fls_set_value(long idx, long long value);