/*
 * $Id$
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netdb.h>

#include <sbuf.h>
#include <event.h>

#include "libvarnish.h"
#include "vcl_lang.h"
#include "heritage.h"
#include "shmlog.h"
#include "cache.h"

static struct event_base *evb;

static struct event accept_e[2 * HERITAGE_NSOCKS];

struct sessmem {
	struct sess	s;
	struct event	e;
};

static void
accept_f(int fd, short event, void *arg)
{
	socklen_t l;
	struct sessmem *sm;
	struct sockaddr addr;
	struct sess *sp;
	char port[10];

	(void)arg;
	sm = calloc(sizeof *sm, 1);
	assert(sm != NULL);	/*
				 * XXX: this is probably one we should handle
				 * XXX: accept, emit error NNN and close
				 */

	sp = &sm->s;
	sp->rd_e = &sm->e;
	sp->mem = sm;

	l = sizeof addr;
	sp->fd = accept(fd, &addr, &l);
	if (sp->fd < 0) {
		free(sp);
		return;
	}
	AZ(getnameinfo(&addr, l,
	    sp->addr, VCA_ADDRBUFSIZE,
	    port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV));
	strlcat(sp->addr, ":", VCA_ADDRBUFSIZE);
	strlcat(sp->addr, port, VCA_ADDRBUFSIZE);
	VSL(SLT_SessionOpen, sp->fd, "%s", sp->addr);
	HttpdGetHead(sp, evb, DealWithSession);
}

void *
vca_main(void *arg)
{
	unsigned u;
	struct event *ep;

	evb = event_init();

	ep = accept_e;
	for (u = 0; u < HERITAGE_NSOCKS; u++) {
		if (heritage.sock_local[u] >= 0) {
			event_set(ep, heritage.sock_local[u],
			    EV_READ | EV_PERSIST,
			    accept_f, NULL);
			event_base_set(evb, ep);
			event_add(ep, NULL);
			ep++;
		}
		if (heritage.sock_remote[u] >= 0) {
			event_set(ep, heritage.sock_remote[u],
			    EV_READ | EV_PERSIST,
			    accept_f, NULL);
			event_base_set(evb, ep);
			event_add(ep, NULL);
			ep++;
		}
	}

	event_base_loop(evb, 0);

	return ("FOOBAR");
}

void
vca_retire_session(struct sess *sp)
{

	if (sp->fd >= 0)
		close(sp->fd);
	free(sp);
}
