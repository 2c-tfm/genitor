#include <genitor.h>
#include <syscall.h>

// TODO: we still need to do checks here for mounting failures
// this will be done when a logging system is working
int init_dev_mounts(void) {
	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sys", 0, NULL);
	mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);
	return 0;
}
