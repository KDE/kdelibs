#ifndef KCUPSPRINTERIMPL_H
#define KCUPSPRINTERIMPL_H

#include "kprinterimpl.h"

class KCupsPrinterImpl : public KPrinterImpl
{
public:
	KCupsPrinterImpl(QObject *parent = 0, const char *name = 0);
	~KCupsPrinterImpl();

	bool printFiles(KPrinter*, const QStringList&);
	void preparePrinting(KPrinter*);
	void broadcastOption(const QString& key, const QString& value);
};

#endif
