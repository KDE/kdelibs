#ifndef KACLMANIP_H
#define KACLMANIP_H "$Id$"
#endif

#include <sys/types.h>
#include <sys/acl.h>

#include <qlist.h>
#include <qstring.h>

namespace KPOSIX1e {

class KACLEntry
{
public:
	enum TAG{
		USER,
		GROUP,
		MASK,
		OTHER
	};
	TAG tag;
	QString qualifier;
	bool access_r, access_w, access_x;
	bool error, ignore;
	static KACLEntry *GetACLEntry (acl_entry_t acl);
};

class KACLList
{
public:
	KACLList ();
	KACLList (const QCString &path);

	bool ReadACL ();
	bool WriteACL ();
	void SetPath (const QCString &);

	void PrintList ();
	void PrintItem (const KACLEntry *);
	QList<KACLEntry> entries;

protected:
	QCString path;
};

};
