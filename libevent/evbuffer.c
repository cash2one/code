/*
 * Copyright (c) 2002-2004 Niels Provos <provos@citi.umich.edu>
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

#include <sys/types.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef WIN32
#include <winsock2.h>
#endif

#include "evutil.h"
#include "event.h"

/* prototypes */

void bufferevent_read_pressure_cb(struct evbuffer *, size_t, size_t, void *);

static int
bufferevent_add(struct event *ev, int timeout)
{
	struct timeval tv, *ptv = NULL;

	if (timeout) {
		evutil_timerclear(&tv);
		tv.tv_sec = timeout;
		ptv = &tv;
	}

	return (event_add(ev, ptv));
}

/* 
 * This callback is executed when the size of the input buffer changes.
 * We use it to apply back pressure on the reading side.
 *
 * zark: 输入缓冲区大小改变时会被调用,在下方的
 *       bufferevent_readcb函数中会有所解释该函
 *       数的作用.
 */
void
bufferevent_read_pressure_cb(struct evbuffer *buf, size_t old, size_t now,
    void *arg) {
	struct bufferevent *bufev = arg;
	/* 
	 * If we are below the watermark then reschedule reading if it's
	 * still enabled.
	 */
	if (bufev->wm_read.high == 0 || now < bufev->wm_read.high) {
		evbuffer_setcb(buf, NULL, NULL);

		if (bufev->enabled & EV_READ)
			bufferevent_add(&bufev->ev_read, bufev->timeout_read);
	}
}

/*
 * zark: 当epoll层的读事件到来, 会从活跃队列中回调该函数,
 *       开始从socket中读取数据.
 */
static void
bufferevent_readcb(int fd, short event, void *arg)
{
	struct bufferevent *bufev = arg;
	int res = 0;
	short what = EVBUFFER_READ;
	size_t len;
	int howmuch = -1;

    //! zark: bufferevent是对I/O操作的封装, 所以不监听超时事件.
	if (event == EV_TIMEOUT) {
		what |= EVBUFFER_TIMEOUT;
		goto error;
	}

	/*
	 * If we have a high watermark configured then we don't want to
	 * read more data than would make us reach the watermark.
     *
     * zark: 这里有必要普及一下libevent关于输入输出时的水位概念.
     *      读取低水位 - 读取操作使得输入缓冲区的数据量在此级别或者更高时,
     *                   读取回调将被调用.默认值为0, 所以每个读取操作都会
     *                   导致读取回调被调用.
     *      读取高水位 - 输入缓冲区中的数据量达到此级别后，bufferevent将停
     *                   止读取，直到输入缓冲区中足够量的数据被抽取，使得数
     *                   据量低于此级别.默认值是无限, 所以永远不会因为输入
     *                   缓冲区的大小而停止读取.
     *      写入低水位 - 写入操作使得输出缓冲区的数据量达到或者低于此级别时,
     *                   写入回调将被调用.默认值是0, 所以只有输出缓冲区空的
     *                   时候才会调用写入回调.
     *      写入高水位 - bufferevent没有直接使用这个水位. 它在bufferevent用
     *                   作另外一个bufferevent的底层传输端口时有特殊意义.请
     *                   看后面关于过滤型bufferevent的介绍.
	 */

    //! zark: 读取高水位到达, 停止读取.
	if (bufev->wm_read.high != 0) {
		howmuch = bufev->wm_read.high - EVBUFFER_LENGTH(bufev->input);
		/* we might have lowered the watermark, stop reading */
		if (howmuch <= 0) {
			struct evbuffer *buf = bufev->input;
			event_del(&bufev->ev_read); //! 删除当前读事件.
            /*
             * zark: 设置一个新的读事件, 该读事件的callback函
             *       数(即bufferevent_read_pressure_cb)会检测
             *       当输入冲区大小小于高水位时, 会再次添加正
             *       常的读取event.
             */
			evbuffer_setcb(buf,
			    bufferevent_read_pressure_cb, bufev);
			return;
		}
	}

    /*
     * zark: 开始从fd读取数据到我们的输入缓冲区中.
     */
	res = evbuffer_read(bufev->input, fd, howmuch);
	if (res == -1) {
		if (errno == EAGAIN || errno == EINTR)
			goto reschedule;
		/* error case */
		what |= EVBUFFER_ERROR;
	} else if (res == 0) {
		/* eof case */
		what |= EVBUFFER_EOF;
	}

	if (res <= 0)
		goto error;

    /*
     * zark: 因为bufferevent当初添加事件的时候没有使用
     *       persist来修饰event, 所以需要重新添加.
     */
	bufferevent_add(&bufev->ev_read, bufev->timeout_read);

	/* See if this callbacks meets the water marks */
	len = EVBUFFER_LENGTH(bufev->input);

    /*
     * zark: 读取低水位线没有达到, 所以不能调用callback, 直接return~
     */
	if (bufev->wm_read.low != 0 && len < bufev->wm_read.low)
		return;

    /*
     * zark: 读取高水位到达, 停止读取.
     */
	if (bufev->wm_read.high != 0 && len >= bufev->wm_read.high) {
		struct evbuffer *buf = bufev->input;
		event_del(&bufev->ev_read);

		/* Now schedule a callback for us when the buffer changes */
		evbuffer_setcb(buf, bufferevent_read_pressure_cb, bufev);
	}

	/* Invoke the user callback - must always be called last */
    /*
     * zark: 哈哈, 一顿乱七八糟的check下来, 终于到了调用我们
     *       真正注册的读取回调函数拉~ 散花~
     */
	if (bufev->readcb != NULL)
		(*bufev->readcb)(bufev, bufev->cbarg);
	return;

 reschedule:
	bufferevent_add(&bufev->ev_read, bufev->timeout_read);
	return;

    /*
     * zark: 调用我们注册的错误处理回调函数.
     */
 error:
	(*bufev->errorcb)(bufev, what, bufev->cbarg);
}

/*
 * zark: 当数据传输完毕, 并符合回调水印时, 该函数被调用.
 *       和bufferevent_readcb类似, 我就不多加熬述了.
 */
static void
bufferevent_writecb(int fd, short event, void *arg)
{
	struct bufferevent *bufev = arg;
	int res = 0;
	short what = EVBUFFER_WRITE;

	if (event == EV_TIMEOUT) {
		what |= EVBUFFER_TIMEOUT;
		goto error;
	}

	if (EVBUFFER_LENGTH(bufev->output)) {
	    res = evbuffer_write(bufev->output, fd);
	    if (res == -1) {
#ifndef WIN32
/*todo. evbuffer uses WriteFile when WIN32 is set. WIN32 system calls do not
 *set errno. thus this error checking is not portable*/
		    if (errno == EAGAIN ||
			errno == EINTR ||
			errno == EINPROGRESS)
			    goto reschedule;
		    /* error case */
		    what |= EVBUFFER_ERROR;

#else
				goto reschedule;
#endif

	    } else if (res == 0) {
		    /* eof case */
		    what |= EVBUFFER_EOF;
	    }
	    if (res <= 0)
		    goto error;
	}

	if (EVBUFFER_LENGTH(bufev->output) != 0)
		bufferevent_add(&bufev->ev_write, bufev->timeout_write);

	/*
	 * Invoke the user callback if our buffer is drained or below the
	 * low watermark.
	 */
	if (bufev->writecb != NULL &&
	    EVBUFFER_LENGTH(bufev->output) <= bufev->wm_write.low)
		(*bufev->writecb)(bufev, bufev->cbarg);

	return;

 reschedule:
	if (EVBUFFER_LENGTH(bufev->output) != 0)
		bufferevent_add(&bufev->ev_write, bufev->timeout_write);
	return;

 error:
	(*bufev->errorcb)(bufev, what, bufev->cbarg);
}

/*
 * Create a new buffered event object.
 *
 * The read callback is invoked whenever we read new data.
 * The write callback is invoked whenever the output buffer is drained.
 * The error callback is invoked on a write/read error or on EOF.
 *
 * Both read and write callbacks maybe NULL.  The error callback is not
 * allowed to be NULL and have to be provided always.
 */
struct bufferevent *
bufferevent_new(int fd, evbuffercb readcb, evbuffercb writecb,
    everrorcb errorcb, void *cbarg)
{
	struct bufferevent *bufev;

    // zark: 分配一个bufferevent, 内部为用户封装了event读写事件, 和I/O读写操作.
	if ((bufev = calloc(1, sizeof(struct bufferevent))) == NULL)
		return (NULL);

    //! zark: 分配输入缓冲区内存.
	if ((bufev->input = evbuffer_new()) == NULL) {
		free(bufev);
		return (NULL);
	}

    //! zark: 分配输出缓冲区内存.
	if ((bufev->output = evbuffer_new()) == NULL) {
		evbuffer_free(bufev->input);
		free(bufev);
		return (NULL);
	}

    /*
     * zark: bufferevent内部维护了两个event(读和写).
     *       当这两个event触发时, 会调用bufferevent_readcb
     *       和bufferevent_writecb, 这两个函数在处理完之后
     *       会分别调用用户注册进来的readcb和writecb.
     *
     *       注: 这边只是set读写事件结构的信息, 并没有add到
     *       底层的监听系统中去, 所以add操作要另外通过一个enabled
     *       选项来控制, 而EV_WRITE是默认打开的, 在下方代码就
     *       能看到, 而EV_READ需要用户手动去打开. 通过
     *       bufferevent_enable, bufferevent_disable 两个方法
     *       来开启和禁用.
     *       bufferevent_enable, bufferevent_disable其实就是调用
     *       event_add, event_del来添加监听和删除监听的.
     *       事件.
     */
	event_set(&bufev->ev_read, fd, EV_READ, bufferevent_readcb, bufev);
	event_set(&bufev->ev_write, fd, EV_WRITE, bufferevent_writecb, bufev);

    //! zark: 将用户注册的回调函数注册进去.
	bufferevent_setcb(bufev, readcb, writecb, errorcb, cbarg);

	/*
	 * Set to EV_WRITE so that using bufferevent_write is going to
	 * trigger a callback.  Reading needs to be explicitly enabled
	 * because otherwise no data will be available.
	 */
    /*
     * zark: 就像之前所说的, 写事件的监听是默认打开的, 理由是开放
     *       了bufferevent_write接口, 如果不默认打开这个选项, 就不
     *       能正常的将buffer里的数据通过tcp传输初期, 这样做岂不是
     *       会很怪? 而读事件的监听需要用户手动来开启, 否则即使有
     *       数据到来, libevent也不会调用用户的读回调函数.
     */
	bufev->enabled = EV_WRITE;

	return (bufev);
}

/*
 * zark: 设置用户的回调函数.
 *   params:
 *      readcb  - 读事件回调.
 *      writecb - 写事件回调.
 *      errorcb - 错误发生时的回调.
 *      cbarg   - 回调函数的参数.
 */
void
bufferevent_setcb(struct bufferevent *bufev,
    evbuffercb readcb, evbuffercb writecb, everrorcb errorcb, void *cbarg)
{
	bufev->readcb = readcb;
	bufev->writecb = writecb;
	bufev->errorcb = errorcb;

	bufev->cbarg = cbarg;
}

void
bufferevent_setfd(struct bufferevent *bufev, int fd)
{
	event_del(&bufev->ev_read);
	event_del(&bufev->ev_write);

	event_set(&bufev->ev_read, fd, EV_READ, bufferevent_readcb, bufev);
	event_set(&bufev->ev_write, fd, EV_WRITE, bufferevent_writecb, bufev);
	if (bufev->ev_base != NULL) {
		event_base_set(bufev->ev_base, &bufev->ev_read);
		event_base_set(bufev->ev_base, &bufev->ev_write);
	}

	/* might have to manually trigger event registration */
}

/*
 * zark: 设置该bufferevent的读/写事件的优先级.
 *       关于event的优先级, 如果你不清楚的话可
 *       以去看event.c中对active queue的处理.
 */
int
bufferevent_priority_set(struct bufferevent *bufev, int priority)
{
	if (event_priority_set(&bufev->ev_read, priority) == -1)
		return (-1);
	if (event_priority_set(&bufev->ev_write, priority) == -1)
		return (-1);

	return (0);
}

/*
 * zark: 资源释放函数, 没神马好讲的哈~
 */
/* Closing the file descriptor is the responsibility of the caller */
void
bufferevent_free(struct bufferevent *bufev)
{
	event_del(&bufev->ev_read);
	event_del(&bufev->ev_write);

	evbuffer_free(bufev->input);
	evbuffer_free(bufev->output);

	free(bufev);
}

/*
 * Returns 0 on success;
 *        -1 on failure.
 *
 * zark: 用户一般会在外部调用这个接口, 也就是像该管道传送数据.
 *       简单的流程是将data放到该bufferevent的写缓冲中, 当epoll
 *       层对该bufferevent所持的fd的写事件响应时, 会将写缓冲中
 *       的数据send出去, 顺便吐槽下libevent的缓冲区设计得比较挫,
 *       在2.0版本中libevent会提供接口供用户在外界提供自己的内存
 *       池方法.
 *       注: 这个版本的bufferevent的所有方法都是线程不安全的!
 *       所以真正要使用的话, 要么使用2.0线程安全的bufferevent, 否
 *       则要做多线程操作的话只能自己做封装加锁了.
 */
int
bufferevent_write(struct bufferevent *bufev, const void *data, size_t size)
{
	int res;

    //! zark: 将数据存放到输出缓存中去.
	res = evbuffer_add(bufev->output, data, size);

	if (res == -1)
		return (res);

	/* If everything is okay, we need to schedule a write */
    /*
     * zark: 驱动一下I/O模型对该写事件的监听, 否则永远不会将input_buffer
     *       中的数据发送出去.
     */
	if (size > 0 && (bufev->enabled & EV_WRITE))
		bufferevent_add(&bufev->ev_write, bufev->timeout_write);

	return (res);
}

int
bufferevent_write_buffer(struct bufferevent *bufev, struct evbuffer *buf)
{
	int res;

	res = bufferevent_write(bufev, buf->buffer, buf->off);
	if (res != -1)
		evbuffer_drain(buf, buf->off);

	return (res);
}

size_t
bufferevent_read(struct bufferevent *bufev, void *data, size_t size)
{
	struct evbuffer *buf = bufev->input;

	if (buf->off < size)
		size = buf->off;

	/* Copy the available data to the user buffer */
	memcpy(data, buf->buffer, size);

	if (size)
		evbuffer_drain(buf, size);

	return (size);
}

int
bufferevent_enable(struct bufferevent *bufev, short event)
{
	if (event & EV_READ) {
		if (bufferevent_add(&bufev->ev_read, bufev->timeout_read) == -1)
			return (-1);
	}
	if (event & EV_WRITE) {
		if (bufferevent_add(&bufev->ev_write, bufev->timeout_write) == -1)
			return (-1);
	}

	bufev->enabled |= event;
	return (0);
}

int
bufferevent_disable(struct bufferevent *bufev, short event)
{
	if (event & EV_READ) {
		if (event_del(&bufev->ev_read) == -1)
			return (-1);
	}
	if (event & EV_WRITE) {
		if (event_del(&bufev->ev_write) == -1)
			return (-1);
	}

	bufev->enabled &= ~event;
	return (0);
}

/*
 * Sets the read and write timeout for a buffered event.
 */

void
bufferevent_settimeout(struct bufferevent *bufev,
    int timeout_read, int timeout_write) {
	bufev->timeout_read = timeout_read;
	bufev->timeout_write = timeout_write;

	if (event_pending(&bufev->ev_read, EV_READ, NULL))
		bufferevent_add(&bufev->ev_read, timeout_read);
	if (event_pending(&bufev->ev_write, EV_WRITE, NULL))
		bufferevent_add(&bufev->ev_write, timeout_write);
}

/*
 * Sets the water marks
 */

void
bufferevent_setwatermark(struct bufferevent *bufev, short events,
    size_t lowmark, size_t highmark)
{
	if (events & EV_READ) {
		bufev->wm_read.low = lowmark;
		bufev->wm_read.high = highmark;
	}

	if (events & EV_WRITE) {
		bufev->wm_write.low = lowmark;
		bufev->wm_write.high = highmark;
	}

	/* If the watermarks changed then see if we should call read again */
	bufferevent_read_pressure_cb(bufev->input,
	    0, EVBUFFER_LENGTH(bufev->input), bufev);
}

int
bufferevent_base_set(struct event_base *base, struct bufferevent *bufev)
{
	int res;

	bufev->ev_base = base;

	res = event_base_set(base, &bufev->ev_read);
	if (res == -1)
		return (res);

	res = event_base_set(base, &bufev->ev_write);
	return (res);
}
