#define KERNEL

#include <linux/ptrace.h>
#include <linux/hashtable.h>

#include "types.h"

#define NAME_LENGHT 256
#define HASHTABLE_BITS 10

struct process
{
    tgid_t tgid;
    fid_t last_fid;
    DECLARE_HASHTABLE(thread_hash,HASHTABLE_BITS);
    DECLARE_HASHTABLE(fiber_hash,HASHTABLE_BITS);
    struct hlist_node pnode;
};

struct thread
{
    pid_t pid;
    fid_t active_fid;
    struct hlist_node tnode;
};

struct fiber_data
{
    char name[NAME_LENGHT];
    pid_t parent_pid;
    atomic_t successful_activations;
    atomic_t failed_activations;
    unsigned long running_time;
    unsigned long last_time_active;
};

struct activation_context
{
    void * stack_base;
    size_t stack_limit;
    struct pt_regs cpu_context;
    struct fpu fpu_context;
    void * entry_point;
};

struct fiber
{
    atomic_t state;
    fid_t fid;
    struct fiber_data data; //my metrics struct 
    void * exception_list; //unused
    struct activation_context context;
    long long * fls;
    struct hlist_node fnode;
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