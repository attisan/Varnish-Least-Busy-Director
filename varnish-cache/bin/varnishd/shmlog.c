/*
 * $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <sys/mman.h>

#include "libvarnish.h"
#include "shmlog.h"
#include "cache.h"

#include "heritage.h"

#ifndef MAP_HASSEMAPHORE
#define MAP_HASSEMAPHORE 0 /* XXX Linux */
#endif

#ifndef MAP_NOSYNC
#define MAP_NOSYNC 0 /* XXX Linux */
#endif

struct varnish_stats *VSL_stats;
static struct shmloghead *loghead;
static unsigned char *logstart;
static pthread_mutex_t vsl_mutex;

/*
 * This variant copies a byte-range directly to the log, without
 * taking the detour over sprintf()
 */

static void
vsl_wrap(void)
{

	*logstart = SLT_ENDMARKER;
	logstart[loghead->ptr] = SLT_WRAPMARKER;
	loghead->ptr = 0;
}
 
void
VSLR(enum shmlogtag tag, unsigned id, const char *b, const char *e)
{
	unsigned char *p;
	unsigned l;

	assert(b != NULL);
	if (e == NULL)
		e = strchr(b, '\0');
	assert(e != NULL);

	/* Truncate */
	l = e - b;
	if (l > 255) {
		l = 255;
		e = b + l;
	}

	AZ(pthread_mutex_lock(&vsl_mutex));
	assert(loghead->ptr < loghead->size);

	/* Wrap if necessary */
	if (loghead->ptr + 4 + l + 1 > loghead->size)
		vsl_wrap();
	p = logstart + loghead->ptr;
	p[1] = l & 0xff;
	p[2] = (id >> 8) & 0xff;
	p[3] = id & 0xff;
	memcpy(p + 4, b, l);
	p[4 + l] = SLT_ENDMARKER;
	p[0] = tag;

	loghead->ptr += 4 + l;
	assert(loghead->ptr < loghead->size);
	AZ(pthread_mutex_unlock(&vsl_mutex));
}


void
VSL(enum shmlogtag tag, unsigned id, const char *fmt, ...)
{
	va_list ap;
	unsigned char *p;
	unsigned n;

	va_start(ap, fmt);

	AZ(pthread_mutex_lock(&vsl_mutex));
	assert(loghead->ptr < loghead->size);

	/* Wrap if we cannot fit a full size record */
	if (loghead->ptr + 4 + 255 + 1 > loghead->size) 
		vsl_wrap();

	p = logstart + loghead->ptr;
	n = 0;
	if (fmt != NULL) {
		n = vsnprintf((char *)(p + 4), 256, fmt, ap);
		if (n > 255)
			n = 255; 	/* we truncate long fields */
	}
	p[1] = n;
	p[2] = id >> 8;
	p[3] = id & 0xff;
	p[4 + n] = SLT_ENDMARKER;
	p[0] = tag;

	loghead->ptr += 4 + n;
	assert(loghead->ptr < loghead->size);
	
	AZ(pthread_mutex_unlock(&vsl_mutex));

	va_end(ap);
}

void
VSL_Init(void)
{

	loghead = mmap(NULL, heritage.vsl_size,
	    PROT_READ|PROT_WRITE,
	    MAP_HASSEMAPHORE | MAP_NOSYNC | MAP_SHARED,
	    heritage.vsl_fd, 0);
	assert(loghead != MAP_FAILED);

	/* XXX check sanity of loghead */
	logstart = (unsigned char *)loghead + loghead->start;
	AZ(pthread_mutex_init(&vsl_mutex, NULL));
	VSL_stats = &loghead->stats;
}

/*--------------------------------------------------------------------*/

void
VSL_MgtInit(const char *fn, unsigned size)
{
	struct shmloghead slh;
	int i = 0;

	heritage.vsl_fd = open(fn, O_RDWR, 0644);
	if (heritage.vsl_fd >= 0)
		i = read(heritage.vsl_fd, &slh, sizeof slh);
	if (heritage.vsl_fd < 0 || i != sizeof slh ||
	    slh.magic != SHMLOGHEAD_MAGIC ||
	    slh.hdrsize != sizeof slh) {
		/* XXX more checks */

		if (heritage.vsl_fd >= 0);
			close(heritage.vsl_fd);
		unlink(fn);
		heritage.vsl_fd = open(fn, O_RDWR | O_CREAT, 0644);
		if (heritage.vsl_fd < 0) {
			fprintf(stderr, "Could not open %s: %s\n",
			    fn, strerror(errno));
			exit (1);
		}
		memset(&slh, 0, sizeof slh);

		slh.magic = SHMLOGHEAD_MAGIC;
		slh.hdrsize = sizeof slh;
		slh.size = size;
		slh.ptr = 0;
		slh.start = sizeof slh;
		AZ(lseek(heritage.vsl_fd, 0, SEEK_SET));
		i = write(heritage.vsl_fd, &slh, sizeof slh);
		assert(i == sizeof slh);
		AZ(ftruncate(heritage.vsl_fd, sizeof slh + size));
	}
	heritage.vsl_size = slh.size + slh.start;

	/*
	 * Call VSL_Init so that we get a VSL_stats pointer in the
	 * management process as well.
	 */
	VSL_Init();
	memset(VSL_stats, 0, sizeof *VSL_stats);
}

/*--------------------------------------------------------------------*/

void
__assert(const char *func, const char *file, int line, const char *failedexpr)
{
	(void)fprintf(stderr,
	     "\r\nAssertion failed: (%s)\n"
	     "    function %s, file %s, line %d.\n"
	     "    errno %d = \"%s\"\n", 
	     failedexpr, func, file, line, errno, strerror(errno));
	abort();
	/* NOTREACHED */
}
