#include <kgenericfactory.h>
#include "klibloadertest_module.h"

typedef KGenericFactory<KLibLoaderTestObject> KLibLoaderTestFactory;
K_EXPORT_COMPONENT_FACTORY( libklibloadertestmodule, KLibLoaderTestFactory("klibloadertestfactory") )

KLibLoaderTestObject::KLibLoaderTestObject( QObject* parent, const char*, const QStringList& )
    : QObject( parent )
{
    qDebug( "KLibLoaderTestObject created" );
}
