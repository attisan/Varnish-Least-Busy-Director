/*-
 * Copyright (c) 2006 Verdens Gang AS
 * Copyright (c) 2006-2008 Linpro AS
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
 */

#include <sys/time.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include <pthread.h>
#ifdef HAVE_PTHREAD_NP_H
#include <pthread_np.h>
#endif
#include <stdint.h>

#include "vqueue.h"

#include "vsb.h"

#include "libvarnish.h"

#include "vcl_returns.h"
#include "common.h"
#include "miniobj.h"

enum {
	/* Fields from the first line of HTTP proto */
	HTTP_HDR_REQ,
	HTTP_HDR_URL,
	HTTP_HDR_PROTO,
	HTTP_HDR_STATUS,
	HTTP_HDR_RESPONSE,
	/* HTTP header lines */
	HTTP_HDR_FIRST,
	HTTP_HDR_MAX = 32		/* XXX: should be #defined */
};

/* Note: intentionally not IOV_MAX unless it has to be */
#if (IOV_MAX < (HTTP_HDR_MAX * 2))
#  define MAX_IOVS	IOV_MAX
#else
#  define MAX_IOVS	(HTTP_HDR_MAX * 2)
#endif

struct cli;
struct vsb;
struct sess;
struct object;
struct objhead;
struct workreq;
struct addrinfo;
struct esi_bit;

/*--------------------------------------------------------------------*/

typedef struct {
	char			*b;
	char			*e;
} txt;

/*--------------------------------------------------------------------*/

enum step {
#define STEP(l, u)	STP_##u,
#include "steps.h"
#undef STEP
};

/*--------------------------------------------------------------------
 * Workspace structure for quick memory allocation.
 */

struct ws {
	unsigned		magic;
#define WS_MAGIC		0x35fac554
	const char		*id;		/* identity */
	char			*s;		/* (S)tart of buffer */
	char			*f;		/* (F)ree pointer */
	char			*r;		/* (R)eserved length */
	char			*e;		/* (E)nd of buffer */
};

/*--------------------------------------------------------------------
 * HTTP Request/Response/Header handling structure.
 */

enum httpwhence {
	HTTP_Rx	 = 1,
	HTTP_Tx  = 2,
	HTTP_Obj = 3
};

struct http {
	unsigned		magic;
#define HTTP_MAGIC		0x6428b5c9

	struct ws		*ws;

	unsigned char		conds;		/* If-* headers present */
	enum httpwhence 	logtag;
	int			status;

	txt 			hd[HTTP_HDR_MAX];
	unsigned char		hdf[HTTP_HDR_MAX];
#define HDF_FILTER		(1 << 0)	/* Filtered by Connection */
	unsigned		nhd;
};

/*--------------------------------------------------------------------
 * HTTP Protocol connection structure
 */

struct http_conn {
	unsigned		magic;
#define HTTP_CONN_MAGIC		0x3e19edd1

	int			fd;
	struct ws		*ws;
	txt			rxbuf;
	txt			pipeline;
};

/*--------------------------------------------------------------------*/

struct acct {
	double			first;
	uint64_t		sess;
	uint64_t		req;
	uint64_t		pipe;
	uint64_t		pass;
	uint64_t		fetch;
	uint64_t		hdrbytes;
	uint64_t		bodybytes;
};

/*--------------------------------------------------------------------*/

struct worker {
	unsigned		magic;
#define WORKER_MAGIC		0x6391adcf
	struct objhead		*nobjhead;
	struct object		*nobj;

	double			used;

	int			pipe[2];

	VTAILQ_ENTRY(worker)	list;
	struct workreq		*wrq;

	int			*wfd;
	unsigned		werr;	/* valid after WRK_Flush() */
	struct iovec		iov[MAX_IOVS];
	int			niov;
	ssize_t			liov;

	struct VCL_conf		*vcl;
	struct srcaddr		*srcaddr;
	struct acct		acct;

	unsigned char		*wlb, *wlp, *wle;
	unsigned		wlr;
};

struct workreq {
	VTAILQ_ENTRY(workreq)	list;
	struct sess		*sess;
};

#include "hash_slinger.h"

/* Backend Request ---------------------------------------------------*/

struct bereq {
	unsigned		magic;
#define BEREQ_MAGIC		0x3b6d250c
	VTAILQ_ENTRY(bereq)	list;
	struct ws		ws[1];
	struct http		http[1];
};

/* Storage -----------------------------------------------------------*/

struct storage {
	unsigned		magic;
#define STORAGE_MAGIC		0x1a4e51c0
	VTAILQ_ENTRY(storage)	list;
	struct stevedore	*stevedore;
	void			*priv;

	unsigned char		*ptr;
	unsigned		len;
	unsigned		space;

	int			fd;
	off_t			where;
};

/* -------------------------------------------------------------------*/
enum e_objtimer {
	TIMER_TTL,
	TIMER_PREFETCH
};

struct object {
	unsigned		magic;
#define OBJECT_MAGIC		0x32851d42
	unsigned 		refcnt;
	unsigned		xid;
	struct objhead		*objhead;
	struct storage		*objstore;

	struct ws		ws_o[1];
	unsigned char		*vary;

	double			timer_when;
	enum e_objtimer		timer_what;
	unsigned		timer_idx;

	unsigned		ban_seq;

	unsigned		pass;

	unsigned		response;

	unsigned		valid;
	unsigned		cacheable;

	unsigned		busy;
	unsigned		len;

	double			age;
	double			entered;
	double			ttl;
	double			grace;
	double			prefetch;

	double			last_modified;

	struct http		http[1];
	VTAILQ_ENTRY(object)	list;

	VTAILQ_ENTRY(object)	deathrow;

	VTAILQ_HEAD(, storage)	store;

	VTAILQ_HEAD(, esi_bit)	esibits;

	double			lru_stamp;

	/* Prefetch */
	struct object		*parent;
	struct object		*child;
};

struct objhead {
	unsigned		magic;
#define OBJHEAD_MAGIC		0x1b96615d
	void			*hashpriv;

	pthread_mutex_t		mtx;
	VTAILQ_HEAD(,object)	objects;
	char			*hash;
	unsigned		hashlen;
	VTAILQ_HEAD(, sess)	waitinglist;
};

/* -------------------------------------------------------------------*/

struct sess {
	unsigned		magic;
#define SESS_MAGIC		0x2c2f9c5a
	int			fd;
	int			id;
	unsigned		xid;

	int			restarts;
	int			esis;

	struct worker		*wrk;

	socklen_t		sockaddrlen;
	socklen_t		mysockaddrlen;
	struct sockaddr		*sockaddr;
	struct sockaddr		*mysockaddr;

	/* formatted ascii client address */
	char			*addr;
	char			*port;
	struct srcaddr		*srcaddr;

	/* HTTP request */
	const char		*doclose;
	struct http		*http;
	struct http		*http0;

	struct ws		ws[1];
	char			*ws_ses;	/* WS above session data */
	char			*ws_req;	/* WS above request data */

	struct http_conn	htc[1];

	/* Timestamps, all on TIM_real() timescale */
	double			t_open;
	double			t_req;
	double			t_resp;
	double			t_end;

	/* Acceptable grace period */
	double			grace;

	enum step		step;
	unsigned		cur_method;
	unsigned 		handling;
	unsigned char		sendbody;
	unsigned char		wantbody;
	int			err_code;
	const char		*err_reason;

	VTAILQ_ENTRY(sess)	list;

	struct backend		*backend;
	struct bereq		*bereq;
	struct object		*obj;
	struct objhead		*objhead;
	struct VCL_conf		*vcl;

	/* Various internal stuff */
	struct sessmem		*mem;

	struct workreq		workreq;
	struct acct		acct;

	/* pointers to hash string components */
	unsigned		nhashptr;
	unsigned		ihashptr;
	unsigned		lhashptr;
	const char		**hashptr;
};

/* -------------------------------------------------------------------*/

/* Backend connection */
struct vbe_conn {
	unsigned		magic;
#define VBE_CONN_MAGIC		0x0c5e6592
	VTAILQ_ENTRY(vbe_conn)	list;
	struct backend		*backend;
	int			fd;
	void			*priv;
};


/* Backend method */
typedef struct vbe_conn *vbe_getfd_f(const struct sess *sp);
typedef void vbe_close_f(struct worker *w, struct vbe_conn *vc);
typedef void vbe_recycle_f(struct worker *w, struct vbe_conn *vc);
typedef void vbe_init_f(void);
typedef const char *vbe_gethostname_f(const struct backend *);
typedef void vbe_cleanup_f(const struct backend *);
typedef void vbe_updatehealth_f(const struct sess *sp, const struct vbe_conn *vc, int);

struct backend_method {
	const char		*name;
	vbe_getfd_f		*getfd;
	vbe_close_f		*close;
	vbe_recycle_f		*recycle;
	vbe_cleanup_f		*cleanup;
	vbe_gethostname_f	*gethostname;
	vbe_updatehealth_f	*updatehealth;
	vbe_init_f		*init;
};

/* Backend indstance */
struct backend {
	unsigned		magic;
#define BACKEND_MAGIC		0x64c4c7c6
	char			*vcl_name;

	VTAILQ_ENTRY(backend)	list;
	int			refcount;
	pthread_mutex_t		mtx;

	struct backend_method	*method;
	void			*priv;

	int			health;
	double			last_check;
	int			minute_limit;
};

/*
 * NB: This list is not locked, it is only ever manipulated from the
 * cachers CLI thread.
 */
VTAILQ_HEAD(backendlist, backend);

/* Prototypes etc ----------------------------------------------------*/


/* cache_acceptor.c */
void vca_return_session(struct sess *sp);
void vca_close_session(struct sess *sp, const char *why);
void VCA_Prep(struct sess *sp);
void VCA_Init(void);
extern int vca_pipes[2];

/* cache_backend.c */

void VBE_Init(void);
struct vbe_conn *VBE_GetFd(const struct sess *sp);
void VBE_ClosedFd(struct worker *w, struct vbe_conn *vc);
void VBE_RecycleFd(struct worker *w, struct vbe_conn *vc);
struct bereq * VBE_new_bereq(void);
void VBE_free_bereq(struct bereq *bereq);
extern struct backendlist backendlist;
void VBE_DropRef(struct backend *);
void VBE_DropRefLocked(struct backend *);
struct backend *VBE_NewBackend(struct backend_method *method);
struct vbe_conn *VBE_NewConn(void);
void VBE_ReleaseConn(struct vbe_conn *);
void VBE_UpdateHealth(const struct sess *sp, const struct vbe_conn *, int);

/* convenience functions for backend methods */
int VBE_TryConnect(const struct sess *sp, const struct addrinfo *ai);
int VBE_CheckFd(int fd);

/* cache_backend_simple.c */
extern struct backend_method	backend_method_simple;
extern struct backend_method	backend_method_random;
extern struct backend_method	backend_method_round_robin;

/* cache_ban.c */
void AddBan(const char *, int hash);
void BAN_Init(void);
void cli_func_url_purge(struct cli *cli, const char * const *av, void *priv);
void cli_func_hash_purge(struct cli *cli, const char * const *av, void *priv);
void BAN_NewObj(struct object *o);
int BAN_CheckObject(struct object *o, const char *url, const char *hash);

/* cache_center.c [CNT] */
void CNT_Session(struct sess *sp);
void CNT_Init(void);

/* cache_cli.c [CLI] */
void CLI_Init(void);

/* cache_expiry.c */
void EXP_Insert(struct object *o);
void EXP_Init(void);
void EXP_TTLchange(struct object *o);
void EXP_Touch(struct object *o, double now);
int EXP_NukeOne(struct sess *sp);

/* cache_fetch.c */
int Fetch(struct sess *sp);
int FetchReqBody(struct sess *sp);

/* cache_hash.c */
void HSH_Prealloc(struct sess *sp);
int HSH_Compare(const struct sess *sp, const struct objhead *o);
void HSH_Copy(const struct sess *sp, const struct objhead *o);
struct object *HSH_Lookup(struct sess *sp);
void HSH_Unbusy(struct object *o);
void HSH_Ref(struct object *o);
void HSH_Deref(struct object *o);
void HSH_Init(void);

/* cache_http.c */
const char *http_StatusMessage(unsigned);
void HTTP_Init(void);
void http_ClrHeader(struct http *to);
unsigned http_Write(struct worker *w, const struct http *hp, int resp);
void http_CopyResp(struct http *to, const struct http *fm);
void http_SetResp(struct http *to, const char *proto, const char *status, const char *response);
void http_FilterFields(struct worker *w, int fd, struct http *to, const struct http *fm, unsigned how);
void http_FilterHeader(struct sess *sp, unsigned how);
void http_PutProtocol(struct worker *w, int fd, struct http *to, const char *protocol);
void http_PutStatus(struct worker *w, int fd, struct http *to, int status);
void http_PutResponse(struct worker *w, int fd, struct http *to, const char *response);
void http_PrintfHeader(struct worker *w, int fd, struct http *to, const char *fmt, ...);
void http_SetHeader(struct worker *w, int fd, struct http *to, const char *hdr);
void http_SetH(struct http *to, unsigned n, const char *fm);
void http_Setup(struct http *ht, struct ws *ws);
int http_GetHdr(const struct http *hp, const char *hdr, char **ptr);
int http_GetHdrField(const struct http *hp, const char *hdr, const char *field, char **ptr);
int http_GetStatus(const struct http *hp);
const char *http_GetReq(const struct http *hp);
const char *http_GetProto(const struct http *hp);
int http_HdrIs(const struct http *hp, const char *hdr, const char *val);
int http_DissectRequest(struct sess *sp);
int http_DissectResponse(struct worker *w, const struct http_conn *htc, struct http *sp);
const char *http_DoConnection(struct http *hp);
void http_CopyHome(struct worker *w, int fd, struct http *hp);
void http_Unset(struct http *hp, const char *hdr);

/* cache_httpconn.c */
void HTC_Init(struct http_conn *htc, struct ws *ws, int fd);
int HTC_Reinit(struct http_conn *htc);
int HTC_Rx(struct http_conn *htc);
int HTC_Read(struct http_conn *htc, void *d, unsigned len);
int HTC_Complete(struct http_conn *htc);

#define HTTPH(a, b, c, d, e, f, g) extern char b[];
#include "http_headers.h"
#undef HTTPH

/* cache_main.c */
void THR_Name(const char *name);

/* cache_pipe.c */
void PipeSession(struct sess *sp);

/* cache_pool.c */
void WRK_Init(void);
void WRK_QueueSession(struct sess *sp);
void WRK_Reset(struct worker *w, int *fd);
unsigned WRK_Flush(struct worker *w);
unsigned WRK_Write(struct worker *w, const void *ptr, int len);
unsigned WRK_WriteH(struct worker *w, const txt *hh, const char *suf);
#ifdef SENDFILE_WORKS
void WRK_Sendfile(struct worker *w, int fd, off_t off, unsigned len);
#endif  /* SENDFILE_WORKS */

/* cache_session.c [SES] */
void SES_Init(void);
struct sess *SES_New(const struct sockaddr *addr, unsigned len);
void SES_Delete(struct sess *sp);
void SES_RefSrcAddr(struct sess *sp);
void SES_Charge(struct sess *sp);

/* cache_shmlog.c */

void VSL_Init(void);
#ifdef SHMLOGHEAD_MAGIC
void VSLR(enum shmlogtag tag, int id, txt t);
void VSL(enum shmlogtag tag, int id, const char *fmt, ...);
void WSLR(struct worker *w, enum shmlogtag tag, int id, txt t);
void WSL(struct worker *w, enum shmlogtag tag, int id, const char *fmt, ...);
void WSL_Flush(struct worker *w);
#define WSP(sess, tag, fmt, ...) \
	WSL((sess)->wrk, tag, (sess)->fd, fmt, __VA_ARGS__)
#define WSPR(sess, tag, txt) \
	WSLR((sess)->wrk, tag, (sess)->fd, txt)

#define INCOMPL() do {							\
	VSL(SLT_Debug, 0, "INCOMPLETE AT: %s(%d)", __func__, __LINE__); \
	fprintf(stderr,"INCOMPLETE AT: %s(%d)\n", (const char *)__func__, __LINE__);	\
	abort();							\
	} while (0)
#endif

/* cache_response.c */
void RES_BuildHttp(struct sess *sp);
void RES_WriteObj(struct sess *sp);

/* cache_synthetic.c */
void SYN_ErrorPage(struct sess *sp, int status, const char *reason);

/* cache_vary.c */
void VRY_Create(const struct sess *sp);
int VRY_Match(const struct sess *sp, const unsigned char *vary);

/* cache_vcl.c */
void VCL_Init(void);
void VCL_Refresh(struct VCL_conf **vcc);
void VCL_Rel(struct VCL_conf **vcc);
void VCL_Get(struct VCL_conf **vcc);

#define VCL_RET_MAC(l,u,b,n)
#define VCL_MET_MAC(l,u,b) void VCL_##l##_method(struct sess *);
#include "vcl_returns.h"
#undef VCL_MET_MAC
#undef VCL_RET_MAC

#ifdef CLI_PRIV_H
cli_func_t	cli_func_config_list;
cli_func_t	cli_func_config_load;
cli_func_t	cli_func_config_discard;
cli_func_t	cli_func_config_use;
cli_func_t	cli_func_dump_pool;
#endif

/* cache_vrt_esi.c */

void ESI_Deliver(struct sess *);
void ESI_Destroy(struct object *);

/* cache_ws.c */

void WS_Init(struct ws *ws, const char *id, void *space, unsigned len);
unsigned WS_Reserve(struct ws *ws, unsigned bytes);
void WS_Release(struct ws *ws, unsigned bytes);
void WS_ReleaseP(struct ws *ws, char *ptr);
void WS_Assert(const struct ws *ws);
void WS_Reset(struct ws *ws, char *p);
char *WS_Alloc(struct ws *ws, unsigned bytes);
char *WS_Dup(struct ws *ws, const char *);
char *WS_Snapshot(struct ws *ws);

/* rfc2616.c */
int RFC2616_cache_policy(const struct sess *sp, const struct http *hp);

#if 1
#define MTX			pthread_mutex_t
#define MTX_INIT(foo)		AZ(pthread_mutex_init(foo, NULL))
#define MTX_DESTROY(foo)	AZ(pthread_mutex_destroy(foo))
#define LOCK(foo)		AZ(pthread_mutex_lock(foo))
#define UNLOCK(foo)		AZ(pthread_mutex_unlock(foo))
#else
#define MTX			pthread_mutex_t
#define MTX_INIT(foo)		AZ(pthread_mutex_init(foo, NULL))
#define MTX_DESTROY(foo)	AZ(pthread_mutex_destroy(foo))
#define LOCK(foo) 					\
do { 							\
	if (pthread_mutex_trylock(foo)) {		\
		VSL(SLT_Debug, 0,			\
		    "MTX_CONTEST(%s,%s,%d," #foo ")",	\
		    __func__, __FILE__, __LINE__);	\
		AZ(pthread_mutex_lock(foo)); 		\
	} else if (1) {					\
		VSL(SLT_Debug, 0,			\
		    "MTX_LOCK(%s,%s,%d," #foo ")",	\
		    __func__, __FILE__, __LINE__); 	\
	}						\
} while (0);
#define UNLOCK(foo)					\
do {							\
	AZ(pthread_mutex_unlock(foo));			\
	if (1)						\
		VSL(SLT_Debug, 0,			\
		    "MTX_UNLOCK(%s,%s,%d," #foo ")",	\
		    __func__, __FILE__, __LINE__);	\
} while (0);
#endif

#if defined(HAVE_PTHREAD_MUTEX_ISLOCKED_NP)
#define ALOCKED(mutex)		AN(pthread_mutex_islocked_np((mutex)))
#elif defined(DIAGNOSTICS)
#define ALOCKED(mutex)		AN(pthread_mutex_trylock((mutex)))
#else
#define ALOCKED(mutex)		(void)(mutex)
#endif

/*
 * A normal pointer difference is signed, but we never want a negative value
 * so this little tool will make sure we don't get that.
 */

static inline unsigned
pdiff(const void *b, const void *e)
{

	assert(b <= e);
	return
	    ((unsigned)((const unsigned char *)e - (const unsigned char *)b));
}

static inline void
Tcheck(const txt t)
{

	AN(t.b);
	AN(t.e);
	assert(t.b <= t.e);
}

/*
 * unsigned length of a txt
 */

static inline unsigned
Tlen(const txt t)
{

	Tcheck(t);
	return
	    ((unsigned)(t.e - t.b));
}

#ifdef WITHOUT_ASSERTS
#define spassert(cond) ((void)(cond))
#else
void panic(const char *, int, const char *,
    const struct sess *, const char *, ...);
#define spassert(cond)						\
	do {							\
		int ok = !!(cond);				\
		if (!ok)					\
			panic(__FILE__, __LINE__, __func__, sp,	\
			    "assertion failed: %s\n", #cond);	\
	} while (0)
#endif
