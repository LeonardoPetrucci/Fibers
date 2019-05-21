#ifndef TYPES_H
#define TYPES_H

#ifdef USER
#include <sys/types.h>
#endif

#ifdef KERNEL
#include <linux/types.h>
#define NAME_LENGHT 256
#define FLS_SIZE 4096

typedef pid_t tgid_t;

typedef struct id_s
{
    pid_t pid;
    tgid_t tgid;
}id_t;

typedef struct fiber_info_s
{
    char name[NAME_LENGHT];
    pid_t parent_pid;
    int state; //boolean, 1 if running, 0 if not
    unsigned long successful_activations;
    unsigned long failed_activations;
    unsigned long last_running_time;
    unsigned long total_running_active;
    unsigned long entry_point;
    void * param;
}fiber_info_t;

typedef struct fiber_context_s
{
    struct pt_regs regs;
    struct fpu fpu;

}fiber_context_t;

typedef struct fls_s
{
    long long data[FLS_SIZE];
    DECLARE_BITMAP(bmp, FLS_SIZE);
}fls_t;

#endif

typedef pid_t fid_t;


typedef struct fib_args_s
{
    void * stack_base;
    size_t stack_size;
    unsigned long entry_point;
    void * params;
}fib_args_t;

typedef struct fls_args_s
{
    long idx;
    long long value;
}fls_args_t;

#endif