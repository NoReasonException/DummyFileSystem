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
static struct dentry *	dfs_construct		(struct file_system_type *,int flags,const char *dev_name,void *data);	//Constructor Function passed in struct file_system_type
static void 		dfs_destruct		(struct super_block *sb);						//Destructor Fuction passed in struct file_system_type
static int  		dfs_fill_super		(struct super_block *sb,void *data,int silent);				//Fills the struct superblock of this filesystem (Passed in mount_bdev)
static struct dentry *	dfs_iop_lookup		(struct inode *,struct dentry *parent,unsigned int flagsMaybe);		//Converts a path name to struct inode
static int 		dfs_iop_mkdir		(struct inode *,struct dentry *parent,umode_t mode);			//Makes a directory (Not supported yet)
static int 		dfs_iop_create		(struct inode *,struct dentry* ,umode_t flags,bool);			//Creates a regular file
static ssize_t 		dfs_fop_read		(struct file*,char __user *,size_t , loff_t *);				//Read the contents of a file...
static int 		dfs_iop_permission	(struct inode *,int);							//return the permissions of the file (not supported)
static int 		dfs_iop_getattr		(struct vfsmount *mnt, struct dentry *, struct kstat *);
static ssize_t 		dfs_fop_write		(struct file *, const char __user *, size_t, loff_t *);
/// Mount folder Operations only...
static int		dfs_mount_dir_iop_iterate(struct file *flip,struct dir_context *ctx);			//iterate the contents of mount folder only (Unstable yet)




static int dfs_iop_permission(struct inode *nod,int checkThat){
	return 0;//nod->i_mode;

}

struct kmem_cache *dfs_dat_inode_pool;	//memory cache(no supported yet)
int count=-1;				//count of inodes..
struct inode **inodes;			//simple array of inodes to save (in ram..)
char **		names;			//simple array with filenames to ascociate...
struct inode *mountFolder=NULL;		//mount folder reference...

struct dfs_data{			//dfs_file data (not supported yet)
//	char name[12];
	char data[500];
};
/*
	dfs_iop (dfs inode operations struct..)
	supports basic functionality for every dfs inode...
	@Note:More on every line...
*/
static struct inode_operations dfs_iop={
	.lookup=	dfs_iop_lookup,		//searches for a specific inode object , using a path , provived by a dentry parameter..
	.mkdir=		dfs_iop_mkdir,		//creates a new directory (Not supported..)
	.create=	dfs_iop_create,		//creates a new file
//	.getattr=	dfs_iop_getattr,	//get attributes for a file...
};
/*
	Mount Folder file_operations struct .
	because the mount folder is a {FOLDER} we can not open it as regular file(read write etc...)
*/
static struct file_operations dfs_mount_dir_iop={
	.iterate=	dfs_mount_dir_iop_iterate, //Iterate through the elements of mount folder (the created files..)

};
/*
	Per file file_operations struct . we support read only (remember is beta yet.. :) )
*/
static struct file_operations dfs_fop={
	.read=dfs_fop_read,
	.write=dfs_fop_write,
};
/*
	struct file_system_info , contains all the information ascociated with every instance of this filesystem...
*/
static struct file_system_type dfs_info={
	.owner=THIS_MODULE,		//owner ,this module. when loaded and mounted , the usage count of this module will 
					//increased, preventing it from unload on behalf.
	.name="dfs",			//dfs name (as show in /proc/filesystems
	.mount=dfs_construct,		//constructor function of the filesystem...
	.kill_sb=dfs_destruct,		//destructor function of the filesystem
	.fs_flags=FS_REQUIRES_DEV,	//flags to mount(required device to operate)

};
/*
	dfs_init, is the modules initialization entry point...
	@purpose: 	registers the filesystem in kernel
			allocate file arrays
	@return : zero on success , nonzero on error
*/
int dfs_init(void){
	int ret;
	inodes	= (struct inode **)kzalloc(sizeof(struct inode *)*12,0);	 //allocate the inode array..!
	names	= (char **)kzalloc(sizeof(char *)*12,0);			//allocate the names array
//	dfs_dat_inode_pool=kmem_cache_create("dfs_dat_inode_pool",sizeof(struct dfs_dat_inode_struct),0,0,NULL);
	ret=register_filesystem(&dfs_info);					//register filesystem(passed the file_system_type parameter...
	if(unlikely(ret!=0)){							//if fails(unlikely extension because is rare!)
		printk(KERN_ERR"dfs cant register , fail...");
		return ret;
	}
	printk(KERN_INFO"[dfs_log]feel free to mount me , i am ready boss!");	//log message and return success(0)
	return 0;
}
/*
	dfs_exit , is the modules destructor entry point
*/
void dfs_exit(void){
//	kmem_cache_destroy(dfs_dat_inode_pool);
	unregister_filesystem(&dfs_info);	//unregister filesystem
	//TODO:memory leak! free arrays here!
}
module_init(dfs_init);				//define initialization entry point...
module_exit(dfs_exit);				//define unitiallization entry point
/*
	dfs_desructor , to properly kill the superblock...
*/
static void dfs_destruct (struct super_block *sb){
	printk(KERN_INFO"[dfs_log] i hope to i pleased you , master , i am unmounted...");
        kill_block_super(sb);		//call the generic kill_sb func...
        return;				//return..
}
/*
	dfs_contruct , is the filesystem's  initialization entry point (passed as pointer to file_system_type )
	@param dfs_init 	the filesystems ascosiated file_system_type
	@param flags		mount flags
	@param dev_name		the device name (example , /dev/loop0)
	@param data		the mount options as ASCII string
*/
static struct dentry *dfs_construct(struct file_system_type *dfs_inf,int flags,const char *dev_name,void *data){
	struct dentry *root;						//dfs must return the mount dentry with inode ascosiated...
	root=mount_bdev(dfs_inf,flags,dev_name,data,dfs_fill_super); 	//mount with device...
	if(unlikely(root==NULL)){					//if root is null , then propably the superblock is not properly initialized...
		printk(KERN_ERR"[dfs_log]OMG,mount_bdev returned NULL!its a joke? propably not! check for any bug in dfs_fill_super() maybe?");
	}
	return root;
}
/*
	dfs_mount_dir_iop_iterate...
	the only folder exists in this dummy filesystem , is the mount folder , so we know easily , by return all files created...
	TODO : in ls command , this function falls in endless loop , #BUG1
	@param flip the file to iterate
	@param ctx the contents of a folder .
*/
static int dfs_mount_dir_iop_iterate(struct file *flip,struct dir_context *ctx){
        printk(KERN_INFO"dfs_mount_dir_iop_iterate started for file %s",flip->f_path.dentry->d_name.name);
	int i;
	for(i=0;i<count;i++)
		dir_emit(ctx,names[i],15,i+5,DT_REG);
        return 0;
}
/*
	dfs_fill_super , to proper fill the superblock...
	@param sb ,		the filesystems superblock
	@param data , 		the mount options(AS ASCII null-terminated string)
	@param silent 		TODO:Document this parameter , the purpose is undefined for me yet...
*/

static int dfs_fill_super(struct super_block *sb,void *data,int silent){
	struct inode *root_inode;
	sb->s_maxbytes=DFS_MAXBYTES;						//max file bytes...
	root_inode=new_inode(sb);						//create new inode..
	root_inode->i_ino=DFS_FIRSTINODE_INO;					//first inode number...
	inode_init_owner(root_inode,NULL,S_IFDIR);				//initialize the owner in null(root inode)
	root_inode->i_op=&dfs_iop;						//pass the inode operations..
	root_inode->i_fop=&dfs_mount_dir_iop;					//pass the mount file operations..
	root_inode->i_sb=sb;							//pass the superblock..
	root_inode->i_atime =							//pass the initial time..
			root_inode->i_mtime = root_inode->i_ctime = CURRENT_TIME;
	root_inode->i_mode|=(MAY_ACCESS|MAY_READ|MAY_OPEN|MAY_WRITE|MAY_EXEC);	//initial mode(777)
	sb->s_root=d_make_root(root_inode);					//make root for inode..
	mountFolder=root_inode;							//save reference for future use...
	return 0;
}
/**
	dfs_iop_lookup -> searches a inode using a name (provided by dentry->d_name.name)
	@param dir 	-> the parent directory inode
	@param query 	-> the dentry we want to search ;
	@param lookup	-> lookup flags
*/
static struct dentry *dfs_iop_lookup(struct inode *dir,struct dentry *query,unsigned int flags)
{
	printk(KERN_INFO"Path to lookup %s in %d inodes",query->d_name.name,count);		//debug messages
	struct list_head *temp;									//temponary list head..
	struct dentry* tempDir;									//--
	int i=0;										//iterator index
	if(!strcmp(query->d_name.name,"/") || !strcmp(query->d_name.name,".")){			//if we search the mount folder...
		printk(KERN_INFO"\tMain Folder lookup",query->d_name.name);			//...
                d_add(query,mountFolder);							//add the mount folder into the inode..
		return NULL;									//return..
	}
	for(i=0;i<count;i++){
		printk(KERN_INFO" check inode %d",i);						//iterate in all names..
		if(!strcmp(names[i],query->d_name.name)){
			printk(KERN_INFO"founded %s!",names[i]);				//if the i-th name exists...
			inode_init_owner(dir,inodes[i],flags);					//then the i-th inode is the query we want...
			d_add(query,inodes[i]);							//add the inode...
			break;									//return...
		}
	}
	printk("\t%s not found!",query->d_name.name);
	return NULL;
}
/***
	dfs_iop_mkdir -> create folder (not supported)
	@param ptrToParent the inode to parent
	@param parent the dentry to add the folder inode...
	@param mode -> the initial permission...

*/
static int dfs_iop_mkdir(struct inode *ptrToParent,struct dentry *parent,umode_t mode){
	return EOPNOTSUPP;
}
/***
	create inode
	@param dir-> the parent folder inode...
	@param childDentry -> the dentry to add the new inode...
	@param v TODO undocumented parameter

*/
static int dfs_iop_create(struct inode *dir,struct dentry*childDentry,umode_t flags,bool v){
	if(count>12)return -EOPNOTSUPP;
	struct dfs_data *file_data = (struct dfs_data * )kzalloc(sizeof(struct dfs_data),0);
	printk(KERN_ERR"Creation of file %s  %d(%d left)",childDentry->d_name.name,count+1,(12-count+1));
	struct inode *inode;							//to create a new inode file...
	inode=new_inode(dir->i_sb);						//allocate a new inode with superblock same as current super block
	inode->i_private=(void  *)kzalloc(sizeof(struct dfs_data),0);
	inode->i_mode|=(MAY_ACCESS|MAY_READ|MAY_OPEN|MAY_WRITE|MAY_EXEC);	//inital permissions...
	inode->i_op=&dfs_iop;
	inode->i_fop=&dfs_fop;							//inode operations struct pass...
	inode->i_size=0;
	inode->i_private=(void *)file_data;					//private struct with characters...
	inode_init_owner(inode,dir,flags);					//initalize ownership
	d_add(childDentry,inode);						//add inode to childDentry...
	count+=1;								//add dentries +1
	names[count]=childDentry->d_name.name;					//save fileName
	inodes[count]=inode;							//save inode..
	return 0;								//everything fine...
}
/***
	dfs_utill_get_file_data(struct file*file)
	gets the private section of the inode pointed by file
	@param file -> the file to return the data
	@note : this function as all utillity functions do not check for invalid values -> in case of NULL parameter for example , core dump is generated...and reboot is needed
*/
static inline struct dfs_data *dfs_utill_get_file_data(struct file *file){
	return (struct dfs_data *)file->f_path.dentry->d_inode->i_private;
}
/***
	dfs_utill_get_file_size(struct file *file)
	gets the size of the file pointed by @param file (in bytes)
	@param file -> the file to point to
	@see warning in dfs_file_data
*/
static inline ssize_t dfs_utill_get_file_size(struct file *file){
	return file->f_path.dentry->d_inode->i_size;
}
/***
	dfs_utill_update_file_loff(struct file *file , loff_t loff)
	updates the current file pointer by loff bytes (long offset )
	@param
*/
static inline void dfs_utill_update_file_loff(struct file *file,loff_t loff){
	file->f_pos+=loff;
	return;
}
/***
	dfs_utill_update_file_size
	updates the file size by adding the @param write_operation_bytes into bytes...
	@note : negative values is allowed
*/
static inline void dfs_utill_update_file_size(struct file *file,ssize_t write_operation_bytes){
	file->f_path.dentry->d_inode->i_size+=write_operation_bytes;
	return;

}
/**
	dfs_iop_read(struct file*file , char __user *buff ,size_t size,loff *loff)
	if read() operation occurs in dfs file , then dfs_fop_read() called...
	@param file -> the file to apply operation read()
	@param buff -> the user_space buffer to send the data on..
	@param size -> the operation size request...
	@the parameter witch read() will return(the number of bytes)

	returnn 0 in error , nonzero otherwise
	gets the data from private section of the inode
	if the end of file reached , then reads the remaining and loff<size
	TODO:BUG: Starts always from beggining and not from file pointer.... FIXME
	TODO:BUG  Starts always from beggining of the buffer ..
	TODO:BUG: Lock the file so other users wait untill operation succeeds
*/
bool once = 0;
static ssize_t dfs_fop_read(struct file*file,char __user *buff,size_t size, loff_t *loff){
	if(once)return -1;
	else once=1;
	int file_size =			dfs_utill_get_file_size(file);					//get fileSize
	struct dfs_data *file_data =	dfs_utill_get_file_data(file);					//get inode private section
	if(size>file_size)size=file_size;								//if we exceed the filesize , then fix the operation size
	printk(KERN_INFO"reading %d bytes from %s",size,file->f_path.dentry->d_name.name);		//debug message...
	if(unlikely(copy_to_user(buff,file_data->data,size))){						//copy to userspace ..
		printk(KERN_ERR"\tError in reading from file %s",file->f_path.dentry->d_name.name);	//if error , then debug message and exit as (OPERATION NOT SUPPORTED (-OPNOTSUPP)
		return -EOPNOTSUPP;
	}
	printk(KERN_ERR"\t read() completed...");							//operation completed message...
	*loff+=size;											//update buffer pointer to new operation ...
	dfs_utill_update_file_loff(file,size);								//update file pointer to new operation...
	return file_size;										//return size(everything is okay!)
}

/***
	dfs_fop_write(struct file*file,const char __user*buff,size_t size,loff_t *offset)
	if write() operation occurs in dfs file , then dfs_fop_write function will called..
	@param file -> the file to apply write()
	@param buff -> the data to write to file...
	@param size -> the file operation size in bytes...
	@param offset-> the buffer offset
*/
static ssize_t dfs_fop_write(struct file *file, const char __user *buff, size_t size, loff_t *offset){
//	if(size>500)return -EOPNOTSUPP;	//if you want to write more than 500 bytes , its not supported..
	int file_size=		dfs_utill_get_file_size(file);
	char * kern_buff=(char *)kzalloc(size,0);
	copy_from_user(kern_buff,buff,size);
	strcpy(dfs_utill_get_file_data(file)->data,kern_buff);
	printk(KERN_INFO"Write on file %s (%d bytes) completed",file->f_path.dentry->d_name.name,size);
	*offset+=size;
	dfs_utill_update_file_size(file,size);
	return size;



}
/*
int  dfs_iop_getattr(struct vfsmount *mnt, struct dentry *curr, struct kstat *stat){
	printk(KERN_INFO"dfs_iop_getattr called [%s]",curr->d_name.name);
	if(!strcmp(curr->d_name.name,"/")){
		printk(KERN_INFO"\tmain folder attr get");
		stat->mode=S_IFDIR;
	}
	else{
		printk(KERN_INFO"\tregular file attr get");
		stat->mode=S_IFREG;
	}
	stat->ino=3;
	stat->mode|=(MAY_READ);
	stat->size=curr->;
	stat->blocks=0;
	return 0;

}*/
