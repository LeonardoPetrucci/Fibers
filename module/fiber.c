#include <linux/kern_levels.h>
#include <linux/printk.h>
#include "include/fiber.h"

int convert_thread_to_fiber()
{
    printk(KERN_INFO "Invoked function convert_thread_to_fiber.\n");
    return 0;
}

int create_fiber()
{
    printk(KERN_INFO "Invoked function create_fiber.\n");
    return 0;
}

int switch_to_fiber()
{
    printk(KERN_INFO "Invoked function switch_to_fiber.\n");
    return 0;
}

int fls_alloc()
{
    printk(KERN_INFO "Invoked function fls_alloc.\n");
    return 0;
}

int fls_get_value()
{
    printk(KERN_INFO "Invoked function fls_get_value.\n");
    return 0;
}

int fls_free()
{
    printk(KERN_INFO "Invoked function fls_free.\n");
    return 0;
}

int fls_set_value()
{
    printk(KERN_INFO "Invoked function fls_set_value.\n");
    return 0;
}