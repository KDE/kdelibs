#ifndef KMCUPSFACTORY_H
#define KMCUPSFACTORY_H

#include <klibloader.h>

class KCupsFactory : public KLibFactory
{
public:
	KCupsFactory(QObject *parent = 0, const char *name = 0);
	virtual ~KCupsFactory();

protected:
	QObject* createObject(QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList& args = QStringList());
};

#endif
