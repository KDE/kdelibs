#ifndef __help_h__
#define __help_h__


#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <unistd.h>


#include <qintdict.h>
#include <qstring.h>
#include <qvaluelist.h>


#include <kio/global.h>
#include <kio/slavebase.h>

class GnomeHelpProtocol : public KIO::SlaveBase
{
public:

    GnomeHelpProtocol( const QCString &pool, const QCString &app);
    virtual ~GnomeHelpProtocol() { }

    virtual void get( const KURL& url );

    virtual void mimetype( const KURL& url );

private:

    void get_file( const KURL& url );

    void notFound();
    QString mParsed;
};


#endif
