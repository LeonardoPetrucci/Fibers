#include <linux/module.h>
#include <linux/export.h>

#include "include/operations.h"

int fibers_open(struct inode * inodep, struct file * filep)
{
    if(!try_module_get(THIS_MODULE))
    {
        return -1;
    }

    return 0;
}

int fibers_release(struct inode * inodep, struct file * filep)
{
    module_put(THIS_MODULE);
    
    return 0;
}

long fibers_ioctl(struct file * filep, unsigned int cmd, unsigned long args)
{
    int error = 0;

    switch(cmd)
    {
        case IOCTL_CONVERT_THREAD_TO_FIBER:
            break;
        
        case IOCTL_CREATE_FIBER:
            break;
        
        case IOCTL_SWITCH_TO_FIBER:
            break;
        
        case IOCTL_FLS_ALLOC:
            break;
        
        case IOCTL_FLS_GET_VALUE:
            break;
        
        case IOCTL_FLS_FREE:
            break;
        
        case IOCTL_FLS_SET_VALUE:
            break;
        
        default:
            error = -1;
            break;
    }

    return error;
}