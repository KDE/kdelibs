/*
 * Copyright (c) 2000,2001 Alex Zepeda <jazepeda@pacbell.net>
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
 * 3. Redistributions of source code or in binary form must consent to
 *    future terms and conditions as set forth by the founding author(s).
 *    The founding author is defined as the creator of following code, or
 *    lacking a clearly defined creator, the founding author is defined as
 *    the first person to claim copyright to, and contribute significantly
 *    to the following code.
 * 4. The following code may be used without explicit consent in any
 *    product provided the previous three conditions are met, and that
 *    the following source code be made available at no cost to consumers
 *    of mentioned product and the founding author as defined above upon
 *    request.  This condition may at any time be waived by means of 
 *    explicit written consent from the founding author.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/acl.h>

#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <stdio.h>

#include <kdebug.h>

#include <qptrlist.h>
#include <qstring.h>

#include <iostream.h>

#include "kaclmanipulation.h"

using namespace KPOSIX1e;

KACLList::KACLList ()
{
}

KACLList::KACLList (const QCString &_path)
{
	SetPath(_path);
	ReadACL();
}

KACLList::~KACLList () 
{
}

void KACLList::SetPath (const QCString &_path)
{
	this->path = _path;
}


bool KACLList::WriteACL ()
{
	acl_t acl = acl_init(3);

	acl_set_file(path, ACL_TYPE_ACCESS, acl);
	acl_free(acl);
	return true;
}

bool KACLList::ReadACL ()
{
	entries.clear();

	acl_t acl;
	acl = acl_get_file(path, ACL_TYPE_ACCESS);
	if (!acl) {
		kdDebug() << "acl_get_file failed " << strerror(errno) << endl;
		return false;
	}
	
	KACLEntry *e;
	acl_entry_t acl_entry;
	int entry_id = ACL_FIRST_ENTRY;

	while (acl_get_entry(acl, entry_id, &acl_entry) == 1) {
		entry_id = ACL_NEXT_ENTRY;
		e = KACLEntry::GetACLEntry(acl_entry);
		if (e)
			entries.append(e);
	}

	acl_free(acl);
	return true;
}

KACLEntry *KACLEntry::GetACLEntry (acl_entry_t acl)
{
	KACLEntry *e = new KACLEntry;
	acl_tag_t tag;
	uid_t *uid; gid_t *gid;

	e->ignore = e->error = false;

	if (!acl_get_tag_type(acl, &tag)) {
		switch (tag) {
			case ACL_USER_OBJ:
			case ACL_USER: {
				e->tag = KACLEntry::USER;
				uid = static_cast<uid_t *>(acl_get_qualifier(acl));
				if (uid) {
					struct passwd *pw = getpwuid(*uid);
					if (pw) {
						e->qualifier = QString::fromLatin1(pw->pw_name);
					} else {
						e->qualifier.setNum(*uid);
					}
				} else {
					e->qualifier = QString::null;
				}
				break;
			}

			case ACL_GROUP_OBJ:
			case ACL_GROUP: {
				e->tag = KACLEntry::GROUP;
				gid = static_cast<gid_t *>(acl_get_qualifier(acl));
				if (gid) {
					struct group *gr = getgrgid(*gid);
					if (gr) {
						e->qualifier = QString::fromLatin1(gr->gr_name);
					} else {
						e->qualifier.setNum(*gid);
					}
				} else {
					e->qualifier = QString::null;
				}
				break;
			}

			case ACL_MASK: {
				e->qualifier = "";
				e->tag = KACLEntry::MASK;
				break;
			}

			case ACL_OTHER: {
//			case ACL_OTHER_OBJ: /* this shouldn't even exist */
				e->tag = KACLEntry::OTHER;
				e->qualifier = QString::null;
			}
				break;

			default:
				e->error = true;
				kdError() << "Unknown tag type returned by acl_get_tag_type" << endl;
				break;
		}
	} else {
		e->error = true;
		kdDebug() << "acl_get_tag_type failed: " << strerror(errno) << endl;
	}

	acl_permset_t perms;
	if (!acl_get_permset(acl, &perms)) {
		e->access_r = (*perms & ACL_READ);
		e->access_w = (*perms & ACL_WRITE);
		e->access_x = (*perms & ACL_EXECUTE);
	} else {
		kdDebug() << "acl-get_permset failed: " << strerror(errno) << endl;
		e->error = true;
	}

	if (e->error) {
		delete e;
		return 0;
	}
	return e;
}

#if 0
int main(int argc, char **argv)
{
	if (argc <= 1) {
		fprintf(stderr, "Must specify a filename\n");
		fflush(stderr);
		return 2;
	}

	KPOSIXACLEntity e(argv[1]);
	e.PrintList();
	return 0;
}
#endif
