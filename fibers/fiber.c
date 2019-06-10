#include <linux/kern_levels.h>
#include <linux/spinlock_types.h>
#include <linux/////printk.h>
#include <linux/slab.h>
#include <asm/fpu/internal.h>
#include <asm/current.h>
#include <linux/bitops.h>

#include "include/fiber.h"
#include "include/device.h"

DEFINE_HASHTABLE(process_hash, H_SIZE);
DEFINE_SPINLOCK(plock);
unsigned long plock_flags;

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
    //printk(KERN_INFO "Fibers : Invoked function convert_thread_to_fiber.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    spin_lock_irqsave(&plock, plock_flags);
    p = get_process(id.tgid);
    if(!p)
    {
        p = (struct process *) kmalloc(sizeof(struct process), GFP_KERNEL);
        if(!p)
        {
            //printk(KERN_INFO "process creation error.\n");
            return -1; //error codes are still placeholders. Change them with correct error codes.
        }
        p->tgid = id.tgid;
        p->last_fid = 0;
        hash_init(p->thread_hash);
        hash_init(p->fiber_hash);

        hash_add_rcu(process_hash, &(p->pnode), p->tgid);
    }
    spin_unlock_irqrestore(&plock, plock_flags);

    t = get_thread(id.pid, p);
    if(t)
    {
        //printk(KERN_INFO "get_thread error.\n");
        return -1;
    }
    t = (struct thread *) kmalloc(sizeof(struct thread), GFP_KERNEL);
    if(!t)
    {
        //printk(KERN_INFO "thread creation error.\n");
        return -1;
    }
    t->pid = id.pid;

    hash_add_rcu(p->thread_hash, &(t->tnode), t->pid);

    f = (struct fiber *) kmalloc(sizeof(struct fiber), GFP_KERNEL);
    if(!f)
    {
        return -1;
    }
    f->fid = p->last_fid++;
    f->reference_pid = id.pid;
    f->flock = __SPIN_LOCK_UNLOCKED(flock);
    f->stack_base = NULL;
    f->stack_limit = 0;
    f->guaranteed_stack_bytes = 0;

    //f->context = NULL;

    memset(f->fls.data, 0, sizeof(long long) * FLS_MAX_SIZE);
    bitmap_zero(f->fls.bmp, FLS_MAX_SIZE);

    snprintf(f->info.name, NAME_LENGHT, "%d", f->fid);
    f->info.parent_pid = id.pid;
    f->info.state = 1;
    f->info.successful_activations = 1;
    f->info.failed_activations = 0;
    f->info.last_running_time = 0;
    f->info.total_running_active = current->utime;
    f->info.entry_point = task_pt_regs(current)->ip;
    f->info.param = NULL;

    hash_add_rcu(p->fiber_hash, &(f->fnode), f->fid);

    t->current_fid = f->fid;
    //printk(KERN_INFO "Fibers : Completed function create_fiber.\n");

    if (!spin_trylock(&f->flock))
    {
        return -1;
    }
    
    return f->fid;
}

fid_t do_create_fiber(id_t id, void * stack_base, size_t stack_size, unsigned long entry_point, void * param)
{
    //printk(KERN_INFO "Fibers : Invoked function create_fiber.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    //printk(KERN_INFO "Arguments identification: stack_size %lu, entry_point %lu, param %lu.\n", (unsigned long) stack_size, entry_point, (unsigned long) param);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(id.tgid);
    if(!p)
    {
        //printk(KERN_INFO "get_process error.\n");
        return -1;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        //printk(KERN_INFO "get_thread error.\n");
        return -1;
    }

    f = (struct fiber *) kmalloc(sizeof(struct fiber), GFP_KERNEL);
    if(!f)
    {
        return -1;
    }
    
    f->fid = p->last_fid++;
    f->reference_pid = 0;
    f->flock = __SPIN_LOCK_UNLOCKED(flock);
    f->stack_base = stack_base;
    f->stack_limit = (unsigned long) f->stack_base + stack_size; //to change?
    f->guaranteed_stack_bytes = stack_size;

    memcpy(&(f->context.regs), task_pt_regs(current), sizeof(struct pt_regs));
    copy_fxregs_to_kernel(&(f->context.fpu));
    f->context.regs.ip = entry_point;
    f->info.entry_point = f->context.regs.ip;
    
    f->context.regs.di = (unsigned long) param;
    f->context.regs.sp = f->stack_limit - 8;
    f->context.regs.bp = f->context.regs.sp;

    memset(f->fls.data, 0, sizeof(long long) * FLS_MAX_SIZE);
    bitmap_zero(f->fls.bmp, FLS_MAX_SIZE);

    snprintf(f->info.name, NAME_LENGHT, "%d", f->fid);
    f->info.parent_pid = id.pid;
    f->info.state = 0;
    f->info.successful_activations = 0;
    f->info.failed_activations = 0;
    f->info.last_running_time = 0;
    f->info.total_running_active = 0;
    f->info.entry_point = f->context.regs.ip;
    f->info.param = param;

    hash_add_rcu(p->fiber_hash, &(f->fnode), f->fid);

    //no spin_trylock here!! the fiber is only created, it is NOT scheduled yet.

    return f->fid;
}

fid_t do_switch_to_fiber(id_t id, fid_t next_fiber)
{
    //printk(KERN_INFO "Fibers : Invoked function switch_to_fiber.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    //printk(KERN_INFO "Arguments identification: next_fiber %d", next_fiber);

    struct process * p;
    struct thread * t;
    struct fiber * old_fiber;
    struct fiber * new_fiber;

    struct pt_regs * old_regs;
    struct fpu * old_fpu;
    struct fpu * new_fpu;
    struct fxregs_state * new_fx_regs;

    p = get_process(id.tgid);
    if(!p)
    {
        //printk(KERN_INFO "get_process error.\n");
        return -1;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        //printk(KERN_INFO "get_thread error.\n");
        return -1;
    }
    
    old_fiber = get_fiber(t->current_fid, p);
    if(!old_fiber)
    {
        //printk(KERN_INFO "get_fiber error for old_fiber.\n");
        return -1;
    }

    //printk(KERN_INFO "Old Fiber Identification: fid %d.\n", old_fiber->fid);

    new_fiber = get_fiber(next_fiber, p); //I would like to be able to switch to a fiber from another process too...
    if(!new_fiber)
    {
        //printk(KERN_INFO "get_process error for new_fiber.\n");
        return -1;
    }

    //printk(KERN_INFO "New Fiber Identification: fid %d.\n", new_fiber->fid);    
    //printk(KERN_INFO "spin_trylock with destination reference_pid: %d\n", new_fiber->reference_pid);
    if (!spin_trylock(&new_fiber->flock) || new_fiber->reference_pid != 0)
    {
        new_fiber->info.failed_activations++;
        //printk(KERN_INFO "spin_trylock not successful. reference_pid: %d\n", new_fiber->reference_pid);
        return -1;
    }

    new_fiber->reference_pid = id.pid;
    t->current_fid = new_fiber->fid;

    old_fiber->info.total_running_active = current->utime - old_fiber->info.last_running_time;

    old_regs = task_pt_regs(current);
    memcpy(&old_fiber->context.regs, old_regs, sizeof(struct pt_regs));

    old_fpu = &(old_fiber->context.fpu);
    copy_fxregs_to_kernel(old_fpu);

    old_fiber->reference_pid = 0;
    old_fiber->info.state = 0;
    spin_unlock(&old_fiber->flock);

    new_fiber->info.state = 1;

    memcpy(old_regs, &new_fiber->context.regs, sizeof(struct pt_regs));

    new_fpu = &(new_fiber->context.fpu);
    new_fx_regs = &(new_fpu->state.fxsave);
    copy_kernel_to_fxregs(new_fx_regs);
    
    return new_fiber->fid;
}

long do_fls_alloc(id_t id)
{
    //printk(KERN_INFO "Fibers : Invoked function fls_alloc.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(id.tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }

    //printk(KERN_INFO "Fiber Identification: fid %d.\n", f->fid);

    long idx = find_first_zero_bit(f->fls.bmp, FLS_MAX_SIZE);
    if(idx < FLS_MAX_SIZE)
    {
        change_bit(idx, f->fls.bmp);
        return idx;
    }
    //printk("CANNOT ALLOC\n");
    return -1;
}

long long do_fls_get_value(id_t id, long index)
{
    //printk(KERN_INFO "Fibers : Invoked function fls_get_value.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    //printk(KERN_INFO "Arguments identification: index %ld.\n", index);

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        //printk(KERN_INFO "Invalid index");
        return -3;
    }

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(id.tgid);
    if(!p)
    {
        //printk(KERN_INFO "get_process error.\n");
        return -4;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        //printk(KERN_INFO "get_thread error.\n");
        return -5;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        //printk(KERN_INFO "get_fiber error.\n");
        return -6;
    }

    //printk(KERN_INFO "Fiber Identification: fid %d.\n", f->fid);

    if(test_bit(index, f->fls.bmp) == 0)
    {
        //printk(KERN_INFO "bitmap error.\n");
        return -7;
    }

    return f->fls.data[index];
}

long do_fls_free(id_t id, long index)
{
    //printk(KERN_INFO "Fibers : Invoked function fls_free.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    //printk(KERN_INFO "Arguments identification: index %ld.\n", index);

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -1;
    }

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(id.tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }

    //printk(KERN_INFO "Fiber Identification: fid %d.\n", f->fid);

    if(test_bit(index, f->fls.bmp) == 0)
    {
        return -1;
    }

    change_bit(index, f->fls.bmp);

    return 0;
}

long do_fls_set_value(id_t id, long index, long long value)
{
    //printk(KERN_INFO "Fibers : Invoked function fls_set_value.\n");
    //printk(KERN_INFO "Process Identification: pid %d, tgid %d.\n", id.pid, id.tgid);
    //printk(KERN_INFO "Arguments identification: index %ld, value %lld\n", index, value);

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -1;
    }

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(id.tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(id.pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }

    //printk(KERN_INFO "Fiber Identification: fid %d.\n", f->fid);

    if(test_bit(index, f->fls.bmp) == 0)
    {
        return -1;
    }

    f->fls.data[index] = value;

    return index;
}

int cleanup_all(void)
{
    struct process * p;
    int bkt = 0;

    hash_for_each_rcu(process_hash, bkt, p, pnode){
        if(p == NULL)
        {
            break;
        }
        cleanup_process(p->tgid);
    }
    return 0;
}

int cleanup_process(tgid_t tgid)
{
        struct process *p;
        struct thread *t;
        struct fiber *f;
        
        int bkt = 0;

        p = get_process(tgid);
        if (p == NULL) {
            return 0;
        }

        hash_for_each_rcu(p->thread_hash, bkt, t, tnode){
            if(t == NULL) 
            {
                break;
            }
            hash_del_rcu(&(t->tnode));
            kfree(t);
        }

        hash_for_each_rcu(p->fiber_hash, bkt, f, fnode){
            if(f == NULL) 
            {
                break;
            }
            hash_del_rcu(&(f->fnode));
            kfree(f);
        }

        hash_del_rcu(&(p->pnode));
        printk(KERN_INFO "Process %d left the party.\n", p->tgid);
        kfree(p);
        return 0;
}