#include <sys/ioctl.h>

#include "kernel_device.h"
#include "fiber.h"

void *fib_convert(void){
    int fibers_device = open(FIBERS_DEVICE, O_RDWR);
    if(fibers_device < 0)
    {
        return -1;
    }
    int ret = ioctl(DEVICE, FIBER_CONVERT, NULL);
    return ret;
}
void *fib_creat(size_t stack_size, void (*entry_point)(void *), void *param);
void fib_switch_to(void *fib);
long long fls_get(long idx);
bool fls_free(long idx);
long fls_alloc(void);
void fls_set(long idx, long long value);