#include <linux/module.h>
#include <linux/export.h>

#include "include/operations.h"
#include "include/fiber.h"

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
            error = convert_thread_to_fiber();
            break;
        
        case IOCTL_CREATE_FIBER:
            error = create_fiber();
            break;
        
        case IOCTL_SWITCH_TO_FIBER:
            error = switch_to_fiber();
            break;
        
        case IOCTL_FLS_ALLOC:
            error = fls_alloc();
            break;
        
        case IOCTL_FLS_GET_VALUE:
            error = fls_get_value();
            break;
        
        case IOCTL_FLS_FREE:
            error = fls_free();
            break;
        
        case IOCTL_FLS_SET_VALUE:
            error = fls_set_value();
            break;
        
        default:
            error = -1;
            break;
    }

    return error;
}