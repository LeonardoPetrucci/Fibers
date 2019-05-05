#include <linux/device.h>
#include <linux/fs.h>

#define DEVICE_NAME "fibers"
#define CLASS_NAME "aosv"

int fibers_device_register(void);
int fibers_device_unregister(void);