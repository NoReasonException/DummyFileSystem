#include <linux/fs.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include <linux/list.h>

#define DFS_MAXBYTES 1024
#define DFS_FIRSTINODE_INO 0
static struct dentry *dfs_construct(struct file_system_type *,int flags,const char *dev_name,void *data);
static void dfs_destruct(struct super_block *sb);
static int  dfs_fill_super(struct super_block *sb,void *data,int silent);
static struct dentry *dfs_iop_lookup(struct inode *,struct dentry *parent,unsigned int flagsMaybe);
static int dfs_iop_mkdir(struct inode *,struct dentry *parent,umode_t mode);
static int dfs_iop_create(struct inode *,struct dentry* ,umode_t flags,bool);
struct kmem_cache *dfs_dat_inode_pool;
struct dfs_dat_inode_struct{
	char data[4096];
};
static struct dentry *test=NULL;

static struct inode_operations dfs_iop={
	.lookup=dfs_iop_lookup,
	.mkdir=dfs_iop_mkdir,
	.create=dfs_iop_create,
};
static struct file_system_type dfs_info={
	.owner=THIS_MODULE,
	.name="dfs",
	.mount=dfs_construct,
	.kill_sb=dfs_destruct,
	.fs_flags=FS_REQUIRES_DEV,

};
int dfs_init(void){
	int ret;
	dfs_dat_inode_pool=kmem_cache_create("dfs_dat_inode_pool",sizeof(struct dfs_dat_inode_struct),0,0,NULL);
	ret=register_filesystem(&dfs_info);
	if(unlikely(ret!=0)){
		printk(KERN_ERR"dfs cant register , fail...");
		return ret;
	}
	printk(KERN_INFO"[dfs_log]feel free to mount me , i am ready boss!");
	return 0;
}

void dfs_exit(void){
	kmem_cache_destroy(dfs_dat_inode_pool);
	unregister_filesystem(&dfs_info);
}
module_init(dfs_init);
module_exit(dfs_exit);
static void dfs_destruct (struct super_block *sb){
	 printk(KERN_INFO"[dfs_log] i hope to i pleased you , master , i am unmounted...");
        kill_block_super(sb);
        return;
}
static struct dentry *dfs_construct(struct file_system_type *dfs_inf,int flags,const char *dev_name,void *data){
	struct dentry *root;
	root=mount_bdev(dfs_inf,flags,dev_name,data,dfs_fill_super);
	if(unlikely(root==NULL)){
		printk(KERN_ERR"[dfs_log]OMG,mount_bdev returned NULL!its a joke? propably not! check for any bug in dfs_fill_super() maybe?");
	}
	return root;
}
static int dfs_fill_super(struct super_block *sb,void *data,int silent){
	struct inode *root_inode;
	sb->s_maxbytes=DFS_MAXBYTES;
	root_inode=new_inode(sb);
	root_inode->i_ino=DFS_FIRSTINODE_INO;
	inode_init_owner(root_inode,NULL,S_IFDIR);
	root_inode->i_op=&dfs_iop;
	root_inode->i_sb=sb;
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
	root_inode->i_mode|=(MAY_ACCESS|MAY_READ|MAY_OPEN|MAY_WRITE|MAY_EXEC);
	sb->s_root=d_make_root(root_inode);

	return 0;
}
static struct dentry *dfs_iop_lookup(struct inode *query,struct dentry *parent,unsigned int flagsMaybe)
{
	struct list_head *temp;
	struct dentry* tempDir;
	list_for_each(temp,&parent->d_subdirs){
		tempDir=container_of(temp,struct dentry,d_subdirs);
		if(tempDir==query){
			return tempDir;
		}
	}
	return NULL;

}
static int dfs_iop_mkdir(struct inode *ptr,struct dentry *parent,umode_t mode){
	test=parent;
	ptr->i_op=&dfs_iop;
	d_add(parent,ptr);
	return 0;
}
static int dfs_iop_create(struct inode *curr,struct dentry*parent,umode_t flags,bool iDontKnowThatSit){
	//curr->i_mode=(S_IFREG|MAY_READ|MAY_EXEC|MAY_ACCESS);

	if(S_ISDIR(flags)){
		printk(KERN_ERR"Folder Created");
		return dfs_iop_mkdir(curr,parent,flags);
	}
	else{
		printk(KERN_ERR"File Created");
	}
	d_add(parent,curr);
	return 0;
}
