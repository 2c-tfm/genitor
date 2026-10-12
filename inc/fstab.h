#ifndef FSTAB_H
#define FSTAB_H

typedef struct fstab_entry {
	char *path;		// eg /dev/sda2
	char *mount_point;
	char *fstype;
	char *options;
	struct fstab_entry *next;
} fstab_entry;

fstab_entry *fstab_populate(void);

extern fstab_entry *fstab;

#endif
