#ifndef SIGNAL_H
#define SIGNAL_H

#define SA_RESTART	0x0002	/* Restart syscall on signal return.  */
#define SA_NODEFER	0x0010	/* Don't automatically block the signal when
				    its handler is being executed.  */
#define SA_RESETHAND	0x0004	/* Reset to SIG_DFL on entry to handler.  */
#define	SA_NOCLDSTOP	0x0008	/* Don't send SIGCHLD when children stop.  */
#define SA_SIGINFO	0x0040	/* Signal handler with SA_SIGINFO args */

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

int sigaction(int signum, const struct sigaction *act,
		struct sigaction *oldact);

#endif
