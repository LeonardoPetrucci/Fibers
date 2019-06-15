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

DEFINE_HASHTABLE(groups, H_SIZE);
DEFINE_SPINLOCK(plock);
unsigned long plock_flags;

inline struct thread_group * generate_group(pid_t tgid)
{
    struct thread_group * tg;

    tg = (struct thread_group *) kmalloc(sizeof(struct thread_group), GFP_KERNEL);
    if(!tg)
    {
        return -ESRCH;
    }
    tg->tgid = tgid;
    atomic_set(&(tg->fid_count), NO_FIBERS);
    hash_init(tg->threads);
    hash_init(tg->fibers);
    hash_add_rcu(groups, &(tg->tgnode), tg->tgid);

    return tg;
}

inline struct thread * generate_thread(struct thread_group * tg, pid_t pid)
{
    struct thread * t;

    t = (struct thread *) kmalloc(sizeof(struct thread), GFP_KERNEL);
    if(!t)
    {
        return -ESRCH;
    }
    t->pid = pid;
    t->current_fid = NO_FIBERS; 
    hash_add_rcu(tg->threads, &(t->tnode), t->pid);

    return t;
}


inline struct fiber * generate_fiber(struct thread_group * tg, struct thread * t)
{
    struct fiber * f;

    f = (struct fiber *) kmalloc(sizeof(struct fiber), GFP_KERNEL);
    if(!f)
    {
        return -ESRCH;
    }

    PREPARE_GENERICS(f, tg)
    PREPARE_CONTEXT(f);
    PREPARE_FLS(f);
    PREPARE_INFO(f, t);

    hash_add_rcu(tg->fibers, &(f->fnode), f->fid);

    return f;
}


inline struct thread_group * get_group(pid_t tgid)
{
    struct thread_group * tg;
    hash_for_each_possible_rcu(groups, tg, tgnode, tgid)
    {
        if(!tg)
        {
            break;
        }

        if(tg->tgid == tgid)
        {
            break;
        }
    }
    
    return tg;
}

inline struct thread * get_thread(pid_t pid, struct thread_group * tg)
{
    struct thread * t;
    hash_for_each_possible_rcu(tg->threads, t, tnode, pid)
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

inline struct fiber * get_fiber(pid_t fid, struct thread_group * tg)
{
    struct fiber * f;
    hash_for_each_possible_rcu(tg->fibers, f, fnode, fid)
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

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    spin_lock_irqsave(&plock, plock_flags);
    tg = get_group(tgid);
    if(!tg)
    {
        tg = generate_group(tgid);
    }
    spin_unlock_irqrestore(&plock, plock_flags);
    
    t = get_thread(pid, tg);
    if(t)
    {
        return -EEXIST;
    }
    t = generate_thread(tg, pid);

    f = generate_fiber(tg, t);
    
    
    if (!spin_trylock(&(f->flock)) || atomic_read(&f->info.state) != 0)
    {
        atomic_inc(&f->info.failed_activations);
        return -EAGAIN;
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

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    tg = get_group(tgid);
    if(!tg)
    {

        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {

        return -ESRCH;
    }
    
    f = generate_fiber(tg, t);

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

    struct thread_group * tg;
    struct thread * t;
    struct fiber * old_f;
    struct fiber * new_f;

    struct pt_regs * old_regs;
    struct fpu * old_fpu;
    struct fpu * new_fpu;
    struct fxregs_state * new_fx_regs;

    tg = get_group(tgid);
    if(!tg)
    {

        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {

        return -ESRCH;
    }

    new_f = get_fiber(next_fiber, tg);
    if(!new_f)
    {

        return -ESRCH;
    }

    if (!spin_trylock(&(new_f->flock)) || atomic_read(&new_f->info.state) != 0)
    {
        atomic_inc(&new_f->info.failed_activations);
        return -EAGAIN;
    }

    old_f = get_fiber(t->current_fid, tg);
    if(!old_f)
    {

        return -ESRCH;
    }


    ACTIVATE_FIBER_FROM_THREAD(new_f, t);

    old_regs = task_pt_regs(current);
    memcpy(&(old_f->context.regs), old_regs, sizeof(struct pt_regs));

    old_fpu = &(old_f->context.fpu);
    copy_fxregs_to_kernel(old_fpu);

    old_f->info.total_running_time += current->utime - old_f->info.last_activation_time;
    atomic_set(&old_f->info.state, 0);
    spin_unlock(&(old_f->flock));

    memcpy(old_regs, &(new_f->context.regs), sizeof(struct pt_regs));

    new_fpu = &(new_f->context.fpu);
    new_fx_regs = &(new_fpu->state.fxsave);
    copy_kernel_to_fxregs(new_fx_regs);
    
    return SUCCESS;
}

long do_fls_alloc(struct task_struct * tsk)
{

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    tg = get_group(tgid);
    if(!tg)
    {
        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {
        return -ESRCH;
    }
    
    f = get_fiber(t->current_fid, tg);
    if(!f)
    {
        return -ESRCH;
    }


    long idx = find_first_zero_bit(f->fls.bmp, FLS_MAX_SIZE);
    if(idx < FLS_MAX_SIZE)
    {
        change_bit(idx, f->fls.bmp);
        return idx;
    }
    return -EINVAL;
}

long long do_fls_get_value(struct task_struct * tsk, long index)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {

        return -ESRCH;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    tg = get_group(tgid);
    if(!tg)
    {

        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {

        return -ESRCH;
    }
    
    f = get_fiber(t->current_fid, tg);
    if(!f)
    {

        return -ESRCH;
    }

    if(test_bit(index, f->fls.bmp) == 0)
    {

        return -EACCES;
    }

    return f->fls.data[index];
}

long do_fls_free(struct task_struct * tsk, long index)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -EINVAL;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    tg = get_group(tgid);
    if(!tg)
    {
        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {
        return -ESRCH;
    }
    
    f = get_fiber(t->current_fid, tg);
    if(!f)
    {
        return -ESRCH;
    }


    if(test_bit(index, f->fls.bmp) == 0)
    {
        return -EACCES;
    }

    change_bit(index, f->fls.bmp);

    return SUCCESS;
}

long do_fls_set_value(struct task_struct * tsk, long index, long long value)
{

    if(index < 0 || index >= FLS_MAX_SIZE)
    {
        return -EINVAL;
    }

    pid_t tgid;
    pid_t pid;

    tgid = task_tgid_nr(tsk);
    pid = task_pid_nr(tsk);

    struct thread_group * tg;
    struct thread * t;
    struct fiber * f;

    tg = get_group(tgid);
    if(!tg)
    {
        return -ESRCH;
    }
    
    t = get_thread(pid, tg);
    if(!t)
    {
        return -ESRCH;
    }
    
    f = get_fiber(t->current_fid, tg);
    if(!f)
    {
        return -ESRCH;
    }


    if(test_bit(index, f->fls.bmp) == 0)
    {
        return -EACCES;
    }

    f->fls.data[index] = value;

    return index;
}


int cleanup_all(void)
{
    struct thread_group * tg;
    int bkt = 0;

    hash_for_each_rcu(groups, bkt, tg, tgnode){
        if(!tg)
        {
            break;
        }
        cleanup_process(tg->tgid);
    }
    return SUCCESS;
}

int cleanup_process(pid_t tgid)
{
    struct thread_group *tg;
    struct thread *t;
    struct fiber *f;
    
    int bkt = 0;

    tg = get_group(tgid);
    if (!tg) {
        return 0;
    }

    hash_for_each_rcu(tg->threads, bkt, t, tnode){
        if(t == NULL) 
        {
            break;
        }
        hash_del_rcu(&(t->tnode));
        kfree(t);
    }

    hash_for_each_rcu(tg->fibers, bkt, f, fnode){
        if(f == NULL) 
        {
            break;
        }
        hash_del_rcu(&(f->fnode));
        kfree(f);
    }

    hash_del_rcu(&(tg->tgnode));
    kfree(tg);
    return SUCCESS;
}