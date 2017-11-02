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
static struct dentry *	dfs_construct(struct file_system_type *,int flags,const char *dev_name,void *data);	//Constructor Function passed in struct file_system_type
static void 		dfs_destruct(struct super_block *sb);							//Destructor Fuction passed in struct file_system_type
static int  		dfs_fill_super(struct super_block *sb,void *data,int silent);				//Fills the struct superblock of this filesystem (Passed in mount_bdev)
static struct dentry *	dfs_iop_lookup(struct inode *,struct dentry *parent,unsigned int flagsMaybe);		//Converts a path name to struct inode
static int 		dfs_iop_mkdir(struct inode *,struct dentry *parent,umode_t mode);			//Makes a directory (Not supported yet)
static int 		dfs_iop_create(struct inode *,struct dentry* ,umode_t flags,bool);			//Creates a regular file
static ssize_t 		dfs_fop_read(struct file*,char __user *,size_t , loff_t *);				//Read the contents of a file...
static int 		dfs_iop_permission(struct inode *,int);							//return the permissions of the file (not supported)
int 			dfs_iop_getattr(struct vfsmount *mnt, struct dentry *, struct kstat *);
/// Mount folder Operations only...
static int		dfs_mount_dir_iop_iterate(struct file *flip,struct dir_context *ctx);			//iterate the contents of mount folder only (Unstable yet)




static int dfs_iop_permission(struct inode *nod,int checkThat){
	printk(KERN_INFO"Permission Runnin...");
	return 0;//nod->i_mode;

}

struct kmem_cache *dfs_dat_inode_pool;
int count=-1;
struct inode **inodes;			//simple array of inodes to save (in ram..)
char **		names;			//simple array with filenames to ascociate...
struct inode *mountFolder=NULL;

struct dfs_data{
	char name[12];
	char data[500];
};
static struct inode_operations dfs_iop={
	.lookup=	dfs_iop_lookup,
	.mkdir=		dfs_iop_mkdir,
	.create=	dfs_iop_create,
	.getattr=	dfs_iop_getattr,
};
/*
	Mount Folder file_operations struct .because the mount folder is a {FOLDER} we can not open it as regular file(read write etc...)
*/
static struct file_operations dfs_mount_dir_iop={
	.iterate=	dfs_mount_dir_iop_iterate, //Iterate through the elements of mount folder..

};
/*
	Per file file_operations struct . we support read only (remember is beta yet.. :) )
*/
static struct file_operations dfs_fop={
	.read=dfs_fop_read,
	.write=NULL,
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
	inodes	= (struct inode **)kzalloc(sizeof(struct inode *)*12,0);
	names	= (char **)kzalloc(sizeof(char *)*12,0);
//	dfs_dat_inode_pool=kmem_cache_create("dfs_dat_inode_pool",sizeof(struct dfs_dat_inode_struct),0,0,NULL);
	ret=register_filesystem(&dfs_info);
	if(unlikely(ret!=0)){
		printk(KERN_ERR"dfs cant register , fail...");
		return ret;
	}
	printk(KERN_INFO"[dfs_log]feel free to mount me , i am ready boss!");
	return 0;
}

void dfs_exit(void){
//	kmem_cache_destroy(dfs_dat_inode_pool);
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
static int              dfs_mount_dir_iop_iterate(struct file *flip,struct dir_context *ctx){
        printk(KERN_INFO"dfs_mount_dir_iop_iterate started for file %s",flip->f_path.dentry->d_name.name);
	int i;
	for(i=0;i<count;i++)
		dir_emit(ctx,names[i],15,i+5,DT_REG);
        return 0;

}

static int dfs_fill_super(struct super_block *sb,void *data,int silent){
	struct inode *root_inode;
	sb->s_maxbytes=DFS_MAXBYTES;
	root_inode=new_inode(sb);
	root_inode->i_ino=DFS_FIRSTINODE_INO;
	inode_init_owner(root_inode,NULL,S_IFDIR);
	root_inode->i_op=&dfs_iop;
	root_inode->i_fop=&dfs_mount_dir_iop;
	root_inode->i_sb=sb;
	root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
	root_inode->i_mode|=(MAY_ACCESS|MAY_READ|MAY_OPEN|MAY_WRITE|MAY_EXEC);
	sb->s_root=d_make_root(root_inode);
	mountFolder=root_inode;
	return 0;
}
static struct dentry *dfs_iop_lookup(struct inode *dir,struct dentry *query,unsigned int flags)
{
	printk(KERN_INFO" path to lookup %s",query->d_name.name);
	struct list_head *temp;
	struct dentry* tempDir;
	int i=0;
	if(!strcmp(query->d_name.name,"/") || !strcmp(query->d_name.name,".")){
		printk(KERN_INFO"Main Folder lookup %s",query->d_name.name);
                d_add(query,mountFolder);
		return NULL;
	}/*
	for(i=0;i<count;i++){
		if(!strcmp(names[i],query->d_name.name)){
			inode_init_owner(dir,inodes[i],flags);
			d_add(query,inodes[i]);
			break;
		}
	}*/
	return NULL;
}
static int dfs_iop_mkdir(struct inode *ptrToParent,struct dentry *parent,umode_t mode){
	return EOPNOTSUPP;
}
static int dfs_iop_create(struct inode *dir,struct dentry*childDentry,umode_t flags,bool iDontKnowThatSit){
	if(count>12)return -EOPNOTSUPP;
	printk(KERN_ERR"Creation of file %s  %d(%d left)",childDentry->d_name.name,count+1,(12-count+1));
	struct inode *inode;							//to create a new inode file...
	inode=new_inode(dir->i_sb);						//allocate a new inode with superblock same as current super block
	inode->i_private=(void  *)kzalloc(sizeof(struct dfs_data),0);
	inode->i_mode|=(MAY_ACCESS|MAY_READ|MAY_OPEN|MAY_WRITE|MAY_EXEC);	//inital permissions...
	inode->i_op=&dfs_iop;
	inode->i_fop=&dfs_fop;							//inode operations struct pass...
	inode_init_owner(inode,dir,flags);					//initalize ownership
	d_add(childDentry,inode);						//add inode to childDentry...
	count+=1;								//add dentries +1

	names[count]=childDentry->d_name.name;					//save fileName
	inodes[count]=inode;							//save inode..
	return 0;								//everything fine...
}
char testString[] = "Hello world";
static ssize_t dfs_fop_read(struct file*file,char __user *buff,size_t size, loff_t *loff){
	if(size>12)size=12;
	if(unlikely(copy_to_user(buff,testString,size))){
		printk(KERN_ERR"Error copy user space buffer");
		return 0;
	}
	*loff+=size;
	return size;

}
int  dfs_iop_getattr(struct vfsmount *mnt, struct dentry *curr, struct kstat *stat){
	stat->ino=10;
	stat->mode=(MAY_EXEC);
	stat->size=210;
	return 0;

}
