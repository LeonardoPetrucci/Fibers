#include <linux/types.h>
#include <linux/ptrace.h>

struct fiber
{
    void * fiber_data;
    void * exception_list; //unused
    void * stack_base;
    size_t stack_limit;
    void * deallocation_stack; //needed?
    struct pt_regs fiber_context;
    void * parent_thread;
    void * activation_context_stack_pointer; //my entry point
    void * fls_data //how to manage?
};

typedef pid_t fid_t;

int convert_thread_to_fiber(void);
int create_fiber(size_t stack_size, void (*entry_point)(void *), void *param);
int switch_to_fiber(void);
int fls_alloc(void);
int fls_get_value(void);
int fls_free(void);
int fls_set_value(void);