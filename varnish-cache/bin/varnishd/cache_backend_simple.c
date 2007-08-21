/*-
 * Copyright (c) 2006 Verdens Gang AS
 * Copyright (c) 2006-2007 Linpro AS
 * All rights reserved.
 *
 * Author: Poul-Henning Kamp <phk@phk.freebsd.dk>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 *
 *
 * XXX: When we switch VCL we can have vbe_conn's dangling from
 * XXX: the backends no longer used.  When the VCL's refcount
 * XXX: drops to zero we should zap them.
 */

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>

#include "shmlog.h"
#include "cache.h"
#include "vrt.h"

static TAILQ_HEAD(,vbe_conn) vbe_head = TAILQ_HEAD_INITIALIZER(vbe_head);

static MTX besmtx;

struct bes {
	unsigned		magic;
#define BES_MAGIC		0x015e17ac
	char			*hostname;
	char			*portname;
	struct addrinfo		*addr;
	struct addrinfo		*last_addr;
	double			dnsttl;
	double			dnstime;
	TAILQ_HEAD(, vbe_conn)	connlist;
};

/*--------------------------------------------------------------------*/

static struct vbe_conn *
bes_new_conn(void)
{
	struct vbe_conn *vbc;

	vbc = calloc(sizeof *vbc, 1);
	if (vbc == NULL)
		return (NULL);
	VSL_stats->n_vbe_conn++;
	vbc->magic = VBE_CONN_MAGIC;
	vbc->fd = -1;
	return (vbc);
}

/*--------------------------------------------------------------------
 * XXX: There is a race here, we need to lock the replacement of the
 * XXX: resolved addresses, or some other thread might try to access
 * XXX: them while/during/after we changed them.
 * XXX: preferably, we should make a copy to the vbe while we hold a
 * XXX: lock anyway.
 */

static void
bes_lookup(struct backend *bp)
{
	struct addrinfo *res, hint, *old;
	int error;
	struct bes *bes;

	CAST_OBJ_NOTNULL(bes, bp->priv, BES_MAGIC);

	memset(&hint, 0, sizeof hint);
	hint.ai_family = PF_UNSPEC;
	hint.ai_socktype = SOCK_STREAM;
	res = NULL;
	error = getaddrinfo(bes->hostname,
	    bes->portname == NULL ? "http" : bes->portname,
	    &hint, &res);
	bes->dnstime = TIM_mono();
	if (error) {
		if (res != NULL)
			freeaddrinfo(res);
		printf("getaddrinfo: %s\n", gai_strerror(error)); /* XXX */
		return;
	}
	old = bes->addr;
	bes->last_addr = res;
	bes->addr = res;
	if (old != NULL)
		freeaddrinfo(old);
}

/*--------------------------------------------------------------------*/

static int
bes_sock_conn(const struct addrinfo *ai)
{
	int s;

	s = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if (s >= 0 && connect(s, ai->ai_addr, ai->ai_addrlen)) {
		AZ(close(s));
		s = -1;
	}
	return (s);
}

/*--------------------------------------------------------------------*/

static int
bes_conn_try(struct backend *bp, struct addrinfo **pai)
{
	struct addrinfo *ai;
	int s;
	struct bes *bes;

	CAST_OBJ_NOTNULL(bes, bp->priv, BES_MAGIC);

	/* First try the cached good address, and any following it */
	for (ai = bes->last_addr; ai != NULL; ai = ai->ai_next) {
		s = bes_sock_conn(ai);
		if (s >= 0) {
			bes->last_addr = ai;
			*pai = ai;
			return (s);
		}
	}

	/* Then try the list until the cached last good address */
	for (ai = bes->addr; ai != bes->last_addr; ai = ai->ai_next) {
		s = bes_sock_conn(ai);
		if (s >= 0) {
			bes->last_addr = ai;
			*pai = ai;
			return (s);
		}
	}

	if (bes->dnstime + bes->dnsttl >= TIM_mono())
		return (-1);

	/* Then do another lookup to catch DNS changes */
	bes_lookup(bp);

	/* And try the entire list */
	for (ai = bes->addr; ai != NULL; ai = ai->ai_next) {
		s = bes_sock_conn(ai);
		if (s >= 0) {
			bes->last_addr = ai;
			*pai = ai;
			return (s);
		}
	}

	return (-1);
}

static int
bes_connect(struct sess *sp, struct backend *bp)
{
	int s;
	char abuf1[TCP_ADDRBUFSIZE], abuf2[TCP_ADDRBUFSIZE];
	char pbuf1[TCP_PORTBUFSIZE], pbuf2[TCP_PORTBUFSIZE];
	struct addrinfo *ai;
	struct bes *bes;


	CHECK_OBJ_NOTNULL(bp, BACKEND_MAGIC);
	CAST_OBJ_NOTNULL(bes, bp->priv, BES_MAGIC);
	AN(bes->hostname);

	s = bes_conn_try(bp, &ai);
	if (s < 0)
		return (s);

	TCP_myname(s, abuf1, sizeof abuf1, pbuf1, sizeof pbuf1);
	TCP_name(ai->ai_addr, ai->ai_addrlen,
	    abuf2, sizeof abuf2, pbuf2, sizeof pbuf2);
	WSL(sp->wrk, SLT_BackendOpen, s, "%s %s %s %s %s",
	    bp->vcl_name, abuf1, pbuf1, abuf2, pbuf2);
	return (s);
}

/* Get a backend connection ------------------------------------------
 *
 * Try all cached backend connections for this backend, and use the
 * first one that is looks like it is still connected.
 * If that fails to get us a connection, create a new one, reusing a
 * connection from the freelist, if possible.
 *
 * This function is slightly complicated by optimizations on besmtx.
 */

static struct vbe_conn *
bes_nextfd(struct sess *sp)
{
	struct vbe_conn *vc, *vc2;
	struct pollfd pfd;
	struct backend *bp;
	int reuse = 0;
	struct bes *bes;

	CHECK_OBJ_NOTNULL(sp, SESS_MAGIC);
	bp = sp->backend;
	CHECK_OBJ_NOTNULL(bp, BACKEND_MAGIC);
	CAST_OBJ_NOTNULL(bes, bp->priv, BES_MAGIC);
	vc2 = NULL;
	while (1) {
		LOCK(&besmtx);
		vc = TAILQ_FIRST(&bes->connlist);
		if (vc != NULL) {
			assert(vc->backend == bp);
			assert(vc->fd >= 0);
			TAILQ_REMOVE(&bes->connlist, vc, list);
		} else {
			vc2 = TAILQ_FIRST(&vbe_head);
			if (vc2 != NULL) {
				VSL_stats->backend_unused--;
				TAILQ_REMOVE(&vbe_head, vc2, list);
			}
		}
		UNLOCK(&besmtx);
		if (vc == NULL)
			break;

		/* Test the connection for remote close before we use it */
		pfd.fd = vc->fd;
		pfd.events = POLLIN;
		pfd.revents = 0;
		if (!poll(&pfd, 1, 0)) {
			reuse = 1;
			break;
		}
		VBE_ClosedFd(sp->wrk, vc);
	}

	if (vc == NULL) {
		if (vc2 == NULL)
			vc = bes_new_conn();
		else
			vc = vc2;
		if (vc != NULL) {
			assert(vc->fd == -1);
			AZ(vc->backend);
			vc->fd = bes_connect(sp, bp);
			if (vc->fd < 0) {
				LOCK(&besmtx);
				TAILQ_INSERT_HEAD(&vbe_head, vc, list);
				VSL_stats->backend_unused++;
				UNLOCK(&besmtx);
				vc = NULL;
			} else {
				vc->backend = bp;
			}
		}
	}
	LOCK(&besmtx);
	if (vc != NULL ) {
		VSL_stats->backend_reuse += reuse;
		VSL_stats->backend_conn++;
	} else {
		VSL_stats->backend_fail++;
	}
	UNLOCK(&besmtx);
	if (vc != NULL ) {
		WSL(sp->wrk, SLT_BackendXID, vc->fd, "%u", sp->xid);
		assert(vc->fd >= 0);
		assert(vc->backend == bp);
	}
	return (vc);
}

/*--------------------------------------------------------------------*/

static struct vbe_conn *
bes_GetFd(struct sess *sp)
{
	struct vbe_conn *vc;
	unsigned n;

	for (n = 1; n < 5; n++) {
		vc = bes_nextfd(sp);
		if (vc != NULL) {
			WSL(sp->wrk, SLT_Backend, sp->fd, "%d %s", vc->fd,
			    sp->backend->vcl_name);
			return (vc);
		}
		usleep(100000 * n);
	}
	return (NULL);
}

/* Close a connection ------------------------------------------------*/

static void
bes_ClosedFd(struct worker *w, struct vbe_conn *vc)
{

	CHECK_OBJ_NOTNULL(vc, VBE_CONN_MAGIC);
	assert(vc->fd >= 0);
	AN(vc->backend);
	WSL(w, SLT_BackendClose, vc->fd, "%s", vc->backend->vcl_name);
	AZ(close(vc->fd));
	vc->fd = -1;
	vc->backend = NULL;
	LOCK(&besmtx);
	TAILQ_INSERT_HEAD(&vbe_head, vc, list);
	VSL_stats->backend_unused++;
	UNLOCK(&besmtx);
}

/* Recycle a connection ----------------------------------------------*/

static void
bes_RecycleFd(struct worker *w, struct vbe_conn *vc)
{
	struct bes *bes;

	CHECK_OBJ_NOTNULL(vc, VBE_CONN_MAGIC);
	CHECK_OBJ_NOTNULL(vc->backend, BACKEND_MAGIC);
	CAST_OBJ_NOTNULL(bes, vc->backend->priv, BES_MAGIC);

	assert(vc->fd >= 0);
	AN(vc->backend);
	WSL(w, SLT_BackendReuse, vc->fd, "%s", vc->backend->vcl_name);
	LOCK(&besmtx);
	VSL_stats->backend_recycle++;
	TAILQ_INSERT_HEAD(&bes->connlist, vc, list);
	UNLOCK(&besmtx);
}

/*--------------------------------------------------------------------*/

static void
bes_Cleanup(struct backend *b)
{
	struct bes *bes;
	struct vbe_conn *vbe;

	CAST_OBJ_NOTNULL(bes, b->priv, BES_MAGIC);
	free(bes->portname);
	free(bes->hostname);
	freeaddrinfo(bes->addr);
	while (1) {
		vbe = TAILQ_FIRST(&bes->connlist);
		if (vbe == NULL)
			break;
		TAILQ_REMOVE(&bes->connlist, vbe, list);
		if (vbe->fd >= 0)
			close(vbe->fd);
		free(vbe);
	}
	free(bes);
}

/*--------------------------------------------------------------------*/

static const char *
bes_GetHostname(struct backend *b)
{
	struct bes *bes;

	CHECK_OBJ_NOTNULL(b, SESS_MAGIC);
	CAST_OBJ_NOTNULL(bes, b->priv, BES_MAGIC);
	return (bes->hostname);
}

/*--------------------------------------------------------------------*/

static void
bes_Init(void)
{

	MTX_INIT(&besmtx);
}

/*--------------------------------------------------------------------*/

struct backend_method backend_method_simple = {
	.name =			"simple",
	.getfd =		bes_GetFd,
	.close =		bes_ClosedFd,
	.recycle =		bes_RecycleFd,
	.gethostname =		bes_GetHostname,
	.cleanup =		bes_Cleanup,
	.init =			bes_Init
};

/*--------------------------------------------------------------------*/

void
VRT_init_simple_backend(struct backend **bp, struct vrt_simple_backend *t)
{
	struct backend *b;
	struct bes *bes;
	
	/*
	 * Scan existing backends to see if we can recycle one of them.
	 */
	TAILQ_FOREACH(b, &backendlist, list) {
		CHECK_OBJ_NOTNULL(b, BACKEND_MAGIC);
		if (b->method != &backend_method_simple)
			continue;
		if (strcmp(b->vcl_name, t->name))
			continue;
		CAST_OBJ_NOTNULL(bes, b->priv, BES_MAGIC);
		if (strcmp(bes->portname, t->port))
			continue;
		if (strcmp(bes->hostname, t->host))
			continue;
		b->refcount++;
		*bp = b;
		return;
	}

	b = VBE_NewBackend(&backend_method_simple);

	bes = calloc(sizeof *bes, 1);
	XXXAN(bes);
	bes->magic = BES_MAGIC;

	b->priv = bes;

	bes->dnsttl = 300;

	AN(t->name);
	b->vcl_name = strdup(t->name);
	XXXAN(b->vcl_name);

	AN(t->port);
	bes->portname = strdup(t->port);
	XXXAN(bes->portname);

	AN(t->host);
	bes->hostname = strdup(t->host);
	XXXAN(bes->hostname);

	*bp = b;
}