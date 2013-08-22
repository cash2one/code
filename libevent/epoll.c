/*
 * Copyright 2000-2003 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <sys/types.h>
#include <sys/resource.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <sys/_libevent_time.h>
#endif
#include <sys/queue.h>
#include <sys/epoll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include "event.h"
#include "event-internal.h"
#include "evsignal.h"
#include "log.h"

/* due to limitations in the epoll interface, we need to keep track of
 * all file descriptors outself.
 */
struct evepoll {
	struct event *evread;
	struct event *evwrite;
};

/* zark: 该结构体维护单个epoll轮询监听的参数信息以及一些运行时的数据.
 *
 */
struct epollop {
	struct evepoll *fds;                //! zark: 一维数组, 以文件描述符(fd)为索引, 存放相应的evepoll, 当fd超出 nfds 时, 该数组将会成倍运算,如达标则扩张(则分配策略实乃霸气).
	int nfds;                           //! zark: 已分配的fd个数, 初始化时为IINITIAL_NFILES(注: 是已分配的, 而不是已使用的).
	struct epoll_event *events;         //! zark: 指向libevent为epoll_event分配的内存块(初始化时为INITIAL_NEVENTS个epoll_event).
	int nevents;                        //! zark: 已分配的epoll_event的数量(初始化时为INITIAL_NEVENTS).
	int epfd;
};

/* zarK: libevent对不同平台的I/O 模型的接口进行抽象, 以下是epoll的抽象接口函数指针.
 *
 */
static void *epoll_init	(struct event_base *);
static int epoll_add	(void *, struct event *);
static int epoll_del	(void *, struct event *);
static int epoll_dispatch	(struct event_base *, void *, struct timeval *);
static void epoll_dealloc	(struct event_base *, void *);

const struct eventop epollops = {
	"epoll",
	epoll_init,
	epoll_add,
	epoll_del,
	epoll_dispatch,
	epoll_dealloc,
	1 /* need reinit */
};

#ifdef HAVE_SETFD
#define FD_CLOSEONEXEC(x) do { \
        if (fcntl(x, F_SETFD, 1) == -1) \
                event_warn("fcntl(%d, F_SETFD)", x); \
} while (0)
#else
#define FD_CLOSEONEXEC(x)
#endif

/* On Linux kernels at least up to 2.6.24.4, epoll can't handle timeout
 * values bigger than (LONG_MAX - 999ULL)/HZ.  HZ in the wild can be
 * as big as 1000, and LONG_MAX can be as small as (1<<31)-1, so the
 * largest number of msec we can support here is 2147482.  Let's
 * round that down by 47 seconds.
 */
#define MAX_EPOLL_TIMEOUT_MSEC (35*60*1000)

#define INITIAL_NFILES 32
#define INITIAL_NEVENTS 32
#define MAX_NEVENTS 4096

/* zark :使用libevent之前必须先初始化, 上层的init接口会调用epoll_init, 来初始化epoll(一些epoll需要的参数).
 *
 */
static void *
epoll_init(struct event_base *base)
{
	int epfd;
	struct epollop *epollop;

	/* Disable epollueue when this environment variable is set */
	if (evutil_getenv("EVENT_NOEPOLL"))     //! 检测是否启用epoll模型.
		return (NULL);

	/* Initalize the kernel queue */
    //! zark: 32000参数对linux 2.6 内核之后的版本已经无效.
	if ((epfd = epoll_create(32000)) == -1) {
		if (errno != ENOSYS)
			event_warn("epoll_create");
		return (NULL);
	}

    //! zark: 如果进程进行exec, 进程在数据拷贝时, 会将该epfd关闭, 也就是说, exec之后, 该epfd会失效.
	FD_CLOSEONEXEC(epfd);

	if (!(epollop = calloc(1, sizeof(struct epollop))))
		return (NULL);

	epollop->epfd = epfd;

    //! zark: 对指向epoll_event的指针进行内存快分配.
	/* Initalize fields */
	epollop->events = malloc(INITIAL_NEVENTS * sizeof(struct epoll_event));
	if (epollop->events == NULL) {
		free(epollop);
		return (NULL);
	}
	epollop->nevents = INITIAL_NEVENTS;

    //! zark: 对指向evepoll的指针进行内存快分配.
	epollop->fds = calloc(INITIAL_NFILES, sizeof(struct evepoll));
	if (epollop->fds == NULL) {
		free(epollop->events);
		free(epollop);
		return (NULL);
	}
	epollop->nfds = INITIAL_NFILES;

    //! 初始化信号
	evsignal_init(base);

	return (epollop);
}

/* zark: 当新添加事件的fd大于nfds时, 调用该方法进行内存成倍扩张.
 *
 */
static int
epoll_recalc(struct event_base *base, void *arg, int max)
{
	struct epollop *epollop = arg;

    //! zark: 如果max大于等于当前所能容纳的大小
    //!       开始扩张内存.
	if (max >= epollop->nfds) {
		struct evepoll *fds;
		int nfds;

		nfds = epollop->nfds;

        //! zark: 对evepoll进行成倍扩展操作, 直至满足需要的大小.
		while (nfds <= max)
			nfds <<= 1;

        //! zark: 开始进行重新分配.
		fds = realloc(epollop->fds, nfds * sizeof(struct evepoll));
		if (fds == NULL) {
			event_warn("realloc");
			return (-1);
		}
		epollop->fds = fds;

        //! zark: 偏移到新申请出的evepoll的内存地址进行初始化.
		memset(fds + epollop->nfds, 0,
		    (nfds - epollop->nfds) * sizeof(struct evepoll));
        //! zark: 更新新的count到epollop
		epollop->nfds = nfds;
	}

	return (0);
}

/* zark: 主逻辑循环处理函数.
 *
 * param:
 *      base - 所属的event_base
 *      arg  - 外部传入的epollop指针
 *      tv   - 超时时间
 */
static int
epoll_dispatch(struct event_base *base, void *arg, struct timeval *tv)
{
	struct epollop *epollop = arg;
	struct epoll_event *events = epollop->events;
	struct evepoll *evep;
	int i, res, timeout = -1;

    //! zark: 将tv转化为毫秒单位.
	if (tv != NULL)
		timeout = tv->tv_sec * 1000 + (tv->tv_usec + 999) / 1000;

    //! zark: 超时最大上限检测, 目前版本epoll支持的最大超时时间为 35 * 60 * 1000 毫秒.
	if (timeout > MAX_EPOLL_TIMEOUT_MSEC) {
		/* Linux kernels can wait forever if the timeout is too big;
		 * see comment on MAX_EPOLL_TIMEOUT_MSEC. */
		timeout = MAX_EPOLL_TIMEOUT_MSEC;
	}

	res = epoll_wait(epollop->epfd, events, epollop->nevents, timeout);

    //! zark: epoll_wait 非正常返回值处理.
	if (res == -1) {
		if (errno != EINTR) {
			event_warn("epoll_wait");
			return (-1);
		}

        //! zark: 错误码为EINTR, 进行信号处理.
		evsignal_process(base);
		return (0);
	} else if (base->sig.evsignal_caught) { //! zark: libevent 捕捉到非中断信号, 进行处理.
		evsignal_process(base);
	}

	event_debug(("%s: epoll_wait reports %d", __func__, res));

    //! zark: epoll准循环.
	for (i = 0; i < res; i++) {
		int what = events[i].events;                    //! zark: 当前循环epoll事件类型.
		struct event *evread = NULL, *evwrite = NULL;
		int fd = events[i].data.fd;                     //! zark: 当前循环epoll需要处理的fd.

		if (fd < 0 || fd >= epollop->nfds)              //! zark: 非正常边界, 本人觉得并不会发生(除非使用十分不当).
			continue;
		evep = &epollop->fds[fd];                       //! zark: 通过索引的方式取得该fd所对应的evepoll.

        /* zark小贴士: EPOLLPRI - 表示对应的文件描述符有紧急的数据可读(这里应该表示有带外数据到来).
         *             EPOLLERR - 表示对应的文件描述符发生错误.
         *             EPOLLHUP - 表示对应的文件描述符被挂断.
         */
		if (what & (EPOLLHUP|EPOLLERR)) {               //! zark: 对于该fd的错误现象进行处理, 进行回调.
			evread = evep->evread;
			evwrite = evep->evwrite;
		} else {
			if (what & EPOLLIN) {                       //! zark: 读事件, 取得读event.
				evread = evep->evread;
			}

			if (what & EPOLLOUT) {                      //! zark: 写事件, 取得写event.
				evwrite = evep->evwrite;
			}
		}

		if (!(evread||evwrite))                         //! zark: 如果evread, evwrite均为空, 则继续循环.
			continue;

		if (evread != NULL)                             //! zark: 如果evread不为空, 添加到libevent的active队列, 外循环将会回调evread中的callback func.
			event_active(evread, EV_READ, 1);
		if (evwrite != NULL)                            //! zark: 如果evwrite不为空, 添加到libevent的active队列, 外循环将会回调evwrite中的callback func.
			event_active(evwrite, EV_WRITE, 1);
	}

    //! zark: 如果epoll_wait返回的fd数量等于我们的nevents, 我们将进行扩展, 这样的话通过一次epoll_wait我们能获得更多的fd, 减少内核线程的切换, 提高性能.
	if (res == epollop->nevents && epollop->nevents < MAX_NEVENTS) {
		/* We used all of the event space this time.  We should
		   be ready for more events next time. */
		int new_nevents = epollop->nevents * 2;         //! zark: 成倍扩张.
		struct epoll_event *new_events;

        //! zark: 重新内存分配.
		new_events = realloc(epollop->events,
		    new_nevents * sizeof(struct epoll_event));
		if (new_events) {
			epollop->events = new_events;
			epollop->nevents = new_nevents;
		}
	}

	return (0);
}


/* zark: epoll事件添加.
 *
 * param:
 *      arg  - 外部传入的epollop指针
 *      ev   - 要添加的事件.
 */
static int
epoll_add(void *arg, struct event *ev)
{
	struct epollop *epollop = arg;
	struct epoll_event epev = {0, {0}};
	struct evepoll *evep;
	int fd, op, events;

	if (ev->ev_events & EV_SIGNAL)          //! zark: 如果要添加的是信号事件, 直接添加并返回.
		return (evsignal_add(ev));

	fd = ev->ev_fd;
	if (fd >= epollop->nfds) {              //! zark: 如果新添加的fd大于我们之前所分配的索引数组fds, 那么进行内存扩展.
		/* Extent the file descriptor array as necessary */
		if (epoll_recalc(ev->ev_base, epollop, fd) == -1)
			return (-1);
	}
	evep = &epollop->fds[fd];               //! zark: 索引到对应的那个evepoll, 该evepoll有可能刚刚新分配出来的, 也可能是之前就存在, 有他自己状态的evepoll.
	op = EPOLL_CTL_ADD;                     //! zark: 先赋值为epoll_add操作, 个人认为该行放到mod操作判断的代码下面能更好理解.
	events = 0;

    //! zark: 该索引到的read event已存在, 改为mod操作.
	if (evep->evread != NULL) {
		events |= EPOLLIN;
		op = EPOLL_CTL_MOD;
	}

    //! zark: 该索引到的write event已存在, 改为mod操作.
	if (evep->evwrite != NULL) {
		events |= EPOLLOUT;
		op = EPOLL_CTL_MOD;
	}

    //! zark: 将 新增/修改 的监听事件注入到epoll中.
	if (ev->ev_events & EV_READ)
		events |= EPOLLIN;
	if (ev->ev_events & EV_WRITE)
		events |= EPOLLOUT;

	epev.data.fd = fd;
	epev.events = events;
	if (epoll_ctl(epollop->epfd, op, ev->ev_fd, &epev) == -1)
			return (-1);

    //! zark: 将该evepoll的回调event设为添加的event, 以便epoll I/O 到来时触发回调event中的callback.
	/* Update events responsible */
	if (ev->ev_events & EV_READ)
		evep->evread = ev;
	if (ev->ev_events & EV_WRITE)
		evep->evwrite = ev;

	return (0);
}

/* zark: epoll事件删除.
 *
 * param:
 *      arg  - 外部传入的epollop指针
 *      ev   - 要删除的事件.
 */
static int
epoll_del(void *arg, struct event *ev)
{
	struct epollop *epollop = arg;
	struct epoll_event epev = {0, {0}};
	struct evepoll *evep;
	int fd, events, op;
	int needwritedelete = 1, needreaddelete = 1;

	if (ev->ev_events & EV_SIGNAL)
		return (evsignal_del(ev));

	fd = ev->ev_fd;
	if (fd >= epollop->nfds)
		return (0);
	evep = &epollop->fds[fd];

	op = EPOLL_CTL_DEL;
	events = 0;

	if (ev->ev_events & EV_READ)
		events |= EPOLLIN;
	if (ev->ev_events & EV_WRITE)
		events |= EPOLLOUT;

	if ((events & (EPOLLIN|EPOLLOUT)) != (EPOLLIN|EPOLLOUT)) {      //! zark: 如果该event并没有同时监听 读/写, 之后将会直接从epoll中删除该fd监听, 并将evepoll结构置空.
		if ((events & EPOLLIN) && evep->evwrite != NULL) {          //! zark: 如果是读事件.
			needwritedelete = 0;
			events = EPOLLOUT;
			op = EPOLL_CTL_MOD;
		} else if ((events & EPOLLOUT) && evep->evread != NULL) {   //! zark: 如果是写事件.
			needreaddelete = 0;
			events = EPOLLIN;
			op = EPOLL_CTL_MOD;
		}
	}

	epev.events = events;
	epev.data.fd = fd;

    //! zark: 将需要置空的event赋为NULL, 以供以后该fd值重新注册event.
	if (needreaddelete)
		evep->evread = NULL;
	if (needwritedelete)
		evep->evwrite = NULL;

	if (epoll_ctl(epollop->epfd, op, fd, &epev) == -1)
		return (-1);

	return (0);
}

/* zark: 释放epoll资源.
 *
 * param:
 *      base - 对应的event_base指针
 *      arg  - 外部传入的epollop指针
 */
static void
epoll_dealloc(struct event_base *base, void *arg)
{
	struct epollop *epollop = arg;

	evsignal_dealloc(base);
	if (epollop->fds)
		free(epollop->fds);
	if (epollop->events)
		free(epollop->events);
	if (epollop->epfd >= 0)
		close(epollop->epfd);

	memset(epollop, 0, sizeof(struct epollop));
	free(epollop);
}
