#define USER

#include "../../fibers/include/device.h"
#include "../../fibers/include/types.h"

#define FIBERS_DEVICE "/dev/fibers"

#define FIB_CONVERT IOCTL_CONVERT_THREAD_TO_FIBER
#define FIB_CREATE  IOCTL_CREATE_FIBER
#define FIB_SWITCH IOCTL_SWITCH_TO_FIBER
#define FLS_ALLOC IOCTL_FLS_ALLOC
#define FLS_GET IOCTL_FLS_GET_VALUE
#define FLS_FREE IOCTL_FLS_FREE
#define FLS_SET IOCTL_FLS_SET_VALUE