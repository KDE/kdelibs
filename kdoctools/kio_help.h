#ifndef __help_h__
#define __help_h__


#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <unistd.h>


#include <q3intdict.h>
#include <qstring.h>
#include <q3valuelist.h>


#include <kio/global.h>
#include <kio/slavebase.h>

class HelpProtocol : public KIO::SlaveBase
{
public:

    HelpProtocol( bool ghelp, const QByteArray &pool, const Q3CString &app);
    virtual ~HelpProtocol() { }

    virtual void get( const KURL& url );

    virtual void mimetype( const KURL& url );

private:

    QString langLookup(QString fname);
    void emitFile( const KURL &url );
    void get_file( const KURL& url );
    QString lookupFile(const QString &fname, const QString &query,
                       bool &redirect);

    void unicodeError( const QString &t );

    QString mParsed;
    bool mGhelp;
};


#endif
