#include <genitor.h>
#include <malloc.h>
#include <disk.h>
#include <string.h>
#include <parse.h>
#include <file.h>
#include <syscall.h>

// TODO: we still need to do checks here for mounting failures
// this will be done when a logging system is working
static int init_dev_mounts(void) {
	
	if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) != 0)
		puts("mount /dev failed");
	if (mount("proc", "/proc", "proc", 0, NULL) != 0)
		puts("mount /proc failed");
	if (mount("sysfs", "/sys", "sysfs", 0, NULL) != 0)
		puts("mount /sys failed");
	return 0;
}


// partition_healthcheck - mounts root partition in read only mode, checks 
// 			if it does have all required files to start
static int partition_healthcheck() {
	char *uuid_val;
	char *part_path;

	uuid_val = extract_cmdline("root");
	part_path = resolve_disk_location(uuid_val);
	(void)part_path;

	free(uuid_val);
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
	partition_healthcheck();

	return (0);
}
