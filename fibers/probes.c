#include "include/proc.h"

proc_kernel_syms_t proc_kernel_syms;
spinlock_t check_nents = __SPIN_LOCK_UNLOCKED(check_nents);
int nents = 0;

struct tgid_dir_data {
				struct file *file;
				struct dir_context *ctx;
};
struct tgid_lookup_data{
				struct dentry *dentry;
				struct inode *inode;
};

struct kretprobe proc_readdir_krp;
struct kretprobe proc_lookup_krp;


struct file_operations file_ops = {
				.read  = generic_read_dir,
				.iterate_shared = fiber_readdir,
				.llseek  = generic_file_llseek,
};
struct inode_operations inode_ops = {
				.lookup = fiber_lookup,
};

const struct pid_entry additional[] = {
				DIR("fibers", S_IRUGO|S_IXUGO, inode_ops, file_ops),
};
/* 
proc_pident_readdir_t readdir;
proc_pident_lookup_t lookup;
pid_getattr_t getattr;
proc_setattr_t setattr;
*/



int register_fiber_kretprobe(void)
{
	proc_kernel_syms.proc_pident_readdir = kallsyms_lookup_name("proc_pident_readdir");
	proc_kernel_syms.proc_pident_lookup = kallsyms_lookup_name("proc_pident_lookup");
	proc_kernel_syms.proc_setattr = kallsyms_lookup_name("proc_setattr");
	proc_kernel_syms.pid_getattr = kallsyms_lookup_name("pid_getattr");

	//inode_ops.getattr = proc_kernel_syms.pid_getattr; //WHY YOU DON'T WORK?????????????
	inode_ops.setattr = proc_kernel_syms.proc_setattr;

	proc_readdir_krp.entry_handler = entry_handler_readdir;
	proc_readdir_krp.handler = handler_readdir;
	proc_readdir_krp.data_size = sizeof(struct tgid_dir_data);
	proc_readdir_krp.kp.symbol_name = "proc_tgid_base_readdir";

	proc_lookup_krp.entry_handler = entry_handler_lookup;
	proc_lookup_krp.handler = handler_lookup;
	proc_lookup_krp.data_size = sizeof(struct tgid_lookup_data);
	proc_lookup_krp.kp.symbol_name = "proc_tgid_base_lookup";

	register_kretprobe(&proc_readdir_krp);
	register_kretprobe(&proc_lookup_krp);

	return 0;
}

int unregister_fiber_kretprobe(void)
{
				unregister_kretprobe(&proc_readdir_krp);
				unregister_kretprobe(&proc_lookup_krp);
				return 0;
}

int entry_handler_lookup(struct kretprobe_instance *k, struct pt_regs *regs)
{

				struct inode *inode = (struct inode *) regs->di;
				struct dentry *dentry = (struct dentry *) regs->si;
				struct tgid_lookup_data data;
				data.inode = inode;
				data.dentry = dentry;
				memcpy(k->data, &data, sizeof(struct tgid_lookup_data));

				return 0;
}


int handler_lookup(struct kretprobe_instance *k, struct pt_regs *regs)
{

	struct tgid_lookup_data *data = (struct tgid_lookup_data *)(k->data);
	struct process *p;
	//unsigned long flags;
	unsigned int pos;
	struct task_struct * task = get_pid_task(proc_pid(data->inode), PIDTYPE_PID);

	p = get_process(task->tgid);
	if (p == NULL)
		return 0;



	if (nents == 0)
		return 0;

	pos = nents;
	proc_kernel_syms.proc_pident_lookup(data->inode, data->dentry, additional - (pos - 2), pos - 1);

	return 0;
}


int entry_handler_readdir(struct kretprobe_instance *k, struct pt_regs *regs)
{

	struct file *file = (struct file *) regs->di;
	struct dir_context * ctx = (struct dir_context *) regs->si;
	struct tgid_dir_data data;
	data.file = file;
	data.ctx = ctx;
	memcpy(k->data, &data, sizeof(struct tgid_dir_data));
	return 0;
}


int handler_readdir(struct kretprobe_instance *k, struct pt_regs *regs)
{

	struct tgid_dir_data *data = (struct tgid_dir_data *)(k->data);
	struct process *p;
	unsigned long flags;
	unsigned int pos;
	struct task_struct * task = get_pid_task(proc_pid(file_inode(data->file)), PIDTYPE_PID);

	if (nents == 0) {
					spin_lock_irqsave(&check_nents, flags);
					if (nents == 0)
									nents = data->ctx->pos;
					spin_unlock_irqrestore(&check_nents, flags);
	}

	p = get_process(task->tgid);
	if (p == NULL)
					return 0;

	pos = nents;
	proc_kernel_syms.proc_pident_readdir(data->file, data->ctx, additional - (pos - 2), pos - 1);
	return 0;
}
