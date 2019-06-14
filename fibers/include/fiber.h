#ifndef FIBER_H
#define FIBER_H

#ifdef KERNEL

#include <linux/ptrace.h>
#include <linux/hashtable.h>
#include <linux/spinlock_types.h>
#include <linux/bitmap.h>
#include <linux/slab.h>

#define NO_FIBERS -1
#define H_SIZE 8
#define NAME_LENGHT 256
#define FLS_MAX_SIZE 4096

/*******************************************************************************/
/*   Original fiber struct from dll/win32/kernel32/client/fiber.c (ReactOS)    */
/*******************************************************************************/

/*
typedef struct _FIBER                                    
 {                                                        
     PVOID FiberData;                                     
     struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
     PVOID StackBase;                                     
     PVOID StackLimit;                                    
     PVOID DeallocationStack;                            
     PREPARE_CONTEXT FiberContext;                                
     PVOID Wx86Tib;                                      
     struct _ACTIVATION_CONTEXT_STACK *ActivationContextStackPointer; 
     PVOID FlsData;                                       
     ULONG GuaranteedStackBytes;                         
     ULONG TebFlags;                                     
 } FIBER, *PFIBER;
 */

/*******************************************************************************/
/*             Models for fibers and auxiliar parent structs                   */
/*******************************************************************************/

#define PREPARE_GENERICS(f, g) {    \
    f->fid = atomic_inc_return(&(g->fid_count)); \
    f->flock = __SPIN_LOCK_UNLOCKED(flock); \
    f->stack_base = NULL;   \
    f->stack_limit = 0; \
    f->guaranteed_stack_bytes = 0;  \
}

//always used
#define PREPARE_CONTEXT(f) {                                                            \
    memcpy(&(f->context.regs), task_pt_regs(current), sizeof(struct pt_regs));  \
    copy_fxregs_to_kernel(&(f->context.fpu));                                   \
}

#define PREPARE_FLS(f) {                                                                \
    memset(f->fls.data, 0, sizeof(long long) * FLS_MAX_SIZE);                   \
    bitmap_zero(f->fls.bmp, FLS_MAX_SIZE);                                      \
}

#define ALLOC_CONTEXT(f, ep, arg) {                                             \
    f->context.regs.ip = ep;                                                    \
    f->context.regs.di = (unsigned long) arg;                                   \
    f->context.regs.sp = f->stack_limit - 8;                                    \
    f->context.regs.bp = f->context.regs.sp;                                    \
    f->info.entry_point = f->context.regs.ip;                                   \
    f->info.param = arg;                                                        \
}

#define ALLOC_STACK(f, sb, ss) {                                                \
    f->stack_base = sb;                                                         \
    f->stack_limit = (unsigned long) f->stack_base + ss;                        \
    f->guaranteed_stack_bytes = ss;                                             \
}

#define PREPARE_INFO(f, pid) {                                                          \
    snprintf(f->info.name, NAME_LENGHT, "%d", f->fid);                          \
    f->info.parent = pid;                                                   \
    atomic_set(&f->info.state, 0);                                                          \
    f->info.successful_activations = 0;                                         \
    atomic_set(&f->info.failed_activations, 0);                                             \
    f->info.last_activation_time = 0;                                           \
    f->info.total_running_time = 0;                                           \
    f->info.entry_point = task_pt_regs(current)->ip;                            \
    f->info.param = (void *) task_pt_regs(current)->di;                                                       \
}

#define ACTIVATE_FIBER_FROM_THREAD(f, t) {                                      \
    atomic_set(&f->info.state, t->pid);                                                     \
    f->info.successful_activations++;                                           \
    f->info.last_activation_time = (unsigned long) current->utime;                              \
    t->current_fid = f->fid;                                                    \
}


struct thread_group
{
    pid_t tgid;
    atomic_t fid_count;
    DECLARE_HASHTABLE(threads, H_SIZE);
    DECLARE_HASHTABLE(fibers, H_SIZE);

    struct hlist_node gnode;
};

struct thread
{
    pid_t pid;
    pid_t current_fid;

    struct hlist_node tnode;
};

typedef struct info_s {
    char name[NAME_LENGHT];
    pid_t parent;
    atomic_t state;
    unsigned long successful_activations;
    atomic_t failed_activations;
    unsigned long last_activation_time;
    unsigned long total_running_time;
    unsigned long entry_point;
    void * param;

}info_t;

typedef struct context_s {
    struct pt_regs regs;
    struct fpu fpu;

}context_t;

typedef struct fls_s {
    long long data[FLS_MAX_SIZE];
    DECLARE_BITMAP(bmp, FLS_MAX_SIZE)
    ;

}fls_t;


struct fiber
{
    pid_t fid;
    spinlock_t flock;
    unsigned long flock_flags;
    void * stack_base;
    unsigned long stack_limit;
    unsigned long guaranteed_stack_bytes;
    context_t context;
    fls_t fls;
    info_t info;
    
    struct hlist_node fnode;
};

inline struct thread_group * get_group(pid_t tgid);
inline struct thread * get_thread(pid_t pid, struct thread_group * g);
inline struct fiber * get_fiber(pid_t fid, struct thread_group * g);

inline struct thread_group * generate_group(pid_t tgid);
inline struct thread * generate_thread(struct thread_group * g, pid_t pid);
inline struct fiber * generate_fiber(struct thread_group * g, struct thread * t);

pid_t do_convert_thread_to_fiber(struct task_struct * tsk);
pid_t do_create_fiber(struct task_struct * tsk, void * stack_base, size_t stack_size, unsigned long entry_point, void * param);
int do_switch_to_fiber(struct task_struct * tsk, pid_t next_fiber);
long do_fls_alloc(struct task_struct * tsk);
long long do_fls_get_value(struct task_struct * tsk, long index);
long do_fls_free(struct task_struct * tsk, long index);
long do_fls_set_value(struct task_struct * tsk, long index, long long value);

int cleanup_all(void);
int cleanup_process(pid_t tgid);

#endif /* KERNEL */

typedef struct fib_args_s {
    void * stack_base;
    size_t stack_size;
    unsigned long entry_point;
    void * params;
}fib_args_t;

typedef struct fls_args_s {
    long idx;
    long long value;
}fls_args_t;

#endif /* FIBER_H */