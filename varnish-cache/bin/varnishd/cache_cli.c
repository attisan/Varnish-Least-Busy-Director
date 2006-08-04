/*
 * $Id$
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>

#include "libvarnish.h"
#include "shmlog.h"
#include "cli.h"
#include "cli_priv.h"
#include "common_cli.h"
#include "cache.h"
#include "sbuf.h"
#include "heritage.h"

/*--------------------------------------------------------------------*/

struct cli_proto CLI_cmds[] = {
	{ CLI_PING,		cli_func_ping },
#if 0
	{ CLI_URL_QUERY,	cli_func_url_query },
#endif
	{ CLI_URL_PURGE,	cli_func_url_purge },
	{ CLI_CONFIG_LOAD,	cli_func_config_load },
	{ CLI_CONFIG_LIST,	cli_func_config_list },
	{ CLI_CONFIG_UNLOAD,	cli_func_config_unload },
	{ CLI_CONFIG_USE,	cli_func_config_use },
	{ NULL }
};

void
CLI_Init(void)
{
	struct pollfd pfd[1];
	char *buf, *p;
	unsigned nbuf, lbuf;
	struct cli *cli, clis;
	int i;

	cli = &clis;
	memset(cli, 0, sizeof *cli);
	
	cli->sb = sbuf_new(NULL, NULL, 0, SBUF_AUTOEXTEND);
	assert(cli->sb != NULL);
	lbuf = 4096;
	buf = malloc(lbuf);
	assert(buf != NULL);
	nbuf = 0;
	while (1) {
		pfd[0].fd = heritage.fds[2];
		pfd[0].events = POLLIN;
		i = poll(pfd, 1, 5000);
		if (i == 0)
			continue;
		if (nbuf == lbuf) {
			lbuf += lbuf;
			buf = realloc(buf, lbuf);
			assert(buf != NULL);
		}
		i = read(heritage.fds[2], buf + nbuf, lbuf - nbuf);
		if (i <= 0) {
			VSL(SLT_Error, 0, "CLI read %d (errno=%d)", i, errno);
			return;
		}
		nbuf += i;
		p = strchr(buf, '\n');
		if (p == NULL)
			continue;
		*p = '\0';
		VSL(SLT_CLI, 0, "Rd %s", buf);
		sbuf_clear(cli->sb);
		cli_dispatch(cli, CLI_cmds, buf);
		sbuf_finish(cli->sb);
		i = cli_writeres(heritage.fds[1], cli);
		if (i) {
			VSL(SLT_Error, 0, "CLI write failed (errno=%d)", errno);
			return;
		}
		VSL(SLT_CLI, 0, "Wr %d %d %s",
		    i, cli->result, sbuf_data(cli->sb));
		i = ++p - buf; 
		assert(i <= nbuf);
		if (i < nbuf)
			memcpy(buf, p, nbuf - i);
		nbuf -= i;
	}
}
