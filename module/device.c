#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/err.h>
#include <linux/kdev_t.h>
#include <linux/export.h>

#include "include/device.h"
#include "include/operations.h"

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

static char *devnode(struct device *dev, umode_t *mode)
{
    if(mode)
    {
        *mode = 0666; //I just open the device in readonly mode. change permissions!
    }

    return kasprintf(GFP_KERNEL, "%s", dev_name(dev));
}

int fibers_device_register(void)
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

int fibers_device_unregister(void)
{
    device_destroy(fibers_device_class, MKDEV(fibers_device_major, 0));     
    class_unregister(fibers_device_class);
    class_destroy(fibers_device_class);
    unregister_chrdev(fibers_device_major, DEVICE_NAME); 
    
    return 0;
}