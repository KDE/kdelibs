#ifndef KMDRIVERDB_H
#define KMDRIVERDB_H

#include <qobject.h>
#include <qlist.h>
#include <qdict.h>
#include <qstring.h>

#include "kmdbentry.h"

class KMDBCreator;

class KMDriverDB : public QObject
{
	Q_OBJECT
public:
	static KMDriverDB* self();

	KMDriverDB(QObject *parent = 0, const char *name = 0);
	~KMDriverDB();

	void init(QWidget *parent = 0);
	KMDBEntryList* findEntry(const QString& manu, const QString& model);
	KMDBEntryList* findPnpEntry(const QString& manu, const QString& model);
	QDict<KMDBEntryList>* findModels(const QString& manu);
	const QDict< QDict<KMDBEntryList> >& manufacturers() const	{ return m_entries; }

protected:
	void loadDbFile();
	void insertEntry(KMDBEntry *entry);
	QString dbFile();

protected slots:
	void slotDbCreated();

signals:
	void dbLoaded(bool reloaded);

private:
	KMDBCreator			*m_creator;
	QDict< QDict<KMDBEntryList> >	m_entries;
	QDict< QDict<KMDBEntryList> >	m_pnpentries;

	static KMDriverDB	*m_self;
};

#endif
