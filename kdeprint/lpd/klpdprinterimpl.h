#ifndef KLPDPRINTERIMPL_H
#define KLPDPRINTERIMPL_H

#include "kprinterimpl.h"

class KLpdPrinterImpl : public KPrinterImpl
{
public:
	KLpdPrinterImpl(QObject *parent = 0, const char *name = 0);
	~KLpdPrinterImpl();

	bool printFiles(KPrinter*, const QStringList&);

protected:
	QString executable();
};

#endif
