#ifndef EXT2_H
#define EXT2_h

#include "kernel.h"
#include "fs.h"

#define EXT2_SUPER_MAGIC 0xEF53

struct ext2_superblock {
	uint32 inodes_count;
	uint32 blocks_count;
	uint32 r_blocks_count;
	uint32 free_blocks_count;
	uint32 free_inodes_count;
	uint32 first_data_block;
	uint32 log_block_size;
	uint32 log_frag_size;
	uint32 blocks_per_group;
	uint32 frags_per_group;
	uint32 inodes_per_group;
	uint32 mtime;
	uint32 wtime;

	uint16 mnt_count;
	uint16 max_mnt_count;
	uint16 magic;
	uint16 state;
	uint16 errors;
	uint16 minor_rev_level;

	uint32 lastcheck;
	uint32 checkinterval;
	uint32 creator_os;
	uint32 rev_level;

	uint16 def_resuid;
	uint16 def_resgid;

	/* EXT2_DYNAMIC_REV */
	uint32 first_ino;
	uint16 inode_size;
	uint16 block_group_nr;
	uint32 feature_compat;
	uint32 feature_incompat;
	uint32 feature_ro_compat;

	uint8 uuid[16];
	uint8 volume_name[16];

	uint8 last_mounted[16];

	uint32 algo_bitmap;

	/* Performance Hints */
	uint8 prealloc_blocks;
	uint8 prealloc_dir_blocks;
	uint16 _padding;

	/* Journaling Support */
	uint8 journal_uuid[16];
	uint32 journal_inum;
	uint32 jounral_dev;
	uint32 last_orphan;

	/* Directory Indexing Support */
	uint32 hash_seed[4];
	uint8 def_hash_version;
	uint16 _padding_a;
	uint8  _padding_b;

	/* Other Options */
	uint32 default_mount_options;
	uint32 first_meta_bg;
	uint8 _unused[760];

} __attribute__((packed));

typedef struct ext2_superblock ext2_superblock_t;

struct ext2_bgdescriptor {
	uint32 block_bitmap;
	uint32 inode_bitmap;
	uint32 inode_table;
	uint16 free_blocks_count;
	uint16 free_inodes_count;
	uint16 used_dirs_count;
	uint16 pad;
	uint8  reserved[12];
} __attribute__((packed));

typedef struct ext2_bgdescriptor ext2_bgdescriptor_t;

/* File Types */
#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000

/* setuid, etc. */
#define EXT2_S_ISUID	0x0800
#define EXT2_S_ISGID	0x0400
#define EXT2_S_ISVTX	0x0200

/* rights */
#define EXT2_S_IRUSR	0x0100
#define EXT2_S_IWUSR	0x0080
#define EXT2_S_IXUSR	0x0040
#define EXT2_S_IRGRP	0x0020
#define EXT2_S_IWGRP	0x0010
#define EXT2_S_IXGRP	0x0008
#define EXT2_S_IROTH	0x0004
#define EXT2_S_IWOTH	0x0002
#define EXT2_S_IXOTH	0x0001


struct ext2_inodetable {
	uint16 mode;
	uint16 uid;
	uint32 size;
	uint32 atime;
	uint32 ctime;
	uint32 mtime;
	uint32 dtime;
	uint16 gid;
	uint16 links_count;
	uint32 blocks;
	uint32 flags;
	uint32 osd1;
	uint32 block[15];
	uint32 generation;
	uint32 file_acl;
	uint32 dir_acl;
	uint32 faddr;
	uint8  osd2[12];
} __attribute__((packed));

typedef struct ext2_inodetable ext2_inodetable_t;

struct ext2_dir {
	uint32 inode;
	uint16 rec_len;
	uint8 name_len;
	uint8  file_type;
	char     name; /* Actually a set of characters, at most 255 bytes */
} __attribute__((packed));

typedef struct ext2_dir ext2_dir_t;

#endif
