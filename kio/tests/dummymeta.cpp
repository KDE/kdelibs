#include <kgenericfactory.h>

#include "dummymeta.h"

K_EXPORT_COMPONENT_FACTORY( dummymeta, KGenericFactory<DummyMeta> )

DummyMeta::DummyMeta( QObject *parent, const char *name,
                      const QStringList &preferredItems )
    : KFilePlugin( parent, name, preferredItems )
{
    qDebug("---- DummyMeta::DummyMeta: got %i preferred items.", preferredItems.count());
}

bool DummyMeta::readInfo( KFileMetaInfo::Internal & info )
{
   qDebug("#### DummyMeta:: readInfo: %s", info.path().toLatin1().constData() );
   return 0L;
}

#include "dummymeta.moc"
