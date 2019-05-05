#include <linux/fs.h>
#include <linux/ioctl.h>

#define IOCTL_NUMBER 'k'
#define IOCTL_CONVERT_THREAD_TO_FIBER _IO(IOCTL_NUMBER, 0)
#define IOCTL_CREATE_FIBER _IO(IOCTL_NUMBER, 1)
#define IOCTL_SWITCH_TO_FIBER _IO(IOCTL_NUMBER, 2)
#define IOCTL_FLS_ALLOC _IO(IOCTL_NUMBER, 3)
#define IOCTL_FLS_GET_VALUE _IO(IOCTL_NUMBER, 4)
#define IOCTL_FLS_FREE _IO(IOCTL_NUMBER, 5)
#define IOCTL_FLS_SET_VALUE _IO(IOCTL_NUMBER, 6)

int fibers_open(struct inode *, struct file *);
int fibers_release(struct inode *, struct file *);
long fibers_ioctl(struct file *, unsigned int, unsigned long);