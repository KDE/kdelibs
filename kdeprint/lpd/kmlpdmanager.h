#ifndef KMLPDMANAGER_H
#define KMLPDMANAGER_H

#include "kmmanager.h"
#include <qdict.h>

class PrintcapEntry;

class KMLpdManager : public KMManager
{
public:
	KMLpdManager(QObject *parent = 0, const char *name = 0);
	~KMLpdManager();

	bool completePrinterShort(KMPrinter*);
	bool completePrinter(KMPrinter*);

protected:
	void listPrinters();
	void loadPrintcapFile(const QString& filename);

private:
	QDict<PrintcapEntry>	m_entries;
};

#endif
