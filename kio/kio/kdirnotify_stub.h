/****************************************************************************
**
** DCOP Stub Definition created by dcopidl2cpp from kdirnotify.kidl
**
** WARNING! All changes made in this file will be lost!
**
*****************************************************************************/

#ifndef __KDIRNOTIFY_STUB__
#define __KDIRNOTIFY_STUB__

#include <dcopstub.h>
#include <dcopobject.h>
#include <kurl.h>

class KIO_EXPORT KDirNotify_stub : virtual public DCOPStub
{
public:
    KDirNotify_stub( const DCOPCString& app, const DCOPCString& id );
    KDirNotify_stub( DCOPClient* client, const DCOPCString& app, const DCOPCString& id );
    explicit KDirNotify_stub( const DCOPRef& ref );
    virtual ASYNC FilesAdded( const KURL& directory );
    virtual ASYNC FilesRemoved( const KURL::List& fileList );
    virtual ASYNC FilesChanged( const KURL::List& fileList );
    virtual ASYNC FileRenamed( const KURL& src, const KURL& dst );
protected:
    KDirNotify_stub() : DCOPStub( never_use ) {}
};


#endif
