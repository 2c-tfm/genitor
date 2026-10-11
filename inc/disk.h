#ifndef DISK_H
#define DISK_H

#define UUID_BASEPATH "/dev/disk/by-uuid/"
#define PARTUUID_BASEPATH "/dev/disk/by-partuuid/"

#define PATH_MAX 1024

#define AT_FDCWD -100

typedef enum DISK_IDENT {
	UUID,
	PARTUUID,
	LABEL,
} DISK_IDENT;

char *resolve_disk_location(char *val);

#endif
