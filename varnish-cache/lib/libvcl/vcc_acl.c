/*
 * $Id$
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdarg.h>
#include <sbuf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "vcc_priv.h"
#include "vcc_compile.h"

#include "libvcl.h"

void
vcc_Cond_Ip(struct var *vp, struct tokenlist *tl)
{

	(void)vp;	/* only client.ip at this time */

	switch (tl->t->tok) {
	case '~':
		vcc_NextToken(tl);
		ExpectErr(tl, ID);
		AddRef(tl, tl->t, R_ACL);
		Fc(tl, 1, "VRT_acl_match(sp, \"%T\", acl_%T)\n", tl->t, tl->t);
		vcc_NextToken(tl);
		break;
	default:
		sbuf_printf(tl->sb, "Illegal condition ");
		vcc_ErrToken(tl, tl->t);
		sbuf_printf(tl->sb, " on IP number variable\n");
		sbuf_printf(tl->sb, "  only '==', '!=' and '~' are legal\n");
		vcc_ErrWhere(tl, tl->t);
		break;
	}
}

void
vcc_Acl(struct tokenlist *tl)
{
	unsigned mask, para, not;
	struct token *t, *an;
	char *p;

	vcc_NextToken(tl);

	ExpectErr(tl, ID);
	an = tl->t;
	vcc_NextToken(tl);

	AddDef(tl, an, R_ACL);
	Fh(tl, 0, "static struct vrt_acl acl_%T[];\n", an);
	Fc(tl, 1, "static struct vrt_acl acl_%T[] = {\n", an);

	tl->indent += INDENT;

	ExpectErr(tl, '{');
	vcc_NextToken(tl);

	while (tl->t->tok != '}') {

		not = para = mask = 0;

		if (tl->t->tok == '(') {
			para = 1;
			vcc_NextToken(tl);
		} 

		if (tl->t->tok == '!') {
			not = 1;
			vcc_NextToken(tl);
		} 

		ExpectErr(tl, CSTR);
		/* XXX: try to look it up, warn if failure */
		t = tl->t;
		vcc_NextToken(tl);
		if (tl->t->tok == '/') {
			vcc_NextToken(tl);
			ExpectErr(tl, CNUM);
			mask = UintVal(tl);
		} 
		Fc(tl, 1, "{ %u, %u, %u, %T, \"", not, mask, para, t);
		if (para)
			Fc(tl, 0, "(");
		if (not)
			Fc(tl, 0, "!");
		p = EncString(t);
		Fc(tl, 0, "%s", p);
		free(p);
		if (mask)
			Fc(tl, 0, "/%u", mask);
		if (para)
			Fc(tl, 0, ")");
		Fc(tl, 0, "\" },\n");

		if (para) {
			ExpectErr(tl, ')');
			vcc_NextToken(tl);
		}
		ExpectErr(tl, ';');
		vcc_NextToken(tl);
	}
	Fc(tl, 1, "{ 0, 0, 0, (void*)0, ""}\n", 0, 0);
	tl->indent -= INDENT;
	Fc(tl, 1, "};\n\n");

	ExpectErr(tl, '}');
	vcc_NextToken(tl);

	Fi(tl, 1, "\tVRT_acl_init(acl_%T);\n", an);
	Ff(tl, 1, "\tVRT_acl_fini(acl_%T);\n", an);
}
