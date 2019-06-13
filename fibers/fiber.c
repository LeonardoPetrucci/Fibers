#define KERNEL

#include <linux/kern_levels.h>
#include <linux/spinlock_types.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <asm/fpu/internal.h>
#include <asm/current.h>
#include <linux/bitops.h>

#include "include/fiber.h"
#include "include/device.h"

DEFINE_HASHTABLE(process_hash, H_SIZE);
DEFINE_SPINLOCK(plock);
unsigned long plock_flags;

inline struct process * generate_group(pid_t tgid)
{
    struct process * p;

    p = (struct process *) kmalloc(sizeof(struct process), GFP_KERNEL);
    if(!p)
    {
        return -1;
    }
    p->tgid = tgid;
    p->last_fid = 0;
    hash_init(p->thread_hash);
    hash_init(p->fiber_hash);
    hash_add_rcu(process_hash, &(p->pnode), p->tgid);

    return p;
}

inline struct thread * generate_thread(struct process * group, pid_t pid)
{
    struct thread * t;

    t = (struct thread *) kmalloc(sizeof(struct thread), GFP_KERNEL);
    if(!t)
    {
        return -1;
    }
    t->pid = pid;
    t->current_fid = NO_FIBERS; 
    hash_add_rcu(group->thread_hash, &(t->tnode), t->pid);

    return t;
}


inline struct fiber * generate_fiber(struct process * group, struct thread * t)
{
    struct fiber * f;

    f = (struct fiber *) kmalloc(sizeof(struct fiber), GFP_KERNEL);
    if(!f)
    {
        return -1;
    }

    f->fid = group->last_fid++;
    f->flock = __SPIN_LOCK_UNLOCKED(flock);
    f->stack_base = NULL;
    f->stack_limit = 0;
    f->guaranteed_stack_bytes = 0;

    CONTEXT(f);
    FLS(f);
    INFO(f, t->pid);

    hash_add_rcu(group->fiber_hash, &(f->fnode), f->fid);

    return f;
}


inline struct process * get_process(pid_t tgid)
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

inline struct fiber * get_fiber(pid_t fid, struct process * p)
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

pid_t do_convert_thread_to_fiber(struct task_struct * tsk)
{
    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    spin_lock_irqsave(&plock, plock_flags);
    p = get_process(tgid);
    if(!p)
    {
        p = generate_group(tgid);
    }
    spin_unlock_irqrestore(&plock, plock_flags);
    
    t = get_thread(pid, p);
    if(t)
    {
        return -1;
    }
    t = generate_thread(p, pid);

    f = generate_fiber(p, t);
    
    
    if (!spin_trylock(&(f->flock)) || f->info.state != 0)
    {
        f->info.failed_activations++;
        return -1;
    }
    


    ACTIVATE_FIBER_FROM_THREAD(f, t);

    return f->fid;
}

pid_t do_create_fiber(struct task_struct * tsk, void * stack_base, size_t stack_size, unsigned long entry_point, void * param)
{

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(tgid);
    if(!p)
    {

        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {

        return -1;
    }
    
    f = generate_fiber(p, t);

    ALLOC_STACK(f, stack_base, stack_size);
  
    ALLOC_CONTEXT(f, entry_point, param);
 
    return f->fid;
}

int do_switch_to_fiber(struct task_struct * tsk, pid_t next_fiber)
{

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * old_fiber;
    struct fiber * new_fiber;

    struct pt_regs * old_regs;
    struct fpu * old_fpu;
    struct fpu * new_fpu;
    struct fxregs_state * new_fx_regs;

    p = get_process(tgid);
    if(!p)
    {

        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {

        return -1;
    }

    new_fiber = get_fiber(next_fiber, p);
    if(!new_fiber)
    {

        return -1;
    }

    if (!spin_trylock(&(new_fiber->flock)) || new_fiber->info.state != 0)
    {
        new_fiber->info.failed_activations++;
        printk(KERN_INFO "Thread %d cannot switch from fiber %d to fiber %d (Used by thread %d)",t->pid, t->current_fid, new_fiber->fid, new_fiber->info.state);
        return -1;
    }

    old_fiber = get_fiber(t->current_fid, p);
    if(!old_fiber)
    {

        return -1;
    }


    ACTIVATE_FIBER_FROM_THREAD(new_fiber, t);

    old_regs = task_pt_regs(current);
    memcpy(&(old_fiber->context.regs), old_regs, sizeof(struct pt_regs));

    old_fpu = &(old_fiber->context.fpu);
    copy_fxregs_to_kernel(old_fpu);

    old_fiber->info.total_running_active = (current->utime) - (old_fiber->info.last_activation_time);
    old_fiber->info.state = 0;
    spin_unlock(&(old_fiber->flock));

    memcpy(old_regs, &(new_fiber->context.regs), sizeof(struct pt_regs));

    new_fpu = &(new_fiber->context.fpu);
    new_fx_regs = &(new_fpu->state.fxsave);
    copy_kernel_to_fxregs(new_fx_regs);
    
    printk(KERN_INFO "Thread %d switches from fiber %d to fiber %d", t->pid, old_fiber->fid, new_fiber->fid);
    return 0;
}

long do_fls_alloc(struct task_struct * tsk)
{

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }


    long idx = find_first_zero_bit(f->fls.bmp, FLS_MAX_SIZE);
    if(idx < FLS_MAX_SIZE)
    {
        change_bit(idx, f->fls.bmp);
        return idx;
    }
    return -1;
}

long long do_fls_get_value(struct task_struct * tsk, long index)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {

        return -1;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(tgid);
    if(!p)
    {

        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {

        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {

        return -1;
    }

    if(test_bit(index, f->fls.bmp) == 0)
    {

        return -1;
    }

    return f->fls.data[index];
}

long do_fls_free(struct task_struct * tsk, long index)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -1;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }


    if(test_bit(index, f->fls.bmp) == 0)
    {
        return -1;
    }

    change_bit(index, f->fls.bmp);

    return 0;
}

long do_fls_set_value(struct task_struct * tsk, long index, long long value)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -1;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct process * p;
    struct thread * t;
    struct fiber * f;

    p = get_process(tgid);
    if(!p)
    {
        return -1;
    }
    
    t = get_thread(pid, p);
    if(!t)
    {
        return -1;
    }
    
    f = get_fiber(t->current_fid, p);
    if(!f)
    {
        return -1;
    }


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

int cleanup_process(pid_t tgid)
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