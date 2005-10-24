#ifndef KLIBLOADERTEST_MODULE_H
#define KLIBLOADERTEST_MODULE_H

#include <qobject.h>
class KLibLoaderTestObject : public QObject
{
public:
    KLibLoaderTestObject( QObject* parent, const char*, const QStringList& );

};

#endif
