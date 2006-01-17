#ifndef __help_h__
#define __help_h__


#include <sys/types.h>
#include <sys/stat.h>


#include <stdio.h>
#include <unistd.h>


#include <qstring.h>


#include <kio/global.h>
#include <kio/slavebase.h>

class HelpProtocol : public KIO::SlaveBase
{
public:

    HelpProtocol( bool ghelp, const QByteArray &pool, const QByteArray &app);
    virtual ~HelpProtocol() { }

    virtual void get( const KUrl& url );

    virtual void mimetype( const KUrl& url );

private:

    QString langLookup(const QString &fname);
    void emitFile( const KUrl &url );
    void get_file( const KUrl& url );
    QString lookupFile(const QString &fname, const QString &query,
                       bool &redirect);

    void unicodeError( const QString &t );

    QString mParsed;
    bool mGhelp;
};


#endif
