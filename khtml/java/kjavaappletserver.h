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
     * Create the applet server.  These shouldn't be used directly,
     * use allocateJavaServer instead
     */
    KJavaAppletServer();
    ~KJavaAppletServer();

    /**
     * A factory method that returns the default server. This is the way this
     * class is usually instantiated.
     */
    static KJavaAppletServer *allocateJavaServer();
    static void               freeJavaServer();
    static QString getAppletLabel();

    /**
     * Create an applet context with the specified id.
     */
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
                       const QString jarFile, QSize size,
                       const QMap< QString, QString >& params,
                       const QString windowTitle );

    void initApplet( int contextId, int appletId );

    /**
     * Destroy an applet in the specified context with the specified id.
     */
    void destroyApplet( int contextId, int appletId );

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

    QString appletLabel();

protected:
    void setupJava( KJavaProcess *p );

    KJavaProcess *process;
    struct KJavaAppletServerPrivate *d;

protected slots:
    void received( const QByteArray& qb );
    void checkShutdown();

};

#endif // KJAVAAPPLETSERVER_H
