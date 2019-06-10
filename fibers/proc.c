#include "include/proc.h"

struct file_operations fiber_fops = {
				.read = fiber_read,
};

struct dentry* fiber_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
	struct dentry *ret;
	struct task_struct * p_task = get_proc_task(dir);
	struct process * p;
	struct fiber *f;
	unsigned long nents;
	struct pid_entry * pid_fibers;

	if(p_task == NULL)
	{
		return -1;
	}
	p = get_process(p_task->tgid);

	if(p == NULL)
	{
		return 0;
	}
	nents = p->last_fid;
	pid_fibers = kmalloc(nents * sizeof(struct pid_entry), GFP_KERNEL);
	memset(pid_fibers, 0, nents * sizeof(struct pid_entry));

	int bkt, i = 0;
	hash_for_each_rcu(p->fiber_hash, bkt, f, fnode)
	{
		pid_fibers[i].name = f->info.name;
		pid_fibers[i].len = strlen(pid_fibers[i].name);
		pid_fibers[i].mode = (S_IFREG|(S_IRUGO));
		pid_fibers[i].iop = NULL;
		pid_fibers[i].fop = &fiber_fops;

		i++;
	}

	ret = proc_kernel_syms.proc_pident_lookup(dir, dentry, pid_fibers, nents);
	kfree(pid_fibers);
	return ret;

}

int fiber_readdir(struct file *file, struct dir_context *ctx)
{
	int ret;
	struct task_struct * p_task = get_proc_task(file_inode(file));
	struct process * p;
	int bkt, i = 0;
	struct fiber * f;
	unsigned long nents;
	struct pid_entry * pid_fibers;

	if(p_task == NULL)
	{
		return -1;
	}
	p = get_process(p_task->tgid);

	if(p == NULL)
	{
		return 0;
	}
	nents = p->last_fid;
	pid_fibers = kmalloc(nents * sizeof(struct pid_entry), GFP_KERNEL);
	memset(pid_fibers, 0, nents * sizeof(struct pid_entry));

	hash_for_each_rcu(p->fiber_hash, bkt, f, fnode)
	{	
		pid_fibers[i].name = f->info.name;
		pid_fibers[i].len = strlen(pid_fibers[i].name);
		pid_fibers[i].mode = (S_IFREG|(S_IRUGO));
		pid_fibers[i].iop = NULL;
		pid_fibers[i].fop = &fiber_fops;

		i++;
	}

	ret = proc_kernel_syms.proc_pident_readdir(file, ctx, pid_fibers, nents);
	kfree(pid_fibers);
	return ret;
}


ssize_t fiber_read(struct file *filp, char __user *buf, size_t buf_size, loff_t *offset)
{
	//TODO
	return 0;
}