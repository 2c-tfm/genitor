#include <genitor.h>
#include <string.h>
#include <syscall.h>
#include <disk.h>
#include <init.h>
#include <syslog.h>
#include <kmod.h>
#include <fs.h>
#include <fstab.h>

int main(void) {
	init_env();
	log(SUCCESS, "init system up and running");
	while(1);
	return 0;
}
