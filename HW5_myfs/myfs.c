#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pagemap.h>      /* PAGE_CACHE_SIZE */
#include <linux/fs.h>           /* This is where libfs stuff is declared */
#include <asm/atomic.h>
#include <asm/uaccess.h>        /* copy_to_user */
 
 
MODULE_LICENSE("GPL");
 
#define MYFS_MAGIC 0x20210607

static struct Input_AB{ // add
	atomic_t *a; // the pointer of a.
	atomic_t *b; // the pointer of b.
} input_ab; 
 
static struct inode *myfs_make_inode(struct super_block *sb, int mode)
{
        struct inode *ret = new_inode(sb);
 
        if (ret) {
                ret->i_mode = mode;
                ret->i_uid.val = 0;
                ret->i_gid.val = 0;
                ret->i_blocks = 0;
                ret->i_atime = current_kernel_time();
                ret->i_mtime = current_kernel_time();
                ret->i_ctime = current_kernel_time();
        }
        return ret;
}
 
static int myfs_open(struct inode *inode, struct file *filp)
{
        filp->private_data = inode->i_private;
        return 0;
}
 
#define TMPSIZE 20
 
static ssize_t myfs_read_file(struct file *filp, char *buf,
                size_t count, loff_t *offset)
{
        atomic_t *counter = (atomic_t *) filp->private_data;
        int v=0, len;
        char tmp[TMPSIZE];
	const char *file_parent_name = filp -> f_path.dentry -> d_parent -> d_name.name;
        const char *file_name = filp -> f_path.dentry -> d_name.name;
        int int_a; // for output file.
        atomic_t input_a; // for output file. 
 
        printk("file_parent_name=%s\n", file_parent_name); // will print a, b, add, sub.
        // 
        if(strcmp(file_parent_name, "input") == 0){ // the name of the parent file is input. 
                v = atomic_read(counter);
                if (*offset > 0){
                        printk("*offset>0\n");
                        v -= 1;  /* the value returned when offset was zero */
                }
        }else if(strcmp(file_parent_name, "output") == 0){ // the name of the parent file is output. 
                struct Input_AB *ab = (struct Input_AB *)filp->private_data; //atomic_read(counter);
                int_a = atomic_read(ab->a);
                
                atomic_set(&input_a, int_a); 
                if(strcmp(file_name, "add") == 0){ // the name of the file is add. 
                        atomic_add(atomic_read(ab->b), &input_a); // a + b
                }else if(strcmp(file_name, "sub") == 0){ // the name of the file is sub. 
                        atomic_sub(atomic_read(ab->b), &input_a); // a - b
                }
                v = atomic_read(&input_a); // answer is in input_a, and will not change the number in "input" file. 
        }else{
                printk("It is not in input/output file.");
        }

        printk("v=%d\n", v);

        len = snprintf(tmp, TMPSIZE, "%d\n", v);
        if (*offset > len)
                return 0;
        if (count > len - *offset)
                count = len - *offset;
 
        if (copy_to_user(buf, tmp + *offset, count))
                return -EFAULT;
        *offset += count;
        return count;
}
 
static ssize_t myfs_write_file(struct file *filp, const char *buf,
                size_t count, loff_t *offset)
{
        atomic_t *counter = (atomic_t *) filp->private_data;
        char tmp[TMPSIZE];
 
        if (*offset != 0)
                return -EINVAL;
 
        if (count >= TMPSIZE)
                return -EINVAL;
        memset(tmp, 0, TMPSIZE);
        if (copy_from_user(tmp, buf, count))
                return -EFAULT;
 
        atomic_set(counter, simple_strtol(tmp, NULL, 10));
        return count;
}
 
 
static struct file_operations myfs_file_ops = {
        .open   = myfs_open,
        .read   = myfs_read_file,
        .write  = myfs_write_file,
};
 
static struct dentry *myfs_create_file (struct super_block *sb,
                struct dentry *dir, const char *name,
                atomic_t *counter)
{
	//atomic_t *counter = *counter_addr;

        struct dentry *dentry;
        struct inode *inode;
        struct qstr qname;
 
        qname.name = name;
        qname.len = strlen (name);
        qname.hash = full_name_hash(name, qname.len);
 
        dentry = d_alloc(dir, &qname);
        if (! dentry)
                goto out;
        inode = myfs_make_inode(sb, S_IFREG | 0644);
        if (! inode)
                goto out_dput;
        inode->i_fop = &myfs_file_ops;
        inode->i_private = counter;
 
        d_add(dentry, inode);
        return dentry;
 
  out_dput:
        dput(dentry);
  out:
        return 0;
}

static struct dentry *myfs_create_Input_AB_file (struct super_block *sb,
                struct dentry *dir, const char *name,
                struct Input_AB *counter)
{ // add
	//atomic_t *counter = *counter_addr;

        struct dentry *dentry;
        struct inode *inode;
        struct qstr qname;
 
        qname.name = name;
        qname.len = strlen (name);
        qname.hash = full_name_hash(name, qname.len);
 
        dentry = d_alloc(dir, &qname);
        if (! dentry)
                goto out;
        inode = myfs_make_inode(sb, S_IFREG | 0644);
        if (! inode)
                goto out_dput;
        inode->i_fop = &myfs_file_ops;
        inode->i_private = counter;
 
        d_add(dentry, inode);
        return dentry;
 
  out_dput:
        dput(dentry);
  out:
        return 0;
}

static struct dentry *myfs_create_dir (struct super_block *sb,
                struct dentry *parent, const char *name)
{
        struct dentry *dentry;
        struct inode *inode;
        struct qstr qname;
 
        qname.name = name;
        qname.len = strlen (name);
        qname.hash = full_name_hash(name, qname.len);
        dentry = d_alloc(parent, &qname);
        if (! dentry)
                goto out;
 
        inode = myfs_make_inode(sb, S_IFDIR | 0644);
        if (! inode)
                goto out_dput;
        inode->i_op = &simple_dir_inode_operations;
        inode->i_fop = &simple_dir_operations;
 
        d_add(dentry, inode);
        return dentry;
 
  out_dput:
        dput(dentry);
  out:
        return 0;
}
 
//static atomic_t counter, subcounter;
static atomic_t a, b; // a, b are in input direction. 

static void myfs_create_files (struct super_block *sb, struct dentry *root)
{ // change 
        struct dentry *input;
	struct dentry *output;
 	// atomic_t *a_addr = &a;
	// atomic_t *b_addr = &b;
        // atomic_t *input_ab_addr = &input_ab;
	//atomic_t *b_addr = &b;

        atomic_set(&a, 0);
	atomic_set(&b, 0);
	
        //myfs_create_file(sb, root, "a", &a);
 
        //atomic_set(&subcounter, 0);

        input = myfs_create_dir(sb, root, "input");
        if (input){
                myfs_create_file(sb, input, "a", &a);
		myfs_create_file(sb, input, "b", &b);
	}

	input_ab.a = &a;
	input_ab.b = &b;
	output = myfs_create_dir(sb, root, "output");
	if(output){
		myfs_create_Input_AB_file(sb, output, "add", &input_ab);
		myfs_create_Input_AB_file(sb, output, "sub", &input_ab);
	}
}
 
 
 
static struct super_operations myfs_s_ops = {
        .statfs         = simple_statfs,
        .drop_inode     = generic_delete_inode,
};
 
static int myfs_fill_super (struct super_block *sb, void *data, int silent)
{
        struct inode *root;
        struct dentry *root_dentry;
 
        sb->s_blocksize = PAGE_SIZE;
        sb->s_blocksize_bits = PAGE_SHIFT;
        sb->s_magic = MYFS_MAGIC;
        sb->s_op = &myfs_s_ops;
 
        root = myfs_make_inode (sb, S_IFDIR | 0755);
        if (! root)
                goto out;
        root->i_op = &simple_dir_inode_operations;
        root->i_fop = &simple_dir_operations;
 
        root_dentry = d_make_root(root);
        if (! root_dentry)
                goto out_iput;
        sb->s_root = root_dentry;
 
        myfs_create_files (sb, root_dentry);
        return 0;
 
  out_iput:
        iput(root);
  out:
        return -ENOMEM;
}
 
static struct dentry *myfs_get_super(struct file_system_type *fst,
                int flags, const char *devname, void *data)
{
        return mount_nodev(fst, flags, data,myfs_fill_super);
}
 
static struct file_system_type myfs_type = {
        .owner          = THIS_MODULE,
        .name           = "myfs",
        .mount          = myfs_get_super,
        .kill_sb        = kill_litter_super,
};
 
static int __init myfs_init(void)
{
        return register_filesystem(&myfs_type);
}
 
static void __exit myfs_exit(void)
{
        unregister_filesystem(&myfs_type);
}
 
module_init(myfs_init);
module_exit(myfs_exit);

