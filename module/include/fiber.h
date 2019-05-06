#include <linux/types.h>
#include <linux/ptrace.h>

typedef struct fiber{
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

int convert_thread_to_fiber();
int create_fiber();
int switch_to_fiber();
int fls_alloc();
int fls_get_value();
int fls_free();
int fls_set_value();