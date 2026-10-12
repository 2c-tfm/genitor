#ifndef FS_H
#define FS_H

typedef enum PART_FS {
	EXT4,
	EXT3,
	EXT2,
	BTRFS,
	XFS,
	F2FS,
	JFS,
	FAT,
	NTFS,
	SQUASHFS,
	NONE,
} PART_FS;

// ext*
#define EXT_SB_OFFSET       1024
#define EXT_MAGIC_OFFSET    0x38
#define EXT_COMPAT_OFFSET   0x5C
#define EXT_INCOMPAT_OFFSET 0x60
#define EXT_RO_COMPAT_OFFSET 0x64

#define EXT_MAGIC           0xEF53
#define EXT_COMPAT_HAS_JOURNAL 0x0004
#define EXT_INCOMPAT_EXTENTS   0x0040
#define EXT_INCOMPAT_64BIT     0x0080
#define EXT_INCOMPAT_FLEX_BG   0x0200
#define EXT_RO_COMPAT_HUGE_FILE 0x0008
#define EXT_RO_COMPAT_DIR_NLINK 0x0020
#define EXT_RO_COMPAT_EXTRA_ISIZE 0x0040
#define EXT_RO_COMPAT_METADATA_CSUM 0x0400


// btrfs
#define BTRFS_SB_OFFSET  0x10000
#define BTRFS_MAGIC_OFF  0x40
#define BTRFS_MAGIC      "_BHRfS_M"

// xfs
#define XFS_MAGIC "XFSB"

// f2fs
#define F2FS_SB_OFFSET 1024
#define F2FS_MAGIC     0xF2F52010U

// jfs
#define JFS_SB_OFFSET 0x8000
#define JFS_MAGIC     "JFS1"


#define SQUASHFS_MAGIC "hsqs"

#define S_IFMT  00170000
#define S_IFBLK 0060000
#define S_ISBLK(mode) (((mode) & S_IFMT) == S_IFBLK) // is a block device

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define le16toh(x) ((uint16_t)(x))
#define le32toh(x) ((uint32_t)(x))

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#define le16toh(x) \
    ((uint16_t)((((uint16_t)(x) & 0x00FFU) << 8) | \
                (((uint16_t)(x) & 0xFF00U) >> 8)))

#define le32toh(x) \
    ((uint32_t)((((uint32_t)(x) & 0x000000FFU) << 24) | \
                (((uint32_t)(x) & 0x0000FF00U) << 8)  | \
                (((uint32_t)(x) & 0x00FF0000U) >> 8)  | \
                (((uint32_t)(x) & 0xFF000000U) >> 24)))

#else
#error "Unknown host byte order"
#endif


PART_FS detect_fs(const char *path);

extern const char *fs_types[];

#endif
