#ifndef DUMMYMETA_H
#define DUMMYMETA_H

#include <kfilemetainfo.h>

class KFileMetaInfo;

class DummyMeta : public KFilePlugin
{
    Q_OBJECT
    
public:
    DummyMeta( QObject *parent, const char *name, const QStringList &args );
    ~DummyMeta() {}

    virtual bool readInfo( KFileMetaInfo::Internal& info );

};

#endif
