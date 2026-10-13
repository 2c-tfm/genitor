#include <genitor.h>
#include <string.h>
#include <unistd.h>
#include <disk.h>
#include <fs.h>

const char *fs_types[] = { "ext4", "ext3", "ext2", "btrfs", "xfs",
	"f2fs", "jfs", "vfat", "ntfs3", "squashfs", NULL};

static int read_u16(int fd, uint16_t *v, off_t off) {
    uint16_t raw;

    if (lseek(fd, off, SEEK_SET) == (off_t)-1)
        return -1;

    if (read(fd, &raw, sizeof(raw)) != sizeof(raw))
        return -1;

    *v = le16toh(raw);
    return 0;
}

static int read_u32(int fd, uint32_t *v, off_t off) {
    uint32_t raw;

    if (lseek(fd, off, SEEK_SET) == (off_t)-1)
        return -1;

    if (read(fd, &raw, sizeof(raw)) != sizeof(raw))
        return -1;

    *v = le32toh(raw);
    return 0;
}

static PART_FS detect_ext(int pfd) {
	uint16_t magic;
	PART_FS ext;
	uint32_t compat, incompat, ro_compat;

	if (read_u16(pfd, &magic, EXT_SB_OFFSET + EXT_MAGIC_OFFSET) < 0 ||
		read_u32(pfd, &compat, EXT_SB_OFFSET + EXT_COMPAT_OFFSET) < 0 ||
		read_u32(pfd, &incompat, EXT_SB_OFFSET + EXT_INCOMPAT_OFFSET) < 0 ||
		read_u32(pfd, &ro_compat, EXT_SB_OFFSET + EXT_RO_COMPAT_OFFSET) < 0)
		return NONE;
	if (magic != EXT_MAGIC) {
		ext = NONE;
	}
	if ((incompat & (EXT_INCOMPAT_EXTENTS |
			EXT_INCOMPAT_64BIT |
			EXT_INCOMPAT_FLEX_BG)) ||
		(ro_compat & (EXT_RO_COMPAT_HUGE_FILE |
			EXT_RO_COMPAT_DIR_NLINK |
			EXT_RO_COMPAT_EXTRA_ISIZE |
			EXT_RO_COMPAT_METADATA_CSUM))) {
		ext = EXT4;
	} else if (compat & EXT_COMPAT_HAS_JOURNAL)
		ext = EXT3;
	else 
		ext = EXT2;
	return (ext);
}

static PART_FS detect_btrfs(int pfd) {
	char magic[8];

	if (lseek(pfd, BTRFS_SB_OFFSET + BTRFS_MAGIC_OFF,
		SEEK_SET) == (off_t)-1)
		return NONE;
	ssize_t n = read(pfd, magic, sizeof(magic));
	if (n != sizeof(magic))
		return NONE;

	if (memcmp(magic, BTRFS_MAGIC, sizeof(magic)) == 0)
		return BTRFS;

	return NONE;
}

static PART_FS detect_xfs(int pfd) {
	char magic[4];

	if (lseek(pfd, 0, SEEK_SET) == (off_t)-1)
		return NONE;

	ssize_t n = read(pfd, magic, sizeof(magic));
	if (n != sizeof(magic))
		return NONE;
	if (memcmp(magic, XFS_MAGIC, sizeof(magic)) == 0)
		return XFS;

	return NONE;
}

static PART_FS detect_f2fs(int pfd) {
	uint32_t magic;

	if (lseek(pfd, F2FS_SB_OFFSET, SEEK_SET) == (off_t)-1)
		return NONE;

	ssize_t n = read(pfd, &magic, sizeof(magic));
	if (n != sizeof(magic))
		return NONE;

	if (magic == F2FS_MAGIC)
		return F2FS;

	return NONE;
}

static PART_FS detect_jfs(int pfd) {
	char magic[4];

	if (lseek(pfd, JFS_SB_OFFSET, SEEK_SET) == (off_t)-1)
		return NONE;

	ssize_t n = read(pfd, magic, sizeof(magic));
	if (n != sizeof(magic))
		return NONE;

	if (memcmp(magic, JFS_MAGIC, sizeof(magic)) == 0)
		return JFS;

	return NONE;
}

static PART_FS detect_fat(int pfd) {
	unsigned char bs[512];

	if (lseek(pfd, 0, SEEK_SET) == (off_t)-1)
		return NONE;

	if (read(pfd, bs, sizeof(bs)) != sizeof(bs))
		return NONE;

	if (bs[510] != 0x55 || bs[511] != 0xAA)		// boot sig
		return NONE;

	uint16_t bps = bs[11] | ((uint16_t)bs[12] << 8); // bps

	if (bps != 512 && bps != 1024 &&
		bps != 2048 && bps != 4096)
		return NONE;

	if (bs[13] == 0 || (bs[13] & (bs[13] - 1)) != 0)
		return NONE;

	if (bs[16] == 0)
		return NONE;

	return FAT;
}

static PART_FS detect_ntfs(int pfd) {
	unsigned char bs[512];

	if (lseek(pfd, 0, SEEK_SET) == (off_t)-1)
		return NONE;

	if (read(pfd, bs, sizeof(bs)) != sizeof(bs))
		return NONE;

	if (memcmp(bs + 3, "NTFS    ", 8) != 0)
		return NONE;

	if (bs[510] != 0x55 || bs[511] != 0xAA)
		return NONE;

	return NTFS;
}

static PART_FS detect_squashfs(int pfd) {
	char magic[4];

	if (lseek(pfd, 0, SEEK_SET) == (off_t)-1)
		return NONE;

	ssize_t n = read(pfd, magic, sizeof(magic));
	if (n != sizeof(magic))
		return NONE;

	if (memcmp(magic, SQUASHFS_MAGIC, 4) == 0)
		return SQUASHFS;

	return NONE;
}


PART_FS detect_fs(const char *path) {
	struct stat st;
	PART_FS fs;
	
	PART_FS (*fs_detectors[])(int) = {
		detect_ext,
		detect_btrfs,
		detect_xfs,
		detect_f2fs,
		detect_jfs,
		detect_fat,
		detect_ntfs,
		detect_squashfs,
	};

	uint32_t fs_dsize = sizeof(fs_detectors) / sizeof(fs_detectors[0]);
	uint32_t i = 0;

	if (stat(path, &st) == -1)
		return NONE;

	if (!S_ISBLK(st.st_mode)) {
		log(ERROR, "%s is not a block device", path);
		return NONE;
	}

	int fd = open(path, O_RDONLY);
	if (fd == -1) {
		log(ERROR, "cannot open %s", path);
		return NONE;
	}

	// detecting
	while (i < fs_dsize) {
		fs = fs_detectors[i](fd);
		if (fs != NONE)
			break;
		lseek(fd, 0, SEEK_SET);
	}

	close(fs);
	return fs;
}
