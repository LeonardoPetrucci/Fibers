#include <linux/init.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "include/device.h"

extern int fibers_device_register(void);
extern int fibers_device_unregister(void);

static int __init fibers_module_init(void)
{
    fibers_device_register();
    printk(KERN_INFO "Fibers: Module successfully loaded\n");
    return 0;
}

static void __exit fibers_module_exit(void)
{
    fibers_device_unregister();
    printk(KERN_INFO "Fibers: Module successfully unloaded\n");
}

module_init(fibers_module_init);
module_exit(fibers_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonardo Petrucci <petrucci.1600764@studenti.uniroma1.it>");
MODULE_DESCRIPTION("AOSV Project for A.A 2017/2018");
MODULE_VERSION("0.1");