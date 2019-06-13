#ifndef FIB_DEVICE_H
#define FIB_DEVICE_H

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

#include "fiber.h"

#define DEVICE_NAME "fibers"
#define CLASS_NAME "aosv"

extern int fibers_open(struct inode *, struct file *);
extern int fibers_release(struct inode *, struct file *);
extern long fibers_ioctl(struct file *, unsigned int, unsigned long);

extern int cleanup_all(void);
extern int cleanup_process(pid_t tgid);

//maybe extern?
int fibers_register_device(void);
int fibers_unregister_device(void);

#endif /* KERNEL */

//TODO: CHANGE _IO WITH OPPORTUNE TYPE!!!!!!
#define IOCTL_NUMBER 'k'

#define IOCTL_CONVERT_THREAD_TO_FIBER _IO(IOCTL_NUMBER, 0)
#define IOCTL_CREATE_FIBER _IOW(IOCTL_NUMBER, 1, fib_args_t)
#define IOCTL_SWITCH_TO_FIBER _IOW(IOCTL_NUMBER, 2, pid_t)

#define IOCTL_FLS_ALLOC _IO(IOCTL_NUMBER, 3)
#define IOCTL_FLS_GET_VALUE _IOR(IOCTL_NUMBER, 4, long)
#define IOCTL_FLS_FREE _IOW(IOCTL_NUMBER, 5, long)
#define IOCTL_FLS_SET_VALUE _IOW(IOCTL_NUMBER, 6, fls_args_t)

#endif /* FIB_DEVICE_H */