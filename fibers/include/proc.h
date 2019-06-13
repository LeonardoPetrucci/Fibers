#ifndef FIBERS_PROC
#define FIBERS_PROC

#include <linux/pid.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/kprobes.h>

#include "fiber.h"

/***************************************************************************/
/*                         From fs/proc/internal.h                         */
/***************************************************************************/

union proc_op {
	int (*proc_get_link)(struct dentry *, struct path *);
	int (*proc_show)(struct seq_file *m, struct pid_namespace *ns, struct pid *pid, struct task_struct *task);
};

struct proc_inode {
	struct pid *pid;
	unsigned int fd;
	union proc_op op;
	struct proc_dir_entry *pde;
	struct ctl_table_header *sysctl;
	struct ctl_table *sysctl_entry;
	struct hlist_node sysctl_inodes;
	const struct proc_ns_operations *ns_ops;
	struct inode vfs_inode;
};

static inline struct proc_inode *PROC_I(const struct inode *inode)
{
    return container_of(inode, struct proc_inode, vfs_inode);
}

static inline struct pid *proc_pid(struct inode *inode)
{
    return PROC_I(inode)->pid;
}

static inline struct task_struct *get_proc_task(struct inode *inode)
{
	return get_pid_task(proc_pid(inode), PIDTYPE_PID);
}

/***************************************************************************/
/*                           From fs/proc/base.c                           */
/***************************************************************************/

struct pid_entry {
	const char *name;
	unsigned int len;
	umode_t mode;
	const struct inode_operations *iop;
	const struct file_operations *fop;
	union proc_op op;
};

#define NOD(NAME, MODE, IOP, FOP, OP) {     							    \
	.name = (NAME),         								    			\
	.len  = sizeof(NAME) - 1,         						    			\
	.mode = MODE,         													\
	.iop  = IOP,          													\
	.fop  = FOP,          													\
	.op   = OP,         													\
}

#define DIR(NAME, MODE, iops, fops) 										\
	NOD(NAME, (S_IFDIR|(MODE)), &iops, &fops, {} )

#define REG(NAME, MODE, fops)												\
	NOD(NAME, (S_IFREG|(MODE)), NULL, &fops, {} )



/***************************************************************************/
/*                  Here the fibers-specific part begins                   */
/***************************************************************************/

struct tgid_dir_data {
	struct file *file;
	struct dir_context *ctx;
};

struct tgid_lookup_data{
	struct dentry *dentry;
	struct inode *inode;
};

ssize_t fiber_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
struct dentry* fiber_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags);
int fiber_readdir(struct file *file, struct dir_context *ctx);

//move in types.h
typedef struct proc_kernel_syms_s {
	int (*proc_pident_readdir)(struct file *file, struct dir_context *ctx, const struct pid_entry *ents, unsigned int nents);
	struct dentry * (*proc_pident_lookup)(struct inode *, struct dentry *, const struct pid_entry *ents, unsigned int nents);
	int (*pid_getattr)(struct vfsmount *mnt, struct dentry *dentry, struct kstat *stat);
	int (*proc_setattr)(struct dentry *dentry, struct iattr *attr);
}proc_kernel_syms_t;

extern proc_kernel_syms_t proc_kernel_syms;

int register_fiber_kretprobe(void);
int unregister_fiber_kretprobe(void);

int entry_handler_readdir(struct kretprobe_instance *k, struct pt_regs *regs);
int handler_readdir(struct kretprobe_instance *k, struct pt_regs *regs);
int handler_lookup(struct kretprobe_instance *k, struct pt_regs *regs);
int entry_handler_lookup(struct kretprobe_instance *k, struct pt_regs *regs);

#endif
