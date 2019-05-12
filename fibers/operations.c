#include <linux/module.h>
#include <linux/export.h>
#include <linux/uaccess.h>

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
    fib_args_t fib_args;
    fls_args_t fls_args;
    long error = 0;

    switch(cmd)
    {
        case IOCTL_CONVERT_THREAD_TO_FIBER:
            error = convert_thread_to_fiber();
            break;
        
        case IOCTL_CREATE_FIBER:
            if(!access_ok(VERIFY_READ, &args, sizeof(fib_args_t)))
            {
                error = -1;
                break;
            }

            if(copy_from_user(&fib_args, (void __user *) args, sizeof(fib_args_t))) //I don't like that void pointer...
            {
                error = -1;
                break;
            }

            error = create_fiber(fib_args.stack_size, fib_args.entry_point, fib_args.params);
            break;
        
        case IOCTL_SWITCH_TO_FIBER:
            error = switch_to_fiber();
            break;
        
        case IOCTL_FLS_ALLOC:
            error = fls_alloc();
            break;
        
        case IOCTL_FLS_GET_VALUE:
            if(!access_ok(VERIFY_READ, &args, sizeof(fls_args_t)))
            {
                error = -1;
                break;
            }

            if(copy_from_user(&fls_args, (void __user *) args, sizeof(fls_args_t))) //I don't like that void pointer...
            {
                error = -1;
                break;
            }
            error = fls_get_value(fls_args.idx);
            break;
        
        case IOCTL_FLS_FREE:
            if(!access_ok(VERIFY_READ, &args, sizeof(fls_args_t)))
            {
                error = -1;
                break;
            }

            if(copy_from_user(&fls_args, (void __user *) args, sizeof(fls_args_t))) //I don't like that void pointer...
            {
                error = -1;
                break;
            }
            error = fls_free(fls_args.idx);
            break;
        
        case IOCTL_FLS_SET_VALUE:
            if(!access_ok(VERIFY_READ, &args, sizeof(fls_args_t)))
            {
                error = -1;
                break;
            }

            if(copy_from_user(&fls_args, (void __user *) args, sizeof(fls_args_t))) //I don't like that void pointer...
            {
                error = -1;
                break;
            }
            error = fls_set_value(fls_args.idx, fls_args.value);
            break;
        
        default:
            error = -1;
            break;
    }

    return error;
}