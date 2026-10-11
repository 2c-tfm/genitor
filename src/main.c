#include <genitor.h>
#include <string.h>
#include <syscall.h>
#include <init.h>
#include <syslog.h>

int main(void) {
	init_env();
	printf("init system up and running");
	while(1);
	return 0;
}
