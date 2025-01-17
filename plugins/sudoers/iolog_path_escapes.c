/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2011-2015 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

#include "sudoers.h"
#include "sudo_iolog.h"

static size_t
fill_seq(char *str, size_t strsize, void *v)
{
#ifdef SUDOERS_NO_SEQ
    debug_decl(fill_seq, SUDOERS_DEBUG_UTIL);
    debug_return_size_t(strlcpy(str, "%{seq}", strsize));
#else
    char *logdir = v;
    static char sessid[7];
    int len;
    debug_decl(fill_seq, SUDOERS_DEBUG_UTIL);

    if (sessid[0] == '\0') {
	if (!iolog_nextid(logdir, sessid))
	    debug_return_size_t((size_t)-1);
    }

    /* Path is of the form /var/log/sudo-io/00/00/01. */
    len = snprintf(str, strsize, "%c%c/%c%c/%c%c", sessid[0],
	sessid[1], sessid[2], sessid[3], sessid[4], sessid[5]);
    if (len < 0)
	debug_return_size_t(strsize); /* handle non-standard snprintf() */
    debug_return_size_t((size_t)len);
#endif /* SUDOERS_NO_SEQ */
}

static size_t
fill_user(char *str, size_t strsize, void *unused)
{
    debug_decl(fill_user, SUDOERS_DEBUG_UTIL);
    debug_return_size_t(strlcpy(str, user_ctx.name, strsize));
}

static size_t
fill_group(char *str, size_t strsize, void *unused)
{
    struct group *grp;
    size_t len;
    debug_decl(fill_group, SUDOERS_DEBUG_UTIL);

    if ((grp = sudo_getgrgid(user_ctx.gid)) != NULL) {
	len = strlcpy(str, grp->gr_name, strsize);
	sudo_gr_delref(grp);
    } else {
	len = (size_t)snprintf(str, strsize, "#%u", (unsigned int)user_ctx.gid);
    }
    debug_return_size_t(len);
}

static size_t
fill_runas_user(char *str, size_t strsize, void *unused)
{
    debug_decl(fill_runas_user, SUDOERS_DEBUG_UTIL);
    debug_return_size_t(strlcpy(str, runas_ctx.pw->pw_name, strsize));
}

static size_t
fill_runas_group(char *str, size_t strsize, void *unused)
{
    struct group *grp;
    size_t len;
    debug_decl(fill_runas_group, SUDOERS_DEBUG_UTIL);

    if (runas_ctx.gr != NULL) {
	len = strlcpy(str, runas_ctx.gr->gr_name, strsize);
    } else {
	if ((grp = sudo_getgrgid(runas_ctx.pw->pw_gid)) != NULL) {
	    len = strlcpy(str, grp->gr_name, strsize);
	    sudo_gr_delref(grp);
	} else {
	    len = (size_t)snprintf(str, strsize, "#%u",
		(unsigned int)runas_ctx.pw->pw_gid);
	}
    }
    debug_return_size_t(len);
}

static size_t
fill_hostname(char *str, size_t strsize, void *unused)
{
    debug_decl(fill_hostname, SUDOERS_DEBUG_UTIL);
    debug_return_size_t(strlcpy(str, user_ctx.shost, strsize));
}

static size_t
fill_command(char *str, size_t strsize, void *unused)
{
    debug_decl(fill_command, SUDOERS_DEBUG_UTIL);
    debug_return_size_t(strlcpy(str, user_ctx.cmnd_base, strsize));
}

/* Note: "seq" must be first in the list. */
static const struct iolog_path_escape path_escapes[] = {
    { "seq", fill_seq },
    { "user", fill_user },
    { "group", fill_group },
    { "runas_user", fill_runas_user },
    { "runas_group", fill_runas_group },
    { "hostname", fill_hostname },
    { "command", fill_command },
    { NULL, NULL }
};
const struct iolog_path_escape *sudoers_iolog_path_escapes = path_escapes;
