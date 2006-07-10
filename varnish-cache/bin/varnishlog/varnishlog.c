/*
 * $Id$
 *
 * Log tailer for Varnish
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sbuf.h>
#include <vis.h>

#include "shmlog.h"
#include "varnishapi.h"

/*
 * It would be simpler to use sparse array initialization and put it
 * directly in tagnames, but -pedantic gets in the way
 */

static struct tagnames {
	enum shmlogtag	tag;
	const char	*name;
} stagnames[] = {
#define SLTM(foo)	{ SLT_##foo, #foo },
#include "shmlog_tags.h"
#undef SLTM
	{ SLT_ENDMARKER, NULL}
};

static const char *tagnames[256];

static char *
vis_it(unsigned char *p)
{
	static char visbuf[255*4 + 3 + 1];

	strcpy(visbuf, " [");
	strvisx(visbuf + 2, p + 4, p[1],
	    VIS_OCTAL | VIS_TAB | VIS_NL);
	strcat(visbuf, "]");
	return (visbuf);
}

/* Ordering-----------------------------------------------------------*/

static struct sbuf	*ob[65536];
static int 		hc[65536];
static int 		xrf[65536];

static void
clean_order(void)
{
	unsigned u;

	for (u = 0; u < 65536; u++) {
		if (ob[u] == NULL)
			continue;
		sbuf_finish(ob[u]);
		if (sbuf_len(ob[u]))
			printf("%s\n", sbuf_data(ob[u]));
		sbuf_clear(ob[u]);
	}
}

static void 
order(unsigned char *p, int h_opt)
{
	unsigned u, v;

	u = (p[2] << 8) | p[3];
	if (ob[u] == NULL) {
		ob[u] = sbuf_new(NULL, NULL, 0, SBUF_AUTOEXTEND);
		assert(ob[u] != NULL);
	}
	v = 0;
	switch (p[0]) {
	case SLT_VCL_call:
		sbuf_printf(ob[u], "%02x %3d %4d %-12s",
		    p[0], p[1], u, tagnames[p[0]]);
		if (p[1] > 0) {
			sbuf_cat(ob[u], " <");
			sbuf_bcat(ob[u], p + 4, p[1]);
		}
		if (h_opt && p[1] == 3 && !memcmp(p + 4, "hit", 3))
			hc[u]++;
		break;
	case SLT_VCL_trace:
		if (p[1] > 0) {
			sbuf_cat(ob[u], " ");
			sbuf_bcat(ob[u], p + 4, p[1]);
		}
		break;
	case SLT_VCL_return:
		if (p[1] > 0) {
			sbuf_cat(ob[u], " ");
			sbuf_bcat(ob[u], p + 4, p[1]);
			sbuf_cat(ob[u], ">\n");
		}
		if (h_opt && p[1] == 7 && !memcmp(p + 4, "deliver", 7))
			hc[u]++;
		if (h_opt && p[1] == 6 && !memcmp(p + 4, "insert", 6)) {
			if (hc[xrf[u]] == 1) {
				hc[u] += 2;
				hc[xrf[u]] = 4;
			}
		}
		break;
	case SLT_Debug:
		if (p[1] == 0)
			break;
		if (!h_opt)
			;
		else if (p[1] > 4 && !memcmp(p + 4, "TTD:", 4))
			break;
		sbuf_printf(ob[u], "%02x %3d %4d %-12s",
		    p[0], p[1], u, tagnames[p[0]]);
		if (p[1] > 0)
			sbuf_cat(ob[u], vis_it(p));
		sbuf_cat(ob[u], "\n");
		break;
	case SLT_HttpError:
		if (!h_opt) 
			v = 1;
		else if (p[1] == 16 && !memcmp(p + 4, "Received nothing", 16))
			;
		else if (p[1] == 17 && !memcmp(p + 4, "Received errno 54", 17))
			;
		else
			v = 1;
		break;
	case SLT_SessionClose:
		if (!h_opt) 
			v = 1;
		else if (p[1] == 10 && !memcmp(p + 4, "no request", 10))
			;
		else if (p[1] == 7 && !memcmp(p + 4, "timeout", 7))
			;
		else
			v = 1;
		break;
	case SLT_Request:
		if (h_opt && p[1] == 3 && !memcmp(p + 4, "GET", 3))
			hc[u]++;
		if (h_opt && p[1] == 4 && !memcmp(p + 4, "HEAD", 3))
			hc[u]++;
		v = 1;
		break;
	case SLT_Backend:
		xrf[u] = atoi(p + 4);
		v = 1;
		break;
	case SLT_Status:
		if (h_opt && p[1] == 3 && !memcmp(p + 4, "200", 3))
			hc[u]++;
		v = 1;
		break;
	default:
		v = 1;
		break;
	}
	if (v) {
		sbuf_printf(ob[u], "%02x %3d %4d %-12s",
		    p[0], p[1], u, tagnames[p[0]]);
		if (p[1] > 0) {
			sbuf_cat(ob[u], " <");
			sbuf_bcat(ob[u], p + 4, p[1]);
			sbuf_cat(ob[u], ">");
		}
		sbuf_cat(ob[u], "\n");
	}
	if (u == 0) {
		sbuf_finish(ob[u]);
		printf("%s", sbuf_data(ob[u]));
		sbuf_clear(ob[u]);
		return;
	}
	switch (p[0]) {
	case SLT_SessionClose:
	case SLT_SessionReuse:
	case SLT_BackendClose:
		sbuf_finish(ob[u]);
		if ((hc[u] != 4 || h_opt == 0) && sbuf_len(ob[u]) > 1)
			printf("%s\n", sbuf_data(ob[u]));
		sbuf_clear(ob[u]);
		hc[u] = 0;
		xrf[u] = 0;
		break;
	default:
		break;
	}
}



/*--------------------------------------------------------------------*/

static void
Usage(void)
{
	fprintf(stderr, "Usage: varnishlog [-o] [-w file] [-r file]\n");
	exit(2);
}

int
main(int argc, char **argv)
{
	int i, c;
	unsigned u, v;
	unsigned char *p, *q;
	int o_flag = 0;
	char *w_opt = NULL;
	FILE *wfile = NULL;
	char *r_opt = NULL;
	FILE *rfile = NULL;
	int h_opt = 0;
	unsigned char rbuf[255+4];
	struct shmloghead *loghead;

	loghead = VSL_OpenLog();
	
	for (i = 0; stagnames[i].tag != SLT_ENDMARKER; i++)
		tagnames[stagnames[i].tag] = stagnames[i].name;

	while ((c = getopt(argc, argv, "hor:w:")) != -1) {
		switch (c) {
		case 'h':
			h_opt = 1;
			break;
		case 'o':
			o_flag = 1;
			break;
		case 'r':
			r_opt = optarg;
			break;
		case 'w':
			w_opt = optarg;
			break;
		default:
			Usage();
		}
	}

	if (r_opt != NULL && w_opt != NULL)
		Usage();
	if (o_flag && w_opt != NULL)
		Usage();

	if (r_opt != NULL) {
		if (!strcmp(r_opt, "-"))
			rfile = stdin;
		else
			rfile = fopen(r_opt, "r");
		if (rfile == NULL)
			perror(r_opt);
	}
	if (w_opt != NULL) {
		wfile = fopen(w_opt, "w");
		if (wfile == NULL)
			perror(w_opt);
	}
	u = 0;
	v = 0;

	q = NULL;
	if (r_opt == NULL) {
		while (VSL_NextLog(loghead, &q) != NULL)
			;
	}
	while (1) {
		if (r_opt == NULL) {
			p = VSL_NextLog(loghead, &q);
			if (p == NULL) {
				if (w_opt == NULL) {
					if (o_flag && ++v == 100)
						clean_order();
					fflush(stdout);
				} else if (++v == 100) {
					fflush(wfile);
					printf("\nFlushed\n");
				}
				usleep(50000);
				continue;
			}
		} else {
			i = fread(rbuf, 4, 1, rfile);
			if (i != 1)
				break;
			if (rbuf[1] > 0)
			i = fread(rbuf + 4, rbuf[1], 1, rfile);
			if (i != 1)
				break;
			p = rbuf;
		}
		v = 0;
		if (wfile != NULL) {
			i = fwrite(p, 4 + p[1], 1, wfile);
			if (i != 1)
				perror(w_opt);
			u++;
			if (!(u % 1000)) {
				printf("%u\r", u);
				fflush(stdout);
			}
			continue;
		}
		if (o_flag) {
			order(p, h_opt);
			continue;
		}
		u = (p[2] << 8) | p[3];
		printf("%02x %3d %4d %-12s",
		    p[0], p[1], u, tagnames[p[0]]);
		
		if (p[1] > 0) {
			if (p[0] != SLT_Debug) {
				printf(" <");
				fwrite(p + 4, p[1], 1, stdout);
				printf(">");
			} else {
				fputs(vis_it(p), stdout);
			}
				
		}
		printf("\n");
	}
	if (o_flag)
		clean_order();
	return (0);
}
