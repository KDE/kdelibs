#include "kmcupsfactory.h"
#include "kmcupsmanager.h"
#include "kmcupsjobmanager.h"
#include "kmcupsuimanager.h"
#include "kcupsprinterimpl.h"

extern "C"
{
	void* init_libkdeprint_cups()
	{
		return new KCupsFactory;
	}
};

KCupsFactory::KCupsFactory(QObject *parent, const char *name)
: KLibFactory(parent,name)
{
}

KCupsFactory::~KCupsFactory()
{
}

QObject* KCupsFactory::createObject(QObject *parent, const char *name, const char *classname, const QStringList&)
{
	if (strcmp(classname,"KMManager") == 0)
		return new KMCupsManager(parent,name);
	else if (strcmp(classname,"KMJobManager") == 0)
		return new KMCupsJobManager(parent,name);
	else if (strcmp(classname,"KMUiManager") == 0)
		return new KMCupsUiManager(parent,name);
	else if (strcmp(classname,"KPrinterImpl") == 0)
		return new KCupsPrinterImpl(parent,name);
	else
		return NULL;
}
