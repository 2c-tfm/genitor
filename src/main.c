#include <genitor.h>
#include <string.h>
#include <syscall.h>
#include <init.h>
#include <syslog.h>

#include <fs.h>
#include <fstab.h>

int main(void) {
	init_env();
	puts("init system up and running");
	while(1);
	return 0;
}
