#include "../include/kernel.h"
#include "../include/fs.h"

fs_node_t *fs_root = 0;

uint32 read_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer) {
        if (node->read != 0) {
                return node->read(node, offset, size, buffer);
        } else {
                return 0;
        }
}

uint32 write_fs(fs_node_t *node, uint32 offset, uint32 size, uint8 *buffer) {
        if (node->write != 0) {
                return node->write(node, offset, size, buffer);
        } else {
                return 0;
        }
}

void open_fs(fs_node_t *node, uint8 read, uint8 write) {
        if (node->open != 0)
                node->open(node, read, write);
}

void close_fs(fs_node_t *node) {
        if (node->close != 0) 
                node->close(node);
}

struct dirent * readdir_fs(fs_node_t *node, uint32 index) {
        if ((node->flags & 0x07) == FS_DIRECTORY && node->readdir != 0){
                return node->readdir(node, index);
        } else {
                return (struct dirent *)NULL;
        }
}

fs_node_t *finddir_fs(fs_node_t *node, char *name) {
        if ((node->flags & 0x07) == FS_DIRECTORY && node->readdir != 0) {
                return node->finddir(node, name);
        } else {
                return (fs_node_t *)NULL;
        }
}

fs_node_t *
kopen(
		const char *filename,
		uint32 flags
     ) {
	/* later */
}
