#define KERNEL

#include "include/device.h"
#include "include/fiber.h"

static int fibers_device_major;
static struct device* fibers_device = NULL;
static struct class* fibers_device_class = NULL;

static struct file_operations fibers_device_operations = 
{
    .owner = THIS_MODULE,
    .open = fibers_open,
    .release = fibers_release,
    .unlocked_ioctl = fibers_ioctl,
};

inline id_t get_current_id(void)
{
    id_t current_id;
    current_id.pid = current->pid;
    current_id.tgid = current->tgid;

    return current_id;
}

static char *devnode(struct device *dev, umode_t *mode)
{
    if(mode)
    {
        *mode = 0666; //I just open the device in readonly mode. change permissions!
    }

    return kasprintf(GFP_KERNEL, "%s", dev_name(dev));
}

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
    struct fib_args fibargs;
    struct fls_args flsargs;
    id_t current_id = get_current_id();
    long error;

    switch(cmd)
    {
        case IOCTL_CONVERT_THREAD_TO_FIBER:
            error = do_convert_thread_to_fiber(current_id);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        case IOCTL_CREATE_FIBER:
            if(!access_ok(VERIFY_READ, &args, sizeof(struct fib_args)))
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld ACCESS ERROR.\n", error);
                break;
            }

            if(copy_from_user(&fibargs, (void __user *) args, sizeof(struct fib_args))) //I don't like that void pointer...
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld COPY ERROR.\n", error);
                break;
            }

            error = do_create_fiber(current_id, fibargs.stack_size, fibargs.entry_point, fibargs.params);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        case IOCTL_SWITCH_TO_FIBER:
            //error = do_switch_to_fiber(current_id);
            break;
        
        case IOCTL_FLS_ALLOC:
            error = do_fls_alloc(current_id);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        case IOCTL_FLS_GET_VALUE:
            if(!access_ok(VERIFY_READ, &args, sizeof(struct fls_args)))
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }

            if(copy_from_user(&flsargs, (void __user *) args, sizeof(struct fls_args))) //I don't like that void pointer...
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }
            error = do_fls_get_value(current_id, flsargs.idx);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        case IOCTL_FLS_FREE:
            if(!access_ok(VERIFY_READ, &args, sizeof(struct fls_args)))
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }

            if(copy_from_user(&flsargs, (void __user *) args, sizeof(struct fls_args))) //I don't like that void pointer...
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }
            error = do_fls_free(current_id, flsargs.idx);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        case IOCTL_FLS_SET_VALUE:
            if(!access_ok(VERIFY_READ, &args, sizeof(struct fls_args)))
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }

            if(copy_from_user(&flsargs, (void __user *) args, sizeof(struct fls_args))) //I don't like that void pointer...
            {
                error = -1;
                printk(KERN_INFO "Call Result %ld", error);
                break;
            }
            error = do_fls_set_value(current_id, flsargs.idx, flsargs.value);
            printk(KERN_INFO "Call Result %ld", error);
            break;
        
        default:
            error = -2;
            printk(KERN_INFO "Call Result %ld", error);
            break;
    }

    return error;
}

int fibers_register_device(void)
{
    void* error;

    printk(KERN_INFO "Fibers: Initializing fibers device registration.\n");

    fibers_device_major = register_chrdev(0, DEVICE_NAME, &fibers_device_operations);
    if(fibers_device_major < 0)
    {
        printk(KERN_ALERT "Fibers: Failed to register a major number for fibers device.\n");
        goto error_major;
    }
    printk(KERN_INFO "Fibers: Successfully registered major number %d for fibers device.\n", fibers_device_major);

    fibers_device_class = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(error = fibers_device_class))
    {
        printk(KERN_ALERT "Fibers: Failed to register a class for fibers device.\n");
        goto error_class;
    }
    fibers_device_class->devnode = devnode;
    printk(KERN_INFO "Fibers: Successfully registered class %s for fibers device.", CLASS_NAME);
    

    fibers_device = device_create(fibers_device_class, NULL, MKDEV(fibers_device_major, 0), NULL, DEVICE_NAME);
    if(IS_ERR(error = fibers_device))
    {
        printk(KERN_ALERT "Fibers: Failed to register a device for fibers device.\n");
        goto error_device;
    }
    printk(KERN_INFO "Fibers: Successfully registered device %s for fibers device.\n", DEVICE_NAME);

    return 0;

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
    
    return 0;
}