#define KERNEL

#include "include/device.h"
#include "include/fiber.h"

static int fibers_device_major;
static struct device* fibers_device = NULL;
static struct class* fibers_device_class = NULL;

static struct file_operations fibers_device_fops = 
{
    .owner = THIS_MODULE,
    .open = fibers_open,
    .read = fibers_dummy_read,
    .write = fibers_dummy_write,
    .release = fibers_release,
    .unlocked_ioctl = fibers_ioctl,
};

static char *devnode(struct device *dev, umode_t *mode)
{
    if(mode)
    {
        *mode = 0666;
    }

    return kasprintf(GFP_KERNEL, "%s", dev_name(dev));
}

int fibers_open(struct inode * inodep, struct file * filep)
{
    if(!try_module_get(THIS_MODULE))
    {
        return -ENOENT;
    }

    return SUCCESS;
}

ssize_t fibers_dummy_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   return 0;
}

ssize_t fibers_dummy_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   return len;
}

int fibers_release(struct inode * inodep, struct file * filep)
{
    pid_t tgid;
    tgid = task_tgid_nr(current);
    
    cleanup_process(tgid);
    module_put(THIS_MODULE);
    
    return SUCCESS;
}

long fibers_ioctl(struct file * filep, unsigned int cmd, unsigned long args)
{
    fib_args_t fibargs;
    fls_args_t flsargs;
    pid_t new_fid;

    long error;
    long long read_value;

    switch(cmd)
    {
        case IOCTL_CONVERT_THREAD_TO_FIBER:
            error = do_convert_thread_to_fiber(current);
            break;
        
        case IOCTL_CREATE_FIBER:
            if(!access_ok(VERIFY_READ, args, sizeof(fib_args_t)))
            {
                error = -EINVAL;
                break;
            }

            if(copy_from_user(&fibargs, (void __user *) args, sizeof(fib_args_t)))
            {
                error = -EINVAL;
                break;
            }

            error = do_create_fiber(current, fibargs.stack_base, fibargs.stack_size, fibargs.entry_point, fibargs.params);
            break;
        
        case IOCTL_SWITCH_TO_FIBER:
            
            if(!access_ok(VERIFY_READ, args, sizeof(pid_t)))
            {
                error = -EINVAL;
            }
            if(copy_from_user(&new_fid, (void __user *) args, sizeof(pid_t)))
            {
                error = -EINVAL;
                break;
            }

            error = do_switch_to_fiber(current, new_fid);
            break;
        
        case IOCTL_FLS_ALLOC:
            error = do_fls_alloc(current);
            break;
        
        case IOCTL_FLS_GET_VALUE:
            if (!access_ok(VERIFY_READ, args, sizeof(fls_args_t))) {
                return -EINVAL;
            }
            if (copy_from_user(&flsargs, (void*)args, sizeof(fls_args_t))) {
                return -EINVAL;
            }

            read_value = do_fls_get_value(current, flsargs.idx);
            if (copy_to_user((void*)flsargs.value, &read_value, sizeof(long long))){
                return -EINVAL;
            }
            error = 0;
            break;
        
        case IOCTL_FLS_FREE:
            if(!access_ok(VERIFY_READ, args, sizeof(fls_args_t)))
            {
                error = -EINVAL;
                break;
            }

            if(copy_from_user(&flsargs, (void __user *) args, sizeof(fls_args_t)))
            {
                error = -EINVAL;
                break;
            }
            error = do_fls_free(current, flsargs.idx);
            break;
        
        case IOCTL_FLS_SET_VALUE:
            if(!access_ok(VERIFY_READ, args, sizeof(fls_args_t)))
            {
                error = -EINVAL;
                break;
            }

            if(copy_from_user(&flsargs, (void __user *) args, sizeof(fls_args_t))) 
            {
                error = -EINVAL;
                break;
            }
            error = do_fls_set_value(current, flsargs.idx, flsargs.value);
            break;
        
        default:
            error = -EPERM;
            break;   
    }

    return error;
}

int fibers_register_device(void)
{
    void* error;


    fibers_device_major = register_chrdev(0, DEVICE_NAME, &fibers_device_fops);
    if(fibers_device_major < 0)
    {
        goto error_major;
    }

    fibers_device_class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(error = fibers_device_class))
    {
        goto error_class;
    }
    fibers_device_class->devnode = devnode;
    

    fibers_device = device_create(fibers_device_class, NULL, MKDEV(fibers_device_major, 0), NULL, DEVICE_NAME);
    if(IS_ERR(error = fibers_device))
    {
        goto error_device;
    }

    return SUCCESS;

error_device:
    class_destroy(fibers_device_class);

error_class:
    unregister_chrdev(fibers_device_major, DEVICE_NAME);
    return PTR_ERR(error);

error_major:
    return fibers_device_major;
}

int fibers_unregister_device(void)
{
    device_destroy(fibers_device_class, MKDEV(fibers_device_major, 0));     
    class_unregister(fibers_device_class);
    class_destroy(fibers_device_class);
    unregister_chrdev(fibers_device_major, DEVICE_NAME); 
    
    return SUCCESS;
}