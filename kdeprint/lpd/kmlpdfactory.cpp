#include "kmlpdfactory.h"
#include "kmlpdmanager.h"
#include "klpdprinterimpl.h"

extern "C"
{
	void* init_libkdeprint_lpd()
	{
		return new KLpdFactory;
	}
};

KLpdFactory::KLpdFactory(QObject *parent, const char *name)
: KLibFactory(parent,name)
{
}

KLpdFactory::~KLpdFactory()
{
}

QObject* KLpdFactory::createObject(QObject *parent, const char *name, const char *classname, const QStringList&)
{
	if (strcmp(classname,"KMManager") == 0)
		return new KMLpdManager(parent,name);
	else if (strcmp(classname,"KPrinterImpl") == 0)
		return new KLpdPrinterImpl(parent,name);
	else
		return NULL;
}
