/* This file is part of the KDE libraries
   Copyright (c) 2000 Stephan Kulow <coolo@kde.org>
   Copyright (c) 2002 Dirk Mueller <mueller@kde.org>
   Copyright (c) 2002 Oswald Buddenhagen <ossi@kde.org>
   Copyright (c) 2003 Joseph Wenninger <kde@jowenn.at>
   Copyright (c) 2005 Jarosław Staniek <staniek@kde.org>
   Copyright (c) 2007 Rafael Fernández López <ereslibre@kde.org>

   unsetenv() taken from the GNU C Library.
   Copyright (C) 1992,1995-1999,2000-2002 Free Software Foundation, Inc. <gnu@gnu.org>

   getgrouplist() taken from the FreeBSD libc. The copyright notice
   in the file /usr/src/lib/libc/gen/getgrouplist.c on FreeBSD is out
   of date, as it has incorrect copyright years and still names the
   4-clause BSD license -- however, the Regents of the University of
   California at Berkeley have declared that clause 3 is void; only
   clauses 1,2 and 4 apply. See the full license text below.
   Copyright (c) 1991, 1993
        The Regents of the University of California.  All rights reserved.



   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kdecore_export.h>
#include <kdefakes.h>

#define KDE_open open
#define KDE_mkdir mkdir


#ifndef HAVE_GETGROUPLIST
/* Although this full license text is 4-clause BSD, it is taken directly
   from the FreeBSD source tree; the Regents of the University of
   California have deleted clause 3. See
      http://www.freebsd.org/copyright/license.html
   In addition, FreeBSD itself is now under a 2-clause BSD license,
   which strikes clause 4 itself. That makes the *effective* license
   on this file a 2-clause BSD license. I am including the complete
   file, unmodified except for the #if 0 which is needed to avoid
   the FreeBSD SCCS IDs.
*/
/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getgrouplist.c	8.2 (Berkeley) 12/8/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD: src/lib/libc/gen/getgrouplist.c,v 1.14 2005/05/03 16:20:03 delphij Exp $");
#endif

/*
 * get credential
 */
#include <sys/types.h>

#include <grp.h>
#include <string.h>
#include <unistd.h>

int
getgrouplist(const char *uname, gid_t agroup, gid_t *groups, int *grpcnt)
{
	const struct group *grp;
	int i, maxgroups, ngroups, ret;

	ret = 0;
	ngroups = 0;
	maxgroups = *grpcnt;
	/*
	 * When installing primary group, duplicate it;
	 * the first element of groups is the effective gid
	 * and will be overwritten when a setgid file is executed.
	 */
	groups[ngroups++] = agroup;
	if (maxgroups > 1)
		groups[ngroups++] = agroup;
	/*
	 * Scan the group file to find additional groups.
	 */
	setgrent();
	while ((grp = getgrent()) != NULL) {
		for (i = 0; i < ngroups; i++) {
			if (grp->gr_gid == groups[i])
				goto skip;
		}
		for (i = 0; grp->gr_mem[i]; i++) {
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups >= maxgroups) {
					ret = -1;
					break;
				}
				groups[ngroups++] = grp->gr_gid;
				break;
			}
		}
skip:
		;
	}
	endgrent();
	*grpcnt = ngroups;
	return (ret);
}
#endif

