#include <genitor.h>
#include <syscall.h>
#include <disk.h>
#include <service.h>
#include <time.h>
#include <init.h>
#include <syslog.h>
#include <kmod.h>
#include <fs.h>
#include <fstab.h>

char **environ;

void sigchld_handler(int sig) {
	(void) sig;
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main(int argc, char **argv, char **envp) {
	(void)argc;
	(void)argv;
	environ = envp;
	
	signal(SIGCHLD, sigchld_handler);

	init_env();
	log(SUCCESS, "init system up and running");
	run_services();
	while (true) {
		manage_services();
		sleep(5);
	}
	return 0;
}
