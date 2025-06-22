#include "../../include/kernel.h"
#include "../../include/ext2.h"
#include "../../include/fs.h"

ext2_superblock_t * initrd_superblock;
ext2_inodetable_t * initrd_root_node;
fs_node_t *	    initrd_root;
fs_node_t *         initrd_dev;

uint32 initrd_node_from_file(ext2_inodetable_t *inode, ext2_dir_t *direntry, fs_node_t *fnode);
uint32 read_initrd(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
uint32 write_initrd(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer);
void open_initrd(fs_node_t *node, uint8 read, uint8 write);
void close_initrd(fs_node_t *node);
struct dirent *readdir_initrd(fs_node_t *node, uint32 index);
fs_node_t *finddir_initrd(fs_node_t *node, char *name);

uint32
read_initrd(
		fs_node_t *node,
		uint32 offset,
		uint32 size,
		uint8 *buffer
	   ) {
	return 0;
}

uint32
write_initrd(
		fs_node_t *node,
		uint32 offset,
		uint32 size,
		uint8 *buffer
	    ) {
	/*
	 * Not implemented
	 */
	return 0;
}

void
open_initrd(
		fs_node_t *node,
		uint8 read,
		uint8 write
	   ) {
	// idk
}

void
close_initrd(
		fs_node_t *node
	    ) {
	/*
	 * Nothing to do here
	 */
}

struct dirent *
readdir_initrd(
		fs_node_t *node,
		uint32 index
	      ) {
	return NULL;
}

fs_node_t *
finddir_initrd(
		fs_node_t *node,
		char *name
	      ) {
	return NULL;
}

uint32
initrd_node_from_file(
		ext2_inodetable_t * inode,
		ext2_dir_t * direntry,
		fs_node_t * fnode
		) {
	if (!fnode) {
		/* fuk u */
		return 0;
	}
	/* Information from the direntry */
	fnode->inode = direntry->inode;
	memcpy(&fnode->name, &direntry->name, direntry->name_len);
	fnode->name[direntry->name_len] = '\0';
	/* Information from the inode */
	fnode->uid = inode->uid;
	fnode->gid = inode->gid;
	fnode->length = inode->size;
	fnode->mask = inode->mode & 0xFFF;
	/* File Flags */
	fnode->flags = 0;
	if (inode->mode & EXT2_S_IFREG) {
		fnode->flags &= FS_FILE;
	}
	if (inode->mode & EXT2_S_IFDIR) {
		fnode->flags &= FS_DIRECTORY;
	}
	if (inode->mode & EXT2_S_IFBLK) {
		fnode->flags &= FS_BLOCKDEVICE;
	}
	if (inode->mode & EXT2_S_IFCHR) {
		fnode->flags &= FS_CHARDEVICE;
	}
	if (inode->mode & EXT2_S_IFIFO) {
		fnode->flags &= FS_PIPE;
	}
	if (inode->mode & EXT2_S_IFLNK) {
		fnode->flags &= FS_SYMLINK;
	}
	fnode->read	= read_initrd;
	fnode->write	= write_initrd;
	fnode->open	= open_initrd;
	fnode->close	= close_initrd;
	fnode->readdir	= readdir_initrd;
	fnode->finddir	= finddir_initrd;
}
