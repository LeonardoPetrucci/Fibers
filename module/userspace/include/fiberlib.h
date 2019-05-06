#include <linux/types.h>

typedef struct{
    size_t stack_size;
    void (*entry_point) (void *);
    void * params;
}fib_args_t;

typedef struct{
    long idx;
    long long value;
}fls_args_t;

void *fib_convert(void);
void *fib_creat(size_t stack_size, void (*entry_point)(void *), void *param);
void fib_switch_to(void *fib);
long long fls_get(long idx);
bool fls_free(long idx);
long fls_alloc(void);
void fls_set(long idx, long long value);