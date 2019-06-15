#define KERNEL

#include "include/proc.h"

struct file_operations fibers_proc_fops = {
	.read = fiber_read,
};

struct dentry* fiber_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
	struct dentry * ret;
	struct task_struct * task = get_proc_task(dir);
	struct thread_group * tg;
	struct fiber * f;
	unsigned long nents;
	struct pid_entry * pid_fibers;

	task = get_proc_task(dir);
	if(!task)
	{
		return -ESRCH;
	}
	tg = get_group(task->tgid);

	if(!tg)
	{
		return 0;
	}
	nents = atomic_read(&tg->fid_count);
	pid_fibers = kmalloc(nents * sizeof(struct pid_entry), GFP_KERNEL);
	memset(pid_fibers, 0, nents * sizeof(struct pid_entry));

	int bkt, i = 0;
	hash_for_each_rcu(tg->fibers, bkt, f, fnode)
	{
		pid_fibers[i].name = f->info.name;
		pid_fibers[i].len = strlen(pid_fibers[i].name);
		pid_fibers[i].mode = (S_IFREG|(S_IRUGO));
		pid_fibers[i].iop = NULL;
		pid_fibers[i].fop = &fibers_proc_fops;
		
		i++;
	}

	ret = proc_kernel_syms.proc_pident_lookup(dir, dentry, pid_fibers, nents);
	kfree(pid_fibers);
	return ret;

}

int fiber_readdir(struct file *file, struct dir_context *ctx)
{
	int ret;
	struct task_struct * task = get_proc_task(file_inode(file));
	struct thread_group * tg;
	
	struct fiber * f;
	unsigned long nents;
	struct pid_entry * pid_fibers;

	if(!task)
	{
		return -ESRCH;
	}
	tg = get_group(task->tgid);

	if(!tg)
	{
		return 0;
	}
	nents = atomic_read(&tg->fid_count);
	pid_fibers = kmalloc(nents * sizeof(struct pid_entry), GFP_KERNEL);
	memset(pid_fibers, 0, nents * sizeof(struct pid_entry));

	int bkt, i = 0;
	hash_for_each_rcu(tg->fibers, bkt, f, fnode)
	{	
		pid_fibers[i].name = f->info.name;
		pid_fibers[i].len = strlen(pid_fibers[i].name);
		pid_fibers[i].mode = (S_IFREG|(S_IRUGO));
		pid_fibers[i].iop = NULL;
		pid_fibers[i].fop = &fibers_proc_fops;
		
		i++;
	}

	ret = proc_kernel_syms.proc_pident_readdir(file, ctx, pid_fibers, nents);
	kfree(pid_fibers);
	return ret;
}

ssize_t fiber_read(struct file *filp, char __user *buf, size_t buf_size, loff_t *offset)
{
	char fiber_data[INFO_SIZE];
	int data_lenght, read_bytes;

	struct task_struct * task;
	pid_t tgid;

	unsigned long read_fid = 0;

	struct thread_group * tg;
	struct fiber * f;

	task = get_proc_task(file_inode(filp));
	tgid = task_tgid_nr(task);

	tg = get_group(tgid);
	if(!tg)
	{
		return -ESRCH;
	}
	kstrtoul(filp->f_path.dentry->d_name.name, DECIMAL, &read_fid);

	f = get_fiber(read_fid, tg);
	if(!f)
	{
		return -ESRCH;
	}
	snprintf(fiber_data, INFO_SIZE,
		"Name (Fiber ID): %s\n"\
		"Created by Thread: %d\n"\
		"Currently used by Thread: %d\n"\
		"Entry point: %x\n"
		"Parameters: %p\n"
		"Activations: %d success, %d failed\n"\
		"Last activation time: %lu\n"\
		"Total running time: %lu\n\n",
		f->info.name,
		f->info.parent,
		atomic_read(&f->info.state),
		f->info.entry_point,
		f->info.param,
		f->info.successful_activations,
		atomic_read(&f->info.failed_activations),
		f->info.last_activation_time,
		f->info.total_running_time);

	data_lenght = strnlen(fiber_data, INFO_SIZE);
	if(*offset >= data_lenght)
	{
		return 0;
	}
	
	if(buf_size < (data_lenght-*offset-1))
	{
		read_bytes = buf_size;
	}
	else
	{
		read_bytes = data_lenght-(*offset)-1;
	}

	if(copy_to_user(buf, fiber_data+(*offset), read_bytes))
	{
		return -1;
	}

	*offset += read_bytes;


	return read_bytes;
}