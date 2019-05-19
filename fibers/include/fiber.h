#ifndef KERNEL
#define KERNEL
#endif

#include <linux/ptrace.h>
#include <linux/hashtable.h>
#include <linux/spinlock_types.h>

#include "types.h"

#define NAME_LENGHT 256
#define HASH_SIZE 10

struct process
{
    tgid_t tgid;
    fid_t last_fid;
    DECLARE_HASHTABLE(thread_hash, HASH_SIZE);
    DECLARE_HASHTABLE(fiber_hash, HASH_SIZE);
    struct hlist_node pnode;
};

struct thread
{
    pid_t pid;
    fid_t active_fid;
    struct hlist_node tnode;
};

struct fiber_info
{
    char name[NAME_LENGHT];
    unsigned long successful_activations;
    unsigned long failed_activations;
    unsigned long running_time;
    unsigned long last_time_active;
};

struct context
{
    struct pt_regs * cpu_context;
    struct fpu fpu_context;
};

struct activation_context
{
    unsigned long entry_point;
    void * params;
};

/*
typedef struct _FIBER                                    
 {                                                        
     PVOID FiberData;                                     
     struct _EXCEPTION_REGISTRATION_RECORD *ExceptionList;
     PVOID StackBase;                                     
     PVOID StackLimit;                                    
     PVOID DeallocationStack;                            
     CONTEXT FiberContext;                                
     PVOID Wx86Tib;                                      
     struct _ACTIVATION_CONTEXT_STACK *ActivationContextStackPointer; 
     PVOID FlsData;                                       
     ULONG GuaranteedStackBytes;                         
     ULONG TebFlags;                                     
 } FIBER, *PFIBER;
 */

struct fiber
{
    fid_t fid;
    struct fiber_info fiber_data; //I will use the fiber_data struct instead of void *, for mantaining both fiber data and metrics data
    //struct exteption_registration_record * exteption_list;
    void * stack_base;
    void * stack_limit;
    //void * dealloction_stack;
    struct context fiber_context;
    pid_t parent_pid; //rename for better understanding
    struct activation_context fiber_activation_context; //I will use it for mantaining the entry point for the fiber and its related arguments
    long long * fls_data; //my fls will be an array of long long values, for coherence with my instance of the problem
    unsigned long guaranteed_stack_bytes; //my stack size
    //unsigned long teb_flags;
    struct hlist_node fnode; //for scanning my hashtable
};

//I want to make a unique function
inline struct process * get_process(tgid_t tgid);
inline struct thread * get_thread(pid_t pid, struct process * p);
inline struct fiber * get_fiber(fid_t fid, struct process * p);

fid_t do_convert_thread_to_fiber(id_t id);
fid_t do_create_fiber(id_t id, size_t stack_size, unsigned long entry_point, void * param);
fid_t do_switch_to_fiber(id_t id, fid_t next_fiber);
long do_fls_alloc(id_t id);
long long do_fls_get_value(id_t id, long index);
long do_fls_free(id_t id, long index);
long do_fls_set_value(id_t id, long index, long long value);