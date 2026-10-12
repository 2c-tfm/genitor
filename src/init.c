#include <genitor.h>
#include <fs.h>
#include <fstab.h>
#include <malloc.h>
#include <disk.h>
#include <string.h>
#include <parse.h>
#include <file.h>
#include <syscall.h>


static int init_dev_mounts(void) {
	
	if (is_mounted("/dev") == false) {
		if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0)
			puts("mount /dev failed");
	}
	if (is_mounted("/proc") == false) {
		if (mount("proc", "/proc", "proc", 0, NULL) != 0)
			puts("mount /proc failed");
	}

	if (is_mounted("/sys") == false) {
		if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
			puts("mount /sys failed");
	}

	return 0;
}


// partition_healthcheck - mounts root partition in read only mode, checks 
// 			if it does have all required files to start
static int partition_healthcheck() {
	char *uuid_val;
	char *root_part;
	PART_FS fs;

	uuid_val = extract_cmdline("root");
	root_part = resolve_disk_location(uuid_val);
	free(uuid_val);
	if (root_part == NULL){
		puts("root partition isn't found");
		return (-1);
	}

	// mounting the root directory
	// TODO: for now we take root partition type as ext4 for granted 
	// it must be checked
	fs = detect_fs(root_part);

	if (fs == NONE) {
		puts("cannot detect root partition fs type");
		return -1;
	}

	if (mount(root_part, "/", fs_types[fs], MS_RDONLY, NULL) != 0) {
		printf("mount / failed\n");
		return -1;
	}

	fstab = fstab_populate();
	if (fstab == NULL)
		puts("Mounting fstab partitions failed");

	// end of health check
	if (root_part != NULL) {
		umount2(root_part, 0);
		free(root_part);
	}

	return (0);
	
}

static int init_syslog() {
	int logfd;

	logfd = open("/dev/console", O_RDWR);
	if (logfd < 0)
		return -1;
	dup2(logfd, 0);
	dup2(logfd, 1);
	dup2(logfd, 2);
	return 0;
}

int init_env(void) {
	init_dev_mounts();
	init_syslog();

	// if the root is mounted we just skip the checks and mounting
	if (is_mounted("/") == false) {
		partition_healthcheck();
	}

	return (0);
}
