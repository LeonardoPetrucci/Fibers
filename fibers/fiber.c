#include <linux/kern_levels.h>
#include <linux/spinlock_types.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <current.h>

#include "include/fiber.h"

DEFINE_HASHTABLE(process_hash, HASH_SIZE);
spinlock_t process_spinlock;
unsigned long spinlock_flags;

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

    struct process * p;
    struct thread * t;
    struct fiber * f;

    //use spinlock for the process scan
    p = get_process(id.tgid);
    if(!p)
    {
        p = (struct process *) kmalloc(sizeof(struct process), GFP_KERNEL);
        if(!p)
        {
            return -1; //error codes are still placeholders. Change them with correct error codes.
        }
        p->tgid = id.tgid;
        p->last_fid = 0;
        hash_init(p->thread_hash);
        hash_init(p->fiber_hash);
        hash_add_rcu(process_hash, &(p->pnode), p->tgid);
    }

    t = get_thread(id.pid, p);
    if(t)
    {
        return -1;
    }
    t = (struct thread *) kmalloc(sizeof(struct thread), GFP_KERNEL);
    if(!t)
    {
        return -1;
    }
    t->pid = id.pid;
    //t->active_fid = NULL; //It will be overwritten
    hash_add_rcu(p->thread_hash, &(t->tnode), t->pid);

    f = (struct fiber *) kmalloc(sizeof(struct fiber), GFP_KERNEL);
    if(!f)
    {
        return -1;
    }
    //fields not belonging to a struct
    f->fid = p->last_fid + 1; //must do in a atomic way, should move in fiber_info struct?
    f->parent_pid = id.pid;
    f->stack_base = NULL;
    f->stack_limit = NULL;
    f->guaranteed_stack_bytes = 0;
    f->fls_data = NULL; //It needs to be allocated through fls_alloc()
    //fields belonging to the struct context
    f->fiber_context = NULL; //once created, I assume no context for a new fiber
    //fields belonging to the struct activation_context
    f->fiber_activation_context.entry_point = task_pt_regs(current)->ip;
    f->fiber_activation_context.params = NULL;
    //fields belonging to the struct fiber_data
    snprintf(f->fiber_data.name, NAME_LENGHT, "%d", f->fid);
    f->fiber_data.successful_activations = 1;
    f->fiber_data.failed_activations = 0;
    f->fiber_data.running_time = 0;
    f->fiber_data.last_time_active = current->utime;
    hash_add_rcu(p->fiber_hash, &(f->fnode), f->fid);

    return f->fid;
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