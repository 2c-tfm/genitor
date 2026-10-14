#include <genitor.h>
#include <syscall.h>
#include <time.h>
#include <string.h>
#include <service.h>
#include <dir.h>
#include <file.h>
#include <unistd.h>
#include <malloc.h>
#include <printf.h>

svc *master = NULL;

static bool is_service_up(const char *service) {
	svc *it;

	if (master == NULL)
		return false;
	it = master;

	while (it != NULL) {
		if (strcmp(service, it->name) == 0)
			return true;
		it = it->next;
	}

	return false;
}

static void add_service(svc *service) {
	svc *it;

	service->next = NULL;
	if (master == NULL) {
		master = service;
		return;
	}

	it = master;

	while (it->next != NULL) {
		it = it->next;
	}
	it->next = service;
}

static void remove_service_record(svc *target) {
	svc **link = &master;

	while (*link != NULL) {
		if (*link == target) {
			*link = target->next;
			free(target->name);
			free(target);
			return;
		}
		link = &(*link)->next;
	}
}

static int service_is_desired(const char *name) {
	dir_iterator *dfd;
	dirent *entry;
	int found = 0;

	dfd = dir_open(RUN_HOME);
	if (dfd == NULL)
		return -1;

	while ((entry = dir_read(dfd)) != NULL) {
		if (strcmp(entry->d_name, name) == 0) {
			found = 1;
			free(entry);
			break;
		}

		free(entry);
	}

	dir_close(dfd);
	return found;
}

static int redirect_service_stdio(const char *log_path) {
	int null_fd;
	int log_fd;

	null_fd = open("/dev/null", O_RDONLY);
	log_fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND);
	if (log_fd < 0) {
		if (null_fd > 2)
			close(null_fd);
		return -1;
	}

	if (null_fd >= 0 && dup2(null_fd, 0) < 0) {
		if (null_fd > 2)
			close(null_fd);
		if (log_fd > 2)
			close(log_fd);
		return -1;
	}

	if (dup2(log_fd, 1) < 0 || dup2(log_fd, 2) < 0) {
		if (null_fd > 2)
			close(null_fd);
		if (log_fd > 2)
			close(log_fd);
		return -1;
	}

	if (null_fd > 2)
		close(null_fd);
	if (log_fd > 2)
		close(log_fd);

	return 0;
}

static int run_child_routine(const char *service) {
	char *service_dir;
	char *log_path;
	size_t log_path_size;

	service_dir = strjoin(RUN_HOME, service);

	if (chdir(service_dir) < 0) {
		log(ERROR, "cannot chdir to service %s", service);
		free(service_dir);
		return -1;
	}
	free(service_dir);

	log_path_size = strlen(SYSLOG_HOME) + strlen(service) + strlen(".log") + 1;
	log_path = malloc(log_path_size);
	snprintf(log_path, log_path_size, SYSLOG_HOME "%s.log", service);

	if (redirect_service_stdio(log_path) < 0) {
		log(ERROR, "cannot open log file for %s", service);
		free(log_path);
		return -1;
	}
	free(log_path);

	if (fsize("./run") < 0) {
		log(ERROR, "run script not found for %s", service);
		return -1;
	}

	execve("./run", NULL, environ);
	return -1;
}

static int run_service(const char *service) {
	svc *entry;
	pid_t pid;

	if (is_service_up(service))
		return 0;

	entry = malloc(sizeof(*entry));
	if (entry == NULL)
		return -1;

	entry->name = strdup(service);
	entry->birth = time();
	entry->stop_time = 0;
	entry->stopping = false;
	entry->kill_sent = false;
	entry->pid = -1;
	entry->next = NULL;

	pid = fork();
	if (pid < 0) {
		log(ERROR, "fork failed for service %s", service);
		free(entry->name);
		free(entry);
		return -1;
	}

	if (pid == 0) {
		run_child_routine(service);
		exit(-1);
	}

	entry->pid = pid;
	add_service(entry);
	log(SUCCESS, "service started %s [pid %d]", entry->name, entry->pid);
	return 0;
}

int64_t run_services(void) {
	log(DEBUG, "run_services");
	dir_iterator *dfd;
	dirent *entry;

	dfd = dir_open(RUN_HOME);
	if (dfd == NULL) {
		return -1;
	}

	while ((entry = dir_read(dfd)) != NULL) {
		if (!is_service_up(entry->d_name)) {
			log(DEBUG, "starting service %s", entry->d_name);
			if (run_service(entry->d_name) < 0) {
				log(ERROR, "cant start service %s", entry->d_name);
			}
		}

		free(entry);
	}

	dir_close(dfd);
	return 0;
}

static void request_service_stop(svc *it, uint64_t now) {
	kill(it->pid, SIGTERM);
	it->stopping = true;
	it->stop_time = now;
	log(INFO, "servcie stopped %s [pid %d]", it->name, it->pid);
}

void manage_services(void) {
	static bool directory_warning = false;
	svc *it = master;
	svc *next;
	uint64_t now;
	int desired;
	int status;
	long result;

	now = time();
	while (it != NULL) {
		next = it->next;

		desired = service_is_desired(it->name);
		if (desired < 0) {
			if (!directory_warning) {
				log(ERROR, "Cannot inspect %s; leaving tracked services alone", RUN_HOME);
				directory_warning = true;
			}
		} else {
			directory_warning = false;
			if (desired == 0 && !it->stopping)
				request_service_stop(it, now);
		}

		if (it->stopping && !it->kill_sent && now >= it->stop_time && 
				now - it->stop_time >= STOP_GRACE_SECONDS) {
			result = kill(it->pid, SIGKILL);
			if (result == 0 || result == -ERR_ESRCH) {
				it->kill_sent = true;
				log(WARN, "Escalating stop for service %s [pid %d]", it->name, it->pid);
			} else {
				log(ERROR, "SIGKILL failed for service %s [pid %d]",
				it->name, it->pid);
			}
		}

		result = wait4(it->pid, &status, WAIT_NOHANG, 0);
		if (result == (long)it->pid) {
			log(INFO, "service %s [pid %d] exited (wait status %d)",
			it->name, it->pid, status);
			remove_service_record(it);
		} else if (result == -ERR_ECHILD) {
			log(WARN, "service %s [pid %d] is no longer a waitable child", it->name, it->pid);
			remove_service_record(it);
		} else if (result < 0 && result != -ERR_EINTR) {
			log(ERROR, "wait4 failed for service %s [pid %d]",
			it->name, it->pid);
		}

		it = next;
	}
}

