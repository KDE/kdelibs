// -*- c++ -*-

#ifndef KJAVAAPPLETCONTEXT_H
#define KJAVAAPPLETCONTEXT_H

#include <qobject.h>
#include <kurl.h>

/**
 * @short Provides a context for KJavaAppletWidgets
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */


class KJavaAppletServer;
class KJavaApplet;

class KJavaAppletContext : public QObject
{
Q_OBJECT

public:
    /**
     * If server is zero then the default server is used.
     */
    KJavaAppletContext();
    ~KJavaAppletContext();

    /**
     * Returns a singleton context. This should only be used for simple
     * purposes, more complex applications require multiple contexts.
     */
    static KJavaAppletContext* getDefaultContext();

    int  contextId();
    void setContextId( int id );

    // Forwarded from the applets
    void create  ( KJavaApplet* );
    void destroy ( KJavaApplet* );
    void init    ( KJavaApplet* );
    void start   ( KJavaApplet* );
    void stop    ( KJavaApplet* );

    // use this for applet call backs instead of the slot
    void processCmd( QString cmd, QStringList args );

signals:
    void showStatus  ( const QString& txt );
    void showDocument( const QString& url, const QString& target );

protected:
     //* The applet server this context is attached to.
     KJavaAppletServer* server;

protected slots:
     void received( const QString& cmd, const QStringList& arg );

private:
     int id;
     struct KJavaAppletContextPrivate* d;

};

#endif // KJAVAAPPLETCONTEXT_H
