#include <linux/kern_levels.h>
#include <linux/printk.h>

#include "include/fiber.h"

DEFINE_HASHTABLE(process_hash, HASHTABLE_BITS);

inline struct process * get_process(tgid_t tgid)
{
    struct process * p;
    hash_for_each_possible_rcu(process_hash, p, pnode, tgid)
    {
        if(p == NULL)
        {
            break;
        }

        if(p->tgid == tgid)
        {
            break;
        }
    }
    
    return p;
}

inline struct thread * get_thread(pid_t pid, struct process * p)
{
    struct thread * t;
    hash_for_each_possible_rcu(p->thread_hash, t, tnode, pid)
    {
        if(t == NULL)
        {
            break;
        }

        if(t->pid == pid)
        {
            break;
        }
    }

    return t;
}

inline struct fiber * get_fiber(fid_t fid, struct process * p)
{
    struct fiber * f;
    hash_for_each_possible_rcu(p->fiber_hash, f, fnode, fid)
    {
        if(f == NULL)
        {
            break;
        }

        if(f->fid == fid)
        {
            break;
        }
    }

    return f;
}

fid_t do_convert_thread_to_fiber(id_t id)
{
    printk(KERN_INFO "Invoked function convert_thread_to_fiber.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    return id.pid;
}

fid_t do_create_fiber(id_t id, size_t stack_size, unsigned long entry_point, void * param)
{
    printk(KERN_INFO "Invoked function create_fiber.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    printk(KERN_INFO "Arguments identification: stack_size %lu, entry_point %lu, param %lu.\n", (unsigned long) stack_size, entry_point, (unsigned long) param);
    return 0;
}

fid_t do_switch_to_fiber(id_t id, fid_t next_fiber)
{
    printk(KERN_INFO "Invoked function switch_to_fiber.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    printk(KERN_INFO "Arguments identification: next_fiber %d", next_fiber);
    return 0;
}

long do_fls_alloc(id_t id)
{
    printk(KERN_INFO "Invoked function fls_alloc.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    return 0;
}

long long do_fls_get_value(id_t id, long index)
{
    printk(KERN_INFO "Invoked function fls_get_value.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    printk(KERN_INFO "Arguments identification: index %ld.\n", index);
    return 0;
}

long do_fls_free(id_t id, long index)
{
    printk(KERN_INFO "Invoked function fls_free.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    printk(KERN_INFO "Arguments identification: index %ld.\n", index);
    return 0;
}

long do_fls_set_value(id_t id, long index, long long value)
{
    printk(KERN_INFO "Invoked function fls_set_value.\n");
    printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    printk(KERN_INFO "Arguments identification: index %ld, value %lld\n", index, value);
    return 0;
}