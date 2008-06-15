/*
 * Copyright (c) 2006-2008 Linpro AS
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "vtc.h"

#define		MAX_FILESIZE		(1024 * 1024)
#define		MAX_TOKENS		20

/**********************************************************************
 * Read a file into memory
 */

static char *
read_file(const char *fn)
{
	char *buf;
	ssize_t sz = MAX_FILESIZE;
	ssize_t s;
	int fd;

	fd = open(fn, O_RDONLY);
	if (fd < 0)
		err(1, "Cannot open %s", fn);
	buf = malloc(sz);
	assert(buf != NULL);
	s = read(fd, buf, sz);
	if (s <= 0) 
		err(1, "Cannot read %s", fn);
	assert(s < sz);		/* XXX: increase MAX_FILESIZE */
	close (fd);
	buf[s] = '\0';
	buf = realloc(buf, s + 1);
	assert(buf != NULL);
	return (buf);
}

/**********************************************************************
 * Execute a file
 */

void
parse_string(char *buf, const struct cmds *cmd, void *priv)
{
	char *token_s[MAX_TOKENS], *token_e[MAX_TOKENS];
	char *p, *q;
	int nest_brace;
	int tn;
	const struct cmds *cp;

	assert(buf != NULL);
	for (p = buf; *p != '\0'; p++) {
		/* Start of line */
		if (isspace(*p))
			continue;
		if (*p == '#') {
			for (; *p != '\0' && *p != '\n'; p++)
				;
			if (*p == '\0')
				break;
			continue;
		}

		/* First content on line, collect tokens */
		tn = 0;
		while (*p != '\0') {
			assert(tn < MAX_TOKENS);
			if (*p == '\n') { /* End on NL */
				break;
			} else if (isspace(*p)) { /* Inter-token whitespace */
				p++;
			} else if (*p == '\\' && p[1] == '\n') {
				p += 2;
			} else if (*p == '"') { /* quotes */
				token_s[tn] = ++p;
				q = p;
				for (; *p != '\0'; p++) {
					if (*p == '"')
						break;

					if (*p == '\\' && p[1] == 'n') {
						*q++ = '\n';
						p++;
					} else if (*p == '\\' && p[1] == '\\') {
						*q++ = '\\';
						p++;
					} else if (*p == '\\' && p[1] == '"') {
						*q++ = '"';
						p++;
					} else {
						assert(*p != '\n');
						*q++ = *p;
					}
				}
				token_e[tn++] = q;
				p++;
			} else if (*p == '{') { /* Braces */
				nest_brace = 0;
				token_s[tn] = p;
				for (; *p != '\0'; p++) {
					if (*p == '{')
						nest_brace++;
					else if (*p == '}') {
						if (--nest_brace == 0)
							break;
					}
				}
				assert(*p == '}');
				token_e[tn++] = ++p;
			} else { /* other tokens */
				token_s[tn] = p;
				for (; *p != '\0' && !isspace(*p); p++)
					;
				token_e[tn++] = p;
			}
		}
		assert(tn < MAX_TOKENS);
		token_s[tn] = NULL;
		for (tn = 0; token_s[tn] != NULL; tn++)
			*token_e[tn] = '\0';

		for (cp = cmd; cp->name != NULL; cp++)
			if (!strcmp(token_s[0], cp->name))
				break;
		if (cp->name == NULL) {
			for (tn = 0; token_s[tn] != NULL; tn++)
				fprintf(stderr, "%s ", token_s[tn]);
			fprintf(stderr, "\n");
			errx(1, "Unknown command: \"%s\"", token_s[0]);
		}
	
		assert(cp->cmd != NULL);
		cp->cmd(token_s, priv);
	}
}

/**********************************************************************
 * Execute a file
 */

void
cmd_dump(char **av, void *priv)
{

	printf("cmd_dump(%p)\n", priv);
	while (*av)
		printf("\t<%s>\n", *av++);
}

static struct cmds cmds[] = {
	{ "server", 	cmd_server },
	{ "client", 	cmd_client },
	{ "vcl", 	cmd_vcl },
	{ "stats", 	cmd_stats },
	{ "varnish", 	cmd_varnish },
	{ NULL, 	NULL }
};

static void
exec_file(const char *fn)
{
	char *buf;

	buf = read_file(fn);
	parse_string(buf, cmds, NULL);
}

/**********************************************************************
 * Main 
 */

int
main(int argc, char **argv)
{
	int ch;

	setbuf(stdout, NULL);
	while ((ch = getopt(argc, argv, "")) != -1) {
		switch (ch) {
		case '?':
		default:
			errx(1, "Usage");
		}
	}
	argc -= optind;
	argv += optind;
	for (ch = 0; ch < argc; ch++)
		exec_file(argv[ch]);
	return (0);
}