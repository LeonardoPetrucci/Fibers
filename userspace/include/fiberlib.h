#include <linux/types.h>

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

void *fib_convert(void);
void *fib_creat(size_t stack_size, void (*entry_point)(void *), void *param);
void fib_switch_to(void *fib);
long fib_fls_alloc(void);
long long fib_fls_get(long idx);
bool fib_fls_free(long idx);
void fib_fls_set(long idx, long long value);