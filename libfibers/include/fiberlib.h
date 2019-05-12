#include <unistd.h>
#include <stdbool.h>
#include "kernel_device.h"

fid_t Convert_thread_to_fiber(void);
fid_t Create_fiber(size_t stack_size, void (*entry_point)(void *), void *param);
void Switch_to_fiber(void *fib);
long Fls_alloc(void);
long long Fls_get_value(long idx);
bool Fls_free(long idx);
void Fls_set_value(long idx, long long value);