#ifndef KMLPDFACTORY_H
#define KMLPDFACTORY_H

#include <klibloader.h>

class KLpdFactory : public KLibFactory
{
public:
	KLpdFactory(QObject *parent = 0, const char *name = 0);
	virtual ~KLpdFactory();

protected:
	QObject* createObject(QObject *parent = 0, const char *name = 0, const char *classname = "QObject", const QStringList& args = QStringList());
};

#endif
