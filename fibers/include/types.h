#ifndef TYPES_H
#define TYPES_H

#ifdef KERNEL
#include <linux/types.h>

typedef pid_t tgid_t;

typedef struct id_s
{
    pid_t pid;
    tgid_t tgid;
}id_t;

#endif

#ifdef USER
#include <sys/types.h>
#endif

typedef pid_t fid_t;

//use typedef for coherence
struct fib_args
{
    size_t stack_size;
    unsigned long entry_point; //the same tecnique adopted by ioctl for parameters passing
    void * params;
};

struct fls_args
{
    long idx;
    long long value;
};

#endif