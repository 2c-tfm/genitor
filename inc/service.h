#ifndef SERVICE_H
#define SERVICE_H

#define WAIT_NOHANG            1
#define ERR_EINTR              4
#define ERR_ESRCH              3
#define ERR_ECHILD             10
#define STOP_GRACE_SECONDS     5


#define SERVICE_HOME	"/etc/genitor/sv/"
#define RUN_HOME	"/run/genitor/service/"
#define SYSLOG_HOME	"/var/log/syslog/"
// #define SYSLOG_HOME	"/tmp/syslog/"

typedef struct svc {
	char *name;
	pid_t pid;
	uint64_t birth;
	uint64_t stop_time;
	bool stopping;
	bool kill_sent;
	struct svc *next;
} svc;

int64_t run_services(void);
void manage_services(void);

#endif
