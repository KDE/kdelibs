// -*- c++ -*-

#ifndef KJAVAAPPLETSERVER_H
#define KJAVAAPPLETSERVER_H

#include <qobject.h>
#include <kjavaprocess.h>

/**
 * @short Communicates with a KJAS server to display and control Java applets.
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */

class KJavaAppletContext;

class KJavaAppletServer : public QObject
{
Q_OBJECT

public:
    /**
     * A factory method that returns the default server. This is the way this
     * class is usually instantiated.
     */
    static KJavaAppletServer *allocateJavaServer();
    static void               freeJavaServer();

    /**
     * Create an applet context with the specified id.
     */
    void createContext( int contextId );
    void createContext( int contextId, KJavaAppletContext* context );

    /**
     * Destroy the applet context with the specified id. All the applets in the
     * context will be destroyed as well.
     */
    void destroyContext( int contextId );

    /**
     * Create an applet in the specified context with the specified id. The applet
     * name, class etc. are specified in the same way as in the HTML APPLET tag.
     */
    void createApplet( int contextId, int appletId,
                       const QString name, const QString clazzName,
                       const QString baseURL, const QString codeBase,
                       const QString jarFile, QSize size );

    /**
     * Destroy an applet in the specified context with the specified id.
     */
    void destroyApplet( int contextId, int appletId );

    /**
     * Set a parameter for a specified applet. The parameter and value are both
     * treated as strings (though the string might of course be a number e.g. "1".
     */
    void setParameter( int contextId, int appletId,
                       const QString name, const QString value );

    /**
     * Display the specified applet and set the window title as specified.
     */
    void showApplet( int contextId, int appletId,
                     const QString title );

    /**
     * Start the specified applet.
     */
    void startApplet( int contextId, int appletId );

    /**
     * Stop the specified applet.
     */
    void stopApplet( int contextId, int appletId );

    /**
     * Shut down the KJAS server.
     */
    void quit();

signals:
    /**
     * Command received from java vm
     */
    void receivedCommand( const QString &cmd, const QStringList &arg );

protected:
    KJavaProcess *process;
    struct KJavaAppletServerPrivate *d;

    /**
     * Create the applet server.
     */
    KJavaAppletServer();
    ~KJavaAppletServer();

protected slots:
    void received( const QString &s );
    void received( const QByteArray& qb );
    void checkShutdown();

private:
    void setupJava( KJavaProcess *p );
};

#endif // KJAVAAPPLETSERVER_H
