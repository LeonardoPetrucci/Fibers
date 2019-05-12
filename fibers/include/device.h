#ifdef KERNEL

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/export.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include "types.h"

#define DEVICE_NAME "fibers"
#define CLASS_NAME "aosv"

extern int fibers_open(struct inode *, struct file *);
extern int fibers_release(struct inode *, struct file *);
extern long fibers_ioctl(struct file *, unsigned int, unsigned long);

int fibers_register_device(void);
int fibers_unregister_device(void);

#endif

#ifdef USER

#include <sys/ioctl.h>

#endif


//TODO: CHANGE _IO WITH OPPORTUNE TYPE!!!!!!
#define IOCTL_NUMBER 'k'
#define IOCTL_CONVERT_THREAD_TO_FIBER _IO(IOCTL_NUMBER, 0)
#define IOCTL_CREATE_FIBER _IO(IOCTL_NUMBER, 1)
#define IOCTL_SWITCH_TO_FIBER _IO(IOCTL_NUMBER, 2)
#define IOCTL_FLS_ALLOC _IO(IOCTL_NUMBER, 3)
#define IOCTL_FLS_GET_VALUE _IO(IOCTL_NUMBER, 4)
#define IOCTL_FLS_FREE _IO(IOCTL_NUMBER, 5)
#define IOCTL_FLS_SET_VALUE _IO(IOCTL_NUMBER, 6)

