#include <linux/init.h>
#include <linux/kern_levels.h>
#include <linux/module.h>
#include <linux/printk.h>

#include "include/device.h"

extern int fibers_register_device(void);
extern int fibers_unregister_device(void);

static int __init fibers_init_module(void)
{
    fibers_register_device();
    printk(KERN_INFO "Fibers: Module successfully loaded\n");
    return 0;
}

static void __exit fibers_exit_module(void)
{
    fibers_unregister_device();
    printk(KERN_INFO "Fibers: Module successfully unloaded\n");
}

module_init(fibers_init_module);
module_exit(fibers_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Leonardo Petrucci <petrucci.1600764@studenti.uniroma1.it>");
MODULE_DESCRIPTION("AOSV Project for A.A 2017/2018");
MODULE_VERSION("0.1");