#ifndef KMDBENTRY_H
#define KMDBENTRY_H

#include <qstring.h>
#include <qlist.h>

struct KMDBEntry
{
	// the file location of the driver
	QString	file;
	// normal informations
	QString	manufacturer;
	QString	model;
	QString	modelname;
	// informations used for auto-detection
	QString	pnpmanufacturer;
	QString	pnpmodel;
	// short driver description (if any)
	QString	description;

	bool validate();
};

typedef QList<KMDBEntry>		KMDBEntryList;
typedef QListIterator<KMDBEntry>	KMDBEntryListIterator;

#endif
