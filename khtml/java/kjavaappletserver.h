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

protected:
    KJavaProcess *process;
    struct KJavaAppletServerPrivate *d;

    /**
     * Create the applet server.
     */
    KJavaAppletServer();
    ~KJavaAppletServer();
private:
    void setupJava( KJavaProcess *p );
};

#endif // KJAVAAPPLETSERVER_H

/**
 *
 * $Log$
 * Revision 1.6  2000/03/21 03:44:44  rogozin
 *
 * New Java support has been merged.
 *
 * Revision 1.5  2000/01/27 23:41:56  rogozin
 * All applet parameters are passed to KJAS now
 * Next step - make use of them.
 *
 * Revision 1.4  1999/12/14 19:56:59  rich
 * Many fixes, see changelog
 *
 * Revision 1.3  1999/11/12 01:22:34  rich
 * Now trys adding a / to the code base if the class loader could not find the applet class file. Fixed applet start/stop
 *
 * Revision 1.2  1999/10/09 18:10:10  rich
 * Const QString fixes
 *
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 */
