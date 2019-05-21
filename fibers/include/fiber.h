#ifndef KERNEL
#define KERNEL
#endif

#include <linux/ptrace.h>
#include <linux/hashtable.h>
#include <linux/spinlock_types.h>
#include <linux/bitmap.h>

#include "types.h"


#define H_SIZE 9

struct process
{
    tgid_t tgid;
    fid_t last_fid;
    DECLARE_HASHTABLE(thread_hash, H_SIZE);
    DECLARE_HASHTABLE(fiber_hash, H_SIZE);

    struct hlist_node pnode;
};

struct thread
{
    pid_t pid;
    fid_t current_fid;

    struct hlist_node tnode;
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
    pid_t reference_pid;
    spinlock_t flock;
    unsigned long flock_flags;
    //struct exteption_registration_record * exteption_list;
    void * stack_base;
    unsigned long stack_limit;
    unsigned long guaranteed_stack_bytes; //my stack size
    //void * dealloction_stack;
    fiber_context_t context;
    fls_t fls;
    fiber_info_t info;
    
    struct hlist_node fnode; //for scanning my hashtable
};

//I want to make a unique function
inline struct process * get_process(tgid_t tgid);
inline struct thread * get_thread(pid_t pid, struct process * p);
inline struct fiber * get_fiber(fid_t fid, struct process * p);

fid_t do_convert_thread_to_fiber(id_t id);
fid_t do_create_fiber(id_t id, void * stack_base, size_t stack_size, unsigned long entry_point, void * param);
fid_t do_switch_to_fiber(id_t id, fid_t next_fiber);
long do_fls_alloc(id_t id);
long long do_fls_get_value(id_t id, long index);
long do_fls_free(id_t id, long index);
long do_fls_set_value(id_t id, long index, long long value);