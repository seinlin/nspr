/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 * 
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 * 
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#ifndef nspr_pthread_defs_h___
#define nspr_pthread_defs_h___

#include <pthread.h>
#include "prthread.h"

#if defined(PTHREADS_USER)
/*
** Thread Local Storage 
*/
extern pthread_key_t current_thread_key;
extern pthread_key_t current_cpu_key;
extern pthread_key_t last_thread_key;
extern pthread_key_t intsoff_key;

#define _MD_CURRENT_THREAD() 			\
			((struct PRThread *) pthread_getspecific(current_thread_key))
#define _MD_CURRENT_CPU() 				\
			((struct _PRCPU *) pthread_getspecific(current_cpu_key))
#define _MD_LAST_THREAD()				\
			((struct PRThread *) pthread_getspecific(last_thread_key))
	
#define _MD_SET_CURRENT_THREAD(newval) 			\
	pthread_setspecific(current_thread_key, (void *)newval)

#define _MD_SET_CURRENT_CPU(newval) 			\
	pthread_setspecific(current_cpu_key, (void *)newval)

#define _MD_SET_LAST_THREAD(newval)	 			\
	pthread_setspecific(last_thread_key, (void *)newval)

#define _MD_SET_INTSOFF(_val)
#define _MD_GET_INTSOFF()	1
	
/*
** Initialize the thread context preparing it to execute _main.
*/
#define _MD_INIT_CONTEXT(_thread, _sp, _main, status)			\
    PR_BEGIN_MACRO				      							\
        *status = PR_TRUE;              						\
		if (SAVE_CONTEXT(_thread)) {							\
	    	(*_main)();											\
		}														\
		_MD_SET_THR_SP(_thread, _sp);							\
		_thread->no_sched = 0; 									\
    PR_END_MACRO

#define _MD_SWITCH_CONTEXT(_thread)  								\
    PR_BEGIN_MACRO 													\
	PR_ASSERT(_thread->no_sched);									\
	if (!SAVE_CONTEXT(_thread)) {									\
		(_thread)->md.errcode = errno;  							\
		_MD_SET_LAST_THREAD(_thread);								\
		_PR_Schedule();		     									\
    } else {														\
		 (_MD_LAST_THREAD())->no_sched = 0;							\
	}																\
    PR_END_MACRO

/*
** Restore a thread context, saved by _MD_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_thread)								\
    PR_BEGIN_MACRO 													\
    errno = (_thread)->md.errcode; 									\
    _MD_SET_CURRENT_THREAD(_thread); 								\
	_thread->no_sched = 1;											\
    GOTO_CONTEXT(_thread); 											\
    PR_END_MACRO


/* Machine-dependent (MD) data structures */

struct _MDThread {
    jmp_buf 		jb;
    int				id;
    int				errcode;
	pthread_t		pthread;
	pthread_mutex_t	pthread_mutex;
	pthread_cond_t	pthread_cond;
	int				wait;
};

struct _MDThreadStack {
    PRInt8 notused;
};

struct _MDLock {
	pthread_mutex_t mutex;
};

struct _MDSemaphore {
    PRInt8 notused;
};

struct _MDCVar {
	pthread_mutex_t mutex;
};

struct _MDSegment {
    PRInt8 notused;
};

struct _MDCPU {
    jmp_buf 			jb;
	pthread_t 			pthread;
	struct _MDCPU_Unix 	md_unix;
};

/*
#define _MD_NEW_LOCK(lock) PR_SUCCESS
#define _MD_FREE_LOCK(lock)
#define _MD_LOCK(lock)
#define _MD_UNLOCK(lock)
*/

extern pthread_mutex_t _pr_heapLock;

#define _PR_LOCK(lock) pthread_mutex_lock(lock)

#define _PR_UNLOCK(lock) pthread_mutex_unlock(lock)


#define _PR_LOCK_HEAP()	{									\
				if (_pr_primordialCPU) {					\
					_PR_LOCK(_pr_heapLock);					\
				}

#define _PR_UNLOCK_HEAP() 	if (_pr_primordialCPU)	{		\
					_PR_UNLOCK(_pr_heapLock);				\
				}											\
			  }

PR_EXTERN(PRStatus) _MD_NEW_LOCK(struct _MDLock *md);
PR_EXTERN(void) _MD_FREE_LOCK(struct _MDLock *lockp);

#define _MD_LOCK(_lockp) _PR_LOCK(&(_lockp)->mutex)
#define _MD_UNLOCK(_lockp) _PR_UNLOCK(&(_lockp)->mutex)

#define _MD_INIT_IO()
#define _MD_IOQ_LOCK()
#define _MD_IOQ_UNLOCK()
#define _MD_CHECK_FOR_EXIT()

PR_EXTERN(PRStatus) _MD_InitThread(struct PRThread *thread);
#define _MD_INIT_THREAD _MD_InitThread
#define _MD_INIT_ATTACHED_THREAD _MD_InitThread

PR_EXTERN(void) _MD_ExitThread(struct PRThread *thread);
#define _MD_EXIT_THREAD _MD_ExitThread

PR_EXTERN(void) _MD_SuspendThread(struct PRThread *thread);
#define _MD_SUSPEND_THREAD _MD_SuspendThread

PR_EXTERN(void) _MD_ResumeThread(struct PRThread *thread);
#define _MD_RESUME_THREAD _MD_ResumeThread

PR_EXTERN(void) _MD_SuspendCPU(struct _PRCPU *thread);
#define _MD_SUSPEND_CPU _MD_SuspendCPU

PR_EXTERN(void) _MD_ResumeCPU(struct _PRCPU *thread);
#define _MD_RESUME_CPU _MD_ResumeCPU

#define _MD_BEGIN_SUSPEND_ALL()
#define _MD_END_SUSPEND_ALL()
#define _MD_BEGIN_RESUME_ALL()
#define _MD_END_RESUME_ALL()

PR_EXTERN(void) _MD_EarlyInit(void);
#define _MD_EARLY_INIT _MD_EarlyInit

#define _MD_FINAL_INIT _PR_UnixInit

PR_EXTERN(void) _MD_InitLocks(void);
#define _MD_INIT_LOCKS _MD_InitLocks

PR_EXTERN(void) _MD_CleanThread(struct PRThread *thread);
#define _MD_CLEAN_THREAD _MD_CleanThread

#define _MD_INIT_PRIMORDIAL_THREAD(threadp)

PR_EXTERN(PRStatus) _MD_CreateThread(
                        struct PRThread *thread,
                        void (*start) (void *),
                        PRThreadPriority priority,
                        PRThreadScope scope,
                        PRThreadState state,
                        PRUint32 stackSize);
#define _MD_CREATE_THREAD _MD_CreateThread

extern void _MD_CleanupBeforeExit(void);
#define _MD_CLEANUP_BEFORE_EXIT _MD_CleanupBeforeExit

PR_EXTERN(void) _MD_InitRunningCPU(struct _PRCPU *cpu);
#define    _MD_INIT_RUNNING_CPU _MD_InitRunningCPU

/* The _PR_MD_WAIT_LOCK and _PR_MD_WAKEUP_WAITER functions put to sleep and
 * awaken a thread which is waiting on a lock or cvar.
 */
PR_EXTERN(PRStatus) _MD_wait(struct PRThread *, PRIntervalTime timeout);
#define _MD_WAIT _MD_wait

PR_EXTERN(PRStatus) _MD_WakeupWaiter(struct PRThread *);
#define _MD_WAKEUP_WAITER _MD_WakeupWaiter

PR_EXTERN(void) _MD_SetPriority(struct _MDThread *thread,
	PRThreadPriority newPri);
#define _MD_SET_PRIORITY _MD_SetPriority

#endif /* PTHREADS_USER */

#endif /* nspr_pthread_defs_h___ */
