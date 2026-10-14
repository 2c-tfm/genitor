#ifndef SIGNAL_H
#define SIGNAL_H

#include <types.h>

#define SA_RESTART	0x0002	/* Restart syscall on signal return.  */
#define SA_NODEFER	0x0010	/* Don't automatically block the signal when
				    its handler is being executed.  */
#define SA_RESETHAND	0x0004	/* Reset to SIG_DFL on entry to handler.  */
#define	SA_NOCLDSTOP	0x0008	/* Don't send SIGCHLD when children stop.  */
#define SA_SIGINFO	0x0040	/* Signal handler with SA_SIGINFO args */

// taked from glibc2.44
/* Historical signals specified by POSIX. */
#define SIGBUS		10	/* Bus error.  */
#define SIGSYS		12	/* Bad system call.  */

/* New(er) POSIX signals (1003.1-2008, 1003.1-2013).  */
#define SIGURG		16	/* Urgent data is available at a socket.  */
#define SIGSTOP		17	/* Stop, unblockable.  */
#define SIGTSTP		18	/* Keyboard stop.  */
#define SIGCONT		19	/* Continue.  */
#define SIGCHLD		20	/* Child terminated or stopped.  */
#define SIGTTIN		21	/* Background read from control terminal.  */
#define SIGTTOU		22	/* Background write to control terminal.  */
#define SIGPOLL		23	/* Pollable event occurred (System V).  */
#define SIGXCPU		24	/* CPU time limit exceeded.  */
#define SIGVTALRM	26	/* Virtual timer expired.  */
#define SIGPROF		27	/* Profiling timer expired.  */
#define SIGXFSZ		25	/* File size limit exceeded.  */
#define SIGUSR1		30	/* User-defined signal 1.  */
#define SIGUSR2		31	/* User-defined signal 2.  */
#define SIGTERM 15
#define SIGKILL 9

#define	WNOHANG		1	/* Don't block waiting.  */
#define	WUNTRACED	2	/* Report status of stopped children.  */

typedef void (*__sighandler_t) (int);
typedef typeof(void (int))  *sighandler_t;

union sigval
{
    int sival_int;
    void *sival_ptr;
};

typedef union sigval __sigval_t;

typedef struct
  {
    int si_signo;		/* Signal number.  */
    int si_errno;		/* If non-zero, an errno value associated with
				   this signal, as defined in <errno.h>.  */
    int si_code;		/* Signal code.  */
    int si_pid;		/* Sending process ID.  */
    int si_uid;		/* Real user ID of sending process.  */
    void *si_addr;		/* Address of faulting instruction.  */
    int si_status;		/* Exit value or signal.  */
    long int si_band;		/* Band event for SIGPOLL.  */
    __sigval_t si_value;	/* Signal value.  */
  } siginfo_t;

struct sigaction {
	union {
		__sighandler_t sa_handler;
		void (*sa_sigaction) (int, siginfo_t *, void *);
	};
	unsigned long int sa_mask;
	int sa_flags;
};

sighandler_t signal (int signum, sighandler_t handler);
pid_t waitpid(pid_t pid, int *wstatus, int options);
int sigaction(int signum, const struct sigaction *act,
		struct sigaction *oldact);

#endif
