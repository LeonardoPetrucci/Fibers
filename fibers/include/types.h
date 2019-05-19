#ifndef TYPES_H
#define TYPES_H

#ifdef USER
#include <sys/types.h>
#endif

#ifdef KERNEL
#include <linux/types.h>

typedef pid_t tgid_t;

typedef struct id_s
{
    pid_t pid;
    tgid_t tgid;
}id_t;

#endif


typedef pid_t fid_t;

//use typedef for coherence
typedef struct fib_args_s
{
    size_t stack_size;
    unsigned long entry_point; //the same tecnique adopted by ioctl for parameters passing
    void * params;
}fib_args_t;

typedef struct fls_args_s
{
    long idx;
    long long value;
}fls_args_t;

#endif