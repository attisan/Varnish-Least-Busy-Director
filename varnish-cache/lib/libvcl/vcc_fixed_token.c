/*
 * $Id$
 *
 * NB:  This file is machine generated, DO NOT EDIT!
 *
 * Edit vcc_gen_fixed_token.tcl instead
 */

#include <stdio.h>
#include <ctype.h>
#include "vcc_priv.h"
#include "vsb.h"

unsigned
vcl_fixed_token(const char *p, const char **q)
{

	switch (p[0]) {
	case '!':
		if (p[0] == '!' && p[1] == '=') {
			*q = p + 2;
			return (T_NEQ);
		}
		if (p[0] == '!') {
			*q = p + 1;
			return ('!');
		}
		return (0);
	case '%':
		if (p[0] == '%') {
			*q = p + 1;
			return ('%');
		}
		return (0);
	case '&':
		if (p[0] == '&' && p[1] == '&') {
			*q = p + 2;
			return (T_CAND);
		}
		if (p[0] == '&') {
			*q = p + 1;
			return ('&');
		}
		return (0);
	case '(':
		if (p[0] == '(') {
			*q = p + 1;
			return ('(');
		}
		return (0);
	case ')':
		if (p[0] == ')') {
			*q = p + 1;
			return (')');
		}
		return (0);
	case '*':
		if (p[0] == '*' && p[1] == '=') {
			*q = p + 2;
			return (T_MUL);
		}
		if (p[0] == '*') {
			*q = p + 1;
			return ('*');
		}
		return (0);
	case '+':
		if (p[0] == '+' && p[1] == '=') {
			*q = p + 2;
			return (T_INCR);
		}
		if (p[0] == '+' && p[1] == '+') {
			*q = p + 2;
			return (T_INC);
		}
		if (p[0] == '+') {
			*q = p + 1;
			return ('+');
		}
		return (0);
	case ',':
		if (p[0] == ',') {
			*q = p + 1;
			return (',');
		}
		return (0);
	case '-':
		if (p[0] == '-' && p[1] == '=') {
			*q = p + 2;
			return (T_DECR);
		}
		if (p[0] == '-' && p[1] == '-') {
			*q = p + 2;
			return (T_DEC);
		}
		if (p[0] == '-') {
			*q = p + 1;
			return ('-');
		}
		return (0);
	case '.':
		if (p[0] == '.') {
			*q = p + 1;
			return ('.');
		}
		return (0);
	case '/':
		if (p[0] == '/' && p[1] == '=') {
			*q = p + 2;
			return (T_DIV);
		}
		if (p[0] == '/') {
			*q = p + 1;
			return ('/');
		}
		return (0);
	case ';':
		if (p[0] == ';') {
			*q = p + 1;
			return (';');
		}
		return (0);
	case '<':
		if (p[0] == '<' && p[1] == '=') {
			*q = p + 2;
			return (T_LEQ);
		}
		if (p[0] == '<' && p[1] == '<') {
			*q = p + 2;
			return (T_SHL);
		}
		if (p[0] == '<') {
			*q = p + 1;
			return ('<');
		}
		return (0);
	case '=':
		if (p[0] == '=' && p[1] == '=') {
			*q = p + 2;
			return (T_EQ);
		}
		if (p[0] == '=') {
			*q = p + 1;
			return ('=');
		}
		return (0);
	case '>':
		if (p[0] == '>' && p[1] == '>') {
			*q = p + 2;
			return (T_SHR);
		}
		if (p[0] == '>' && p[1] == '=') {
			*q = p + 2;
			return (T_GEQ);
		}
		if (p[0] == '>') {
			*q = p + 1;
			return ('>');
		}
		return (0);
	case 'a':
		if (p[0] == 'a' && p[1] == 'c' && p[2] == 'l'
		     && !isvar(p[3])) {
			*q = p + 3;
			return (T_ACL);
		}
		return (0);
	case 'b':
		if (p[0] == 'b' && p[1] == 'a' && p[2] == 'c' && 
		    p[3] == 'k' && p[4] == 'e' && p[5] == 'n' && 
		    p[6] == 'd' && !isvar(p[7])) {
			*q = p + 7;
			return (T_BACKEND);
		}
		return (0);
	case 'c':
		if (p[0] == 'c' && p[1] == 'a' && p[2] == 'l' && 
		    p[3] == 'l' && !isvar(p[4])) {
			*q = p + 4;
			return (T_CALL);
		}
		return (0);
	case 'd':
		if (p[0] == 'd' && p[1] == 'i' && p[2] == 's' && 
		    p[3] == 'c' && p[4] == 'a' && p[5] == 'r' && 
		    p[6] == 'd' && !isvar(p[7])) {
			*q = p + 7;
			return (T_DISCARD);
		}
		if (p[0] == 'd' && p[1] == 'e' && p[2] == 'l' && 
		    p[3] == 'i' && p[4] == 'v' && p[5] == 'e' && 
		    p[6] == 'r' && !isvar(p[7])) {
			*q = p + 7;
			return (T_DELIVER);
		}
		return (0);
	case 'e':
		if (p[0] == 'e' && p[1] == 'r' && p[2] == 'r' && 
		    p[3] == 'o' && p[4] == 'r' && !isvar(p[5])) {
			*q = p + 5;
			return (T_ERROR);
		}
		if (p[0] == 'e' && p[1] == 'l' && p[2] == 's' && 
		    p[3] == 'i' && p[4] == 'f' && !isvar(p[5])) {
			*q = p + 5;
			return (T_ELSIF);
		}
		if (p[0] == 'e' && p[1] == 'l' && p[2] == 's' && 
		    p[3] == 'e' && p[4] == 'i' && p[5] == 'f'
		     && !isvar(p[6])) {
			*q = p + 6;
			return (T_ELSEIF);
		}
		if (p[0] == 'e' && p[1] == 'l' && p[2] == 's' && 
		    p[3] == 'e' && !isvar(p[4])) {
			*q = p + 4;
			return (T_ELSE);
		}
		return (0);
	case 'f':
		if (p[0] == 'f' && p[1] == 'u' && p[2] == 'n' && 
		    p[3] == 'c' && !isvar(p[4])) {
			*q = p + 4;
			return (T_FUNC);
		}
		if (p[0] == 'f' && p[1] == 'e' && p[2] == 't' && 
		    p[3] == 'c' && p[4] == 'h' && !isvar(p[5])) {
			*q = p + 5;
			return (T_FETCH);
		}
		return (0);
	case 'h':
		if (p[0] == 'h' && p[1] == 'a' && p[2] == 's' && 
		    p[3] == 'h' && !isvar(p[4])) {
			*q = p + 4;
			return (T_HASH);
		}
		return (0);
	case 'i':
		if (p[0] == 'i' && p[1] == 'n' && p[2] == 's' && 
		    p[3] == 'e' && p[4] == 'r' && p[5] == 't'
		     && !isvar(p[6])) {
			*q = p + 6;
			return (T_INSERT);
		}
		if (p[0] == 'i' && p[1] == 'n' && p[2] == 'c' && 
		    p[3] == 'l' && p[4] == 'u' && p[5] == 'd' && 
		    p[6] == 'e' && !isvar(p[7])) {
			*q = p + 7;
			return (T_INCLUDE);
		}
		if (p[0] == 'i' && p[1] == 'f' && !isvar(p[2])) {
			*q = p + 2;
			return (T_IF);
		}
		return (0);
	case 'l':
		if (p[0] == 'l' && p[1] == 'o' && p[2] == 'o' && 
		    p[3] == 'k' && p[4] == 'u' && p[5] == 'p'
		     && !isvar(p[6])) {
			*q = p + 6;
			return (T_LOOKUP);
		}
		return (0);
	case 'n':
		if (p[0] == 'n' && p[1] == 'o' && p[2] == '_' && 
		    p[3] == 'n' && p[4] == 'e' && p[5] == 'w' && 
		    p[6] == '_' && p[7] == 'c' && p[8] == 'a' && 
		    p[9] == 'c' && p[10] == 'h' && p[11] == 'e'
		     && !isvar(p[12])) {
			*q = p + 12;
			return (T_NO_NEW_CACHE);
		}
		if (p[0] == 'n' && p[1] == 'o' && p[2] == '_' && 
		    p[3] == 'c' && p[4] == 'a' && p[5] == 'c' && 
		    p[6] == 'h' && p[7] == 'e' && !isvar(p[8])) {
			*q = p + 8;
			return (T_NO_CACHE);
		}
		return (0);
	case 'p':
		if (p[0] == 'p' && p[1] == 'r' && p[2] == 'o' && 
		    p[3] == 'c' && !isvar(p[4])) {
			*q = p + 4;
			return (T_PROC);
		}
		if (p[0] == 'p' && p[1] == 'i' && p[2] == 'p' && 
		    p[3] == 'e' && !isvar(p[4])) {
			*q = p + 4;
			return (T_PIPE);
		}
		if (p[0] == 'p' && p[1] == 'a' && p[2] == 's' && 
		    p[3] == 's' && !isvar(p[4])) {
			*q = p + 4;
			return (T_PASS);
		}
		return (0);
	case 'r':
		if (p[0] == 'r' && p[1] == 'e' && p[2] == 'w' && 
		    p[3] == 'r' && p[4] == 'i' && p[5] == 't' && 
		    p[6] == 'e' && !isvar(p[7])) {
			*q = p + 7;
			return (T_REWRITE);
		}
		return (0);
	case 's':
		if (p[0] == 's' && p[1] == 'w' && p[2] == 'i' && 
		    p[3] == 't' && p[4] == 'c' && p[5] == 'h' && 
		    p[6] == '_' && p[7] == 'c' && p[8] == 'o' && 
		    p[9] == 'n' && p[10] == 'f' && p[11] == 'i' && 
		    p[12] == 'g' && !isvar(p[13])) {
			*q = p + 13;
			return (T_SWITCH_CONFIG);
		}
		if (p[0] == 's' && p[1] == 'u' && p[2] == 'b'
		     && !isvar(p[3])) {
			*q = p + 3;
			return (T_SUB);
		}
		return (0);
	case '{':
		if (p[0] == '{') {
			*q = p + 1;
			return ('{');
		}
		return (0);
	case '|':
		if (p[0] == '|' && p[1] == '|') {
			*q = p + 2;
			return (T_COR);
		}
		if (p[0] == '|') {
			*q = p + 1;
			return ('|');
		}
		return (0);
	case '}':
		if (p[0] == '}') {
			*q = p + 1;
			return ('}');
		}
		return (0);
	case '~':
		if (p[0] == '~') {
			*q = p + 1;
			return ('~');
		}
		return (0);
	default:
		return (0);
	}
}

const char *vcl_tnames[256];

void
vcl_init_tnames(void)
{
	vcl_tnames['!'] = "'!'";
	vcl_tnames['%'] = "'%'";
	vcl_tnames['&'] = "'&'";
	vcl_tnames['('] = "'('";
	vcl_tnames[')'] = "')'";
	vcl_tnames['*'] = "'*'";
	vcl_tnames['+'] = "'+'";
	vcl_tnames[','] = "','";
	vcl_tnames['-'] = "'-'";
	vcl_tnames['.'] = "'.'";
	vcl_tnames['/'] = "'/'";
	vcl_tnames['<'] = "'<'";
	vcl_tnames['='] = "'='";
	vcl_tnames['>'] = "'>'";
	vcl_tnames['{'] = "'{'";
	vcl_tnames['}'] = "'}'";
	vcl_tnames['|'] = "'|'";
	vcl_tnames['~'] = "'~'";
	vcl_tnames[';'] = "';'";
	vcl_tnames[CNUM] = "CNUM";
	vcl_tnames[CSTR] = "CSTR";
	vcl_tnames[EOI] = "EOI";
	vcl_tnames[ID] = "ID";
	vcl_tnames[METHOD] = "METHOD";
	vcl_tnames[T_ACL] = "acl";
	vcl_tnames[T_BACKEND] = "backend";
	vcl_tnames[T_CALL] = "call";
	vcl_tnames[T_CAND] = "&&";
	vcl_tnames[T_COR] = "||";
	vcl_tnames[T_DEC] = "--";
	vcl_tnames[T_DECR] = "-=";
	vcl_tnames[T_DELIVER] = "deliver";
	vcl_tnames[T_DISCARD] = "discard";
	vcl_tnames[T_DIV] = "/=";
	vcl_tnames[T_ELSE] = "else";
	vcl_tnames[T_ELSEIF] = "elseif";
	vcl_tnames[T_ELSIF] = "elsif";
	vcl_tnames[T_EQ] = "==";
	vcl_tnames[T_ERROR] = "error";
	vcl_tnames[T_FETCH] = "fetch";
	vcl_tnames[T_FUNC] = "func";
	vcl_tnames[T_GEQ] = ">=";
	vcl_tnames[T_HASH] = "hash";
	vcl_tnames[T_IF] = "if";
	vcl_tnames[T_INC] = "++";
	vcl_tnames[T_INCLUDE] = "include";
	vcl_tnames[T_INCR] = "+=";
	vcl_tnames[T_INSERT] = "insert";
	vcl_tnames[T_LEQ] = "<=";
	vcl_tnames[T_LOOKUP] = "lookup";
	vcl_tnames[T_MUL] = "*=";
	vcl_tnames[T_NEQ] = "!=";
	vcl_tnames[T_NO_CACHE] = "no_cache";
	vcl_tnames[T_NO_NEW_CACHE] = "no_new_cache";
	vcl_tnames[T_PASS] = "pass";
	vcl_tnames[T_PIPE] = "pipe";
	vcl_tnames[T_PROC] = "proc";
	vcl_tnames[T_REWRITE] = "rewrite";
	vcl_tnames[T_SHL] = "<<";
	vcl_tnames[T_SHR] = ">>";
	vcl_tnames[T_SUB] = "sub";
	vcl_tnames[T_SWITCH_CONFIG] = "switch_config";
	vcl_tnames[VAR] = "VAR";
}

void
vcl_output_lang_h(struct vsb *sb)
{
	vsb_cat(sb, "#define VCL_RET_ERROR  (1 << 0)\n");
	vsb_cat(sb, "#define VCL_RET_LOOKUP  (1 << 1)\n");
	vsb_cat(sb, "#define VCL_RET_HASH  (1 << 2)\n");
	vsb_cat(sb, "#define VCL_RET_PIPE  (1 << 3)\n");
	vsb_cat(sb, "#define VCL_RET_PASS  (1 << 4)\n");
	vsb_cat(sb, "#define VCL_RET_FETCH  (1 << 5)\n");
	vsb_cat(sb, "#define VCL_RET_INSERT  (1 << 6)\n");
	vsb_cat(sb, "#define VCL_RET_DELIVER  (1 << 7)\n");
	vsb_cat(sb, "#define VCL_RET_DISCARD  (1 << 8)\n");
	vsb_cat(sb, "/*\n");
	vsb_cat(sb, " * $Id$\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * NB:  This file is machine generated, DO NOT EDIT!\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * Edit vcc_gen_fixed_token.tcl instead\n");
	vsb_cat(sb, " */\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "struct sess;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "typedef void vcl_init_f(void);\n");
	vsb_cat(sb, "typedef void vcl_fini_f(void);\n");
	vsb_cat(sb, "typedef int vcl_func_f(struct sess *sp);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "struct VCL_conf {\n");
	vsb_cat(sb, "	unsigned        magic;\n");
	vsb_cat(sb, "#define VCL_CONF_MAGIC  0x7406c509      /* from /dev/random */\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "        struct backend  **backend;\n");
	vsb_cat(sb, "        unsigned        nbackend;\n");
	vsb_cat(sb, "        struct vrt_ref  *ref;\n");
	vsb_cat(sb, "        unsigned        nref;\n");
	vsb_cat(sb, "        unsigned        busy;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "	unsigned	nsrc;\n");
	vsb_cat(sb, "	const char	**srcname;\n");
	vsb_cat(sb, "	const char	**srcbody;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "        void            *priv;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "        vcl_init_f      *init_func;\n");
	vsb_cat(sb, "        vcl_fini_f      *fini_func;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "	vcl_func_f	*recv_func;\n");
	vsb_cat(sb, "	vcl_func_f	*pipe_func;\n");
	vsb_cat(sb, "	vcl_func_f	*pass_func;\n");
	vsb_cat(sb, "	vcl_func_f	*hash_func;\n");
	vsb_cat(sb, "	vcl_func_f	*miss_func;\n");
	vsb_cat(sb, "	vcl_func_f	*hit_func;\n");
	vsb_cat(sb, "	vcl_func_f	*fetch_func;\n");
	vsb_cat(sb, "	vcl_func_f	*timeout_func;\n");
	vsb_cat(sb, "};\n");
	vsb_cat(sb, "/*-\n");
	vsb_cat(sb, " * Copyright (c) 2006 Verdens Gang AS\n");
	vsb_cat(sb, " * Copyright (c) 2006 Linpro AS\n");
	vsb_cat(sb, " * All rights reserved.\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * Author: Poul-Henning Kamp <phk@phk.freebsd.dk>\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * Redistribution and use in source and binary forms, with or without\n");
	vsb_cat(sb, " * modification, are permitted provided that the following conditions\n");
	vsb_cat(sb, " * are met:\n");
	vsb_cat(sb, " * 1. Redistributions of source code must retain the above copyright\n");
	vsb_cat(sb, " *    notice, this list of conditions and the following disclaimer.\n");
	vsb_cat(sb, " * 2. Redistributions in binary form must reproduce the above copyright\n");
	vsb_cat(sb, " *    notice, this list of conditions and the following disclaimer in the\n");
	vsb_cat(sb, " *    documentation and/or other materials provided with the distribution.\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND\n");
	vsb_cat(sb, " * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n");
	vsb_cat(sb, " * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n");
	vsb_cat(sb, " * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE\n");
	vsb_cat(sb, " * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n");
	vsb_cat(sb, " * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n");
	vsb_cat(sb, " * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n");
	vsb_cat(sb, " * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\n");
	vsb_cat(sb, " * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\n");
	vsb_cat(sb, " * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\n");
	vsb_cat(sb, " * SUCH DAMAGE.\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * $Id$\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * Runtime support for compiled VCL programs.\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * XXX: When this file is changed, lib/libvcl/vcc_gen_fixed_token.tcl\n");
	vsb_cat(sb, " * XXX: *MUST* be rerun.\n");
	vsb_cat(sb, " */\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "struct sess;\n");
	vsb_cat(sb, "struct vsb;\n");
	vsb_cat(sb, "struct backend;\n");
	vsb_cat(sb, "struct VCL_conf;\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "struct vrt_ref {\n");
	vsb_cat(sb, "	unsigned	source;\n");
	vsb_cat(sb, "	unsigned	offset;\n");
	vsb_cat(sb, "	unsigned	line;\n");
	vsb_cat(sb, "	unsigned	pos;\n");
	vsb_cat(sb, "	unsigned	count;\n");
	vsb_cat(sb, "	const char	*token;\n");
	vsb_cat(sb, "};\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "struct vrt_acl {\n");
	vsb_cat(sb, "	unsigned char	not;\n");
	vsb_cat(sb, "	unsigned char	mask;\n");
	vsb_cat(sb, "	unsigned char	paren;\n");
	vsb_cat(sb, "	const char	*name;\n");
	vsb_cat(sb, "	const char	*desc;\n");
	vsb_cat(sb, "	void		*priv;\n");
	vsb_cat(sb, "};\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "/* ACL related */\n");
	vsb_cat(sb, "int VRT_acl_match(struct sess *, const char *, struct vrt_acl *);\n");
	vsb_cat(sb, "void VRT_acl_init(struct vrt_acl *);\n");
	vsb_cat(sb, "void VRT_acl_fini(struct vrt_acl *);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "/* Regexp related */\n");
	vsb_cat(sb, "void VRT_re_init(void **, const char *);\n");
	vsb_cat(sb, "void VRT_re_fini(void *);\n");
	vsb_cat(sb, "int VRT_re_match(const char *, void *re);\n");
	vsb_cat(sb, "int VRT_re_test(struct vsb *, const char *);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "void VRT_count(struct sess *, unsigned);\n");
	vsb_cat(sb, "int VRT_rewrite(const char *, const char *);\n");
	vsb_cat(sb, "void VRT_error(struct sess *, unsigned, const char *);\n");
	vsb_cat(sb, "int VRT_switch_config(const char *);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "char *VRT_GetHdr(struct sess *, int where, const char *);\n");
	vsb_cat(sb, "void VRT_handling(struct sess *sp, unsigned hand);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "/* Backend related */\n");
	vsb_cat(sb, "void VRT_set_backend_name(struct backend *, const char *);\n");
	vsb_cat(sb, "void VRT_alloc_backends(struct VCL_conf *cp);\n");
	vsb_cat(sb, "void VRT_free_backends(struct VCL_conf *cp);\n");
	vsb_cat(sb, "void VRT_fini_backend(struct backend *be);\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "#define VRT_done(sp, hand)			\\\n");
	vsb_cat(sb, "	do {					\\\n");
	vsb_cat(sb, "		VRT_handling(sp, hand);		\\\n");
	vsb_cat(sb, "		return (1);			\\\n");
	vsb_cat(sb, "	} while (0)\n");
	vsb_cat(sb, "/*\n");
	vsb_cat(sb, " * $Id$\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * NB:  This file is machine generated, DO NOT EDIT!\n");
	vsb_cat(sb, " *\n");
	vsb_cat(sb, " * Edit vcc_gen_obj.tcl instead\n");
	vsb_cat(sb, " */\n");
	vsb_cat(sb, "\n");
	vsb_cat(sb, "const char * VRT_r_backend_host(struct backend *);\n");
	vsb_cat(sb, "void VRT_l_backend_host(struct backend *, const char *);\n");
	vsb_cat(sb, "const char * VRT_r_backend_port(struct backend *);\n");
	vsb_cat(sb, "void VRT_l_backend_port(struct backend *, const char *);\n");
	vsb_cat(sb, "double VRT_r_backend_dnsttl(struct backend *);\n");
	vsb_cat(sb, "void VRT_l_backend_dnsttl(struct backend *, double);\n");
	vsb_cat(sb, "const unsigned char * VRT_r_client_ip(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_client_ip(struct sess *, const unsigned char *);\n");
	vsb_cat(sb, "const char * VRT_r_req_request(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_request(struct sess *, const char *);\n");
	vsb_cat(sb, "const char * VRT_r_req_host(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_host(struct sess *, const char *);\n");
	vsb_cat(sb, "const char * VRT_r_req_url(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_url(struct sess *, const char *);\n");
	vsb_cat(sb, "const char * VRT_r_req_proto(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_proto(struct sess *, const char *);\n");
	vsb_cat(sb, "struct backend * VRT_r_req_backend(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_backend(struct sess *, struct backend *);\n");
	vsb_cat(sb, "double VRT_r_obj_valid(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_obj_valid(struct sess *, double);\n");
	vsb_cat(sb, "double VRT_r_obj_cacheable(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_obj_cacheable(struct sess *, double);\n");
	vsb_cat(sb, "double VRT_r_obj_ttl(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_obj_ttl(struct sess *, double);\n");
	vsb_cat(sb, "const char * VRT_r_req_http_(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_req_http_(struct sess *, const char *);\n");
	vsb_cat(sb, "const char * VRT_r_resp_http_(struct sess *);\n");
	vsb_cat(sb, "void VRT_l_resp_http_(struct sess *, const char *);\n");
}
