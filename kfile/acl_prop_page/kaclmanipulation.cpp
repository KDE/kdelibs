// $Id$

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

#include <qlist.h>
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
	for (int i=0; i < acl->acl_cnt; i++) {
		e = KACLEntry::GetACLEntry(acl->acl_entry[i]);
		if (e)
			entries.append(e);
	}

	acl_free(acl);
	return true;
}

void KACLList::PrintList ()
{
	KACLEntry *e;
	for (e = entries.first(); e != 0; e = entries.next())
		PrintItem(e);
}

void KACLList::PrintItem (const KACLEntry *e)
{
#if 0
	if (!e) {
		kdError() << "Null E passed!" << endl;
		return;
	}

	if (e->ignore) {
		return;
	}

	switch (e->tag) {
		case KACLEntry::USER:
			cout << "User: " << e->qualifier << endl;
			break;
		case KACLEntry::GROUP:
			cout << "Group: " << e->qualifier << endl;
			break;
		case KACLEntry::MASK:
			cout << "Max permissions: " << endl;
			break;
	}
	cout << "\t";

	if (e->access_r)
		cout << "read, ";
	if (e->access_w)
		cout << "write, ";
	if (e->access_x)
		cout << "execute";
	cout << endl;
#endif
}

KACLEntry *KACLEntry::GetACLEntry (acl_entry &acl)
{
	KACLEntry *e = new KACLEntry;
	acl_tag_t tag;
	uid_t *uid; gid_t *gid;

	e->ignore = e->error = false;

	if (!acl_get_tag_type(&acl, &tag)) {
		switch (tag) {
			case ACL_USER_OBJ:
			case ACL_USER: {
				e->tag = KACLEntry::USER;
				uid = static_cast<uid_t *>(acl_get_qualifier(&acl));
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
				gid = static_cast<gid_t *>(acl_get_qualifier(&acl));
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
	if (!acl_get_permset(&acl, &perms)) {
#if defined(ACL_READ)
		e->access_r = (*perms & ACL_READ);
		e->access_w = (*perms & ACL_WRITE);
		e->access_x = (*perms & ACL_EXECUTE);
#else
		e->access_r = (*perms & ACL_PERM_READ);
		e->access_w = (*perms & ACL_PERM_WRITE);
		e->access_x = (*perms & ACL_PERM_EXEC);
#endif
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
