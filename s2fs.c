#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <asm/atomic.h>
#include <asm/current.h>
#include <linux/slab.h> 

#define MAGIC 0x12345678


static struct super_operations s2fs_s_ops = {
	.statfs		= simple_statfs,
	.drop_inode	= generic_delete_inode,
};
static int s2fs_open(struct inode *inode, struct file *filp)
{
	return 0;
}
static ssize_t s2fs_write_file(struct file *filp, const char *buf,size_t count, loff_t *offset)
{	
	return 0;
}
static ssize_t s2fs_read_file(struct file *filp, char *buf, size_t count, loff_t *offset)
{
	static char msg[15] = {0};
	int ret;
	strncpy(msg, "Hello World!\n", sizeof(msg));

	int msg_size;
	msg_size = strlen(msg); 
	ret = copy_to_user(buf, msg, msg_size);
	if (ret==0)
		return msg_size;
	else
		return -EFAULT;     
}

static struct file_operations s2fs_fops = {
	.open	= s2fs_open,
	.write  = s2fs_write_file,
	.read 	= s2fs_read_file,
};


static struct inode *s2fs_make_inode(struct super_block *sb, int mode)
{
	struct inode *inode = new_inode(sb);
	if (inode)
	{
		inode->i_ino = get_next_ino();	
		inode->i_mode = mode;
		inode->i_atime = current_time(inode);
		inode->i_mtime = current_time(inode);
		inode->i_ctime = current_time(inode);
		inode->i_blocks = 0;
	}
	return inode;
} 
static struct dentry *s2fs_create_file(struct super_block *sb, struct dentry *dir, const char *file_name)
{
	struct dentry *dentry;
	struct inode *inode;

	dentry = d_alloc_name(dir, file_name);
	inode = s2fs_make_inode(sb, S_IFREG|0644);
	if (dentry != NULL)
	{
		if (inode != NULL)
		{
			inode->i_fop = &s2fs_fops;
			inode->i_private = 0;
			d_add(dentry, inode);

			return dentry;
		}
		else
		{
			dput(dentry);
			return 0;
		}
	}
	else
		return 0;
}
static struct dentry *s2fs_create_dir(struct super_block *sb, struct dentry *parent, const char *dir_name)
{
	struct dentry *dentry;
	struct inode *inode;

	dentry = d_alloc_name(parent, dir_name);
	inode = s2fs_make_inode(sb, S_IFDIR|0755);

	if (dentry != NULL)
	{
		if (inode!=NULL)
		{
			inode->i_op = &simple_dir_inode_operations;
			inode->i_fop = &simple_dir_operations;

			d_add(dentry, inode);

			return dentry;
		}
		else
		{
			dput(dentry);
			return 0;
		}
	}
	else
		return 0;
}

static void s2fs_create_foobar(struct super_block *sb, struct dentry *root)
{
	struct dentry *dir;
	dir = s2fs_create_dir(sb, root, "foo");
	if (dir != NULL)
		s2fs_create_file(sb, dir, "bar");
}

static int s2fs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct inode *root;
	struct dentry *root_dentry;

	sb->s_magic = MAGIC;
	sb->s_op = &s2fs_s_ops;

	root = s2fs_make_inode(sb, S_IFDIR | 0755);
	inode_init_owner(root, NULL, S_IFDIR|0755);

	root->i_op = &simple_dir_inode_operations;
	root->i_fop = &simple_dir_operations;

	set_nlink(root, 2);
	root_dentry = d_make_root(root);

	s2fs_create_foobar(sb, root_dentry);
	sb->s_root = root_dentry;	

	return 0;
}

static struct dentry *s2fs_mount(struct file_system_type *fs_type, int flags, const char *dev, void *data)
{
	struct dentry *const entry = mount_nodev(fs_type, flags, data, s2fs_fill_super);

	return entry;
}

static struct file_system_type s2fs_type = {
	.owner 		= THIS_MODULE,
	.name		= "s2fs",
	.mount		= s2fs_mount,
	.kill_sb	= kill_litter_super,

};

static int __init s2fs_init(void)
{
	int ret = 0;
	ret = register_filesystem(&s2fs_type);
	printk("s2fs registered\n");

	return ret;
}

static void __exit s2fs_exit(void)
{
	unregister_filesystem(&s2fs_type);
	printk("module s2fs removed\n");
}

module_init(s2fs_init)
	module_exit(s2fs_exit)
	MODULE_LICENSE("GPL");