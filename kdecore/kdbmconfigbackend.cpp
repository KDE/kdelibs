#include <config.h>

#include <sys/types.h>

#include <fcntl.h>
#include <limits.h>
#include <db.h>
#include <stdlib.h>
#include <unistd.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kstddirs.h"
#include "kglobal.h"
#include "kconfigbackend.h"
#include "kdbmconfigbackend.h"
#include "kdebug.h"
#include "kapp.h"

class KConfigDBMBackEndPrivate 
{
public:
  QMap <QString,QString> *global_dbs;
  DB *local_db;  
};


bool KConfigDBMBackEnd::parseConfigFiles()
{
	if (useKDEGlobals) {
		QStringList kdercs = KGlobal::dirs()->
		  findAllResources("config", QString::fromLatin1("kdeglobals"));

		kdercs += KGlobal::dirs()->
		  findAllResources("config", QString::fromLatin1("system.kdeglobals"));

		QStringList::ConstIterator it;

		for (it = kdercs.fromLast(); it != kdercs.end(); it--) {
			DB * aConfigFile = dbopen( (*it).latin1(), O_RDWR, 0600, DB_HASH, 0 );
			parseSingleConfigFile( aConfigFile, 0L, true );
			aConfigFile->close(aConfigFile);
		}

	}

	return true;
}

void CreateDBTFromQString(const QString &str, DBT &key)
{
	char *ascii_string = strdup(str.latin1());
	key.size = strlen(ascii_string);
	key.data = malloc(key.size);
	memcpy(key.data, ascii_string, key.size);
}

void CreateQStringFromDBT(QString &str, const DBT &key)
{
	char *DBT_string;
	DBT_string=new char(key.size+1);
	memset(DBT_string, 0, key.size+1);
	memcpy(DBT_string, key.data, key.size);
	str == QString::fromLatin1(const_cast<const char *>(DBT_string));
	delete DBT_string;
	return;
}

void CreateEntryFromDBT(KEntryKey &entry, const DBT &key)
{
	QString s_group, s_key;
	CreateQStringFromDBT(s_group, key);
	if (s_group[0] == '[') {
		s_key=s_group.mid(s_group.find("]")+1, s_group.length());
	} else
	  return; // Invalid key processed
	entry = KEntryKey(s_group, s_key);
	return;
}

void KConfigDBMBackEnd::parseSingleConfigFile(void *rFile,
                                           KEntryMap *pWriteBackMap,
                                           bool bGlobal)
{
	KEntryKey key;
	KEntry entry;
	DBT DBT_data, DBT_key;
	DB *db = static_cast<DB *>(rFile);
	QString value;

	if ( (db->seq)(db, &DBT_key, &DBT_data, R_FIRST)) {
		kdDebug() << "Nothing in the database given grr." << endl;
		return;
	}
	CreateEntryFromDBT(key, DBT_key);
	CreateQStringFromDBT(value, DBT_data);
	entry.aValue=value; entry.bGlobal=bGlobal;
	pConfig->putData(key, entry);

	while (!(db->seq)(db, &DBT_key, &DBT_data, R_NEXT)) {
		CreateEntryFromDBT(key, DBT_key);
		CreateQStringFromDBT(value, DBT_data);
		entry.aValue=value; entry.bGlobal=bGlobal;
		pConfig->putData(key, entry);
	}

	return;
}

KConfigBase::ConfigState KConfigDBMBackEnd::getConfigState() const
{
	if (fileName.isEmpty())
		return KConfigBase::NoAccess;

	QString aLocalFileName = KGlobal::dirs()->saveLocation("config") + fileName;

	// Can we allow the write? We can, if the program
	// doesn't run SUID. But if it runs SUID, we must
	// check if the user would be allowed to write if
	// it wasn't SUID.
	if (checkAccess(aLocalFileName, W_OK|R_OK))
		return KConfigBase::ReadWrite;
	else
	if (checkAccess(aLocalFileName, R_OK))
		return KConfigBase::ReadOnly;

	return KConfigBase::NoAccess;
}

void KConfigDBMBackEnd::sync(bool bMerge)
{
	return;
}

bool KConfigDBMBackEnd::writeConfigFile(QString filename, bool bGlobal,
                                        bool bMerge)
{
	DBT DBT_key, DBT_data;
	QString real_keyname;
	// is the config object read-only?
	if (pConfig->isReadOnly())
		return true; // pretend we wrote it

	DB *db = dbopen( filename.latin1(), O_RDWR, 0600, DB_HASH, 0 );
	KEntryMap aMap = pConfig->internalEntryMap();
	// augment this structure with the dirty entries from the config object

	for (KEntryMapIterator aInnerIt = aMap.begin();
		aInnerIt != aMap.end(); ++aInnerIt) {

		KEntry currentEntry = *aInnerIt;

		if (!currentEntry.bDirty)
			continue;

		if ( (currentEntry.bGlobal == bGlobal) ||
		     (bMerge) ) {
			KEntryKey entryKey = aInnerIt.key();
			real_keyname="[";
			real_keyname += entryKey.group;
			real_keyname += "]";
			real_keyname += entryKey.key;
			CreateDBTFromQString(real_keyname, DBT_key);
			CreateDBTFromQString((*aInnerIt).aValue, DBT_data);
			(db->del)(db, &DBT_key, 0);
			(db->put)(db, &DBT_key, &DBT_data, R_IAFTER);
		}

	}
	
}
