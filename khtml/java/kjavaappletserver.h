// -*- c++ -*-

#ifndef KJAVAAPPLETSERVER_H
#define KJAVAAPPLETSERVER_H

#include <qobject.h>
#include <kjavaprocess.h>

/**
 * @short Communicates with a KJAS server to display and control Java applets.
 *
 *
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaAppletServer : public QObject
{
Q_OBJECT

public:
    /**
     * Create the applet server.
     */
    KJavaAppletServer();

    /**
     * A factory method that returns the default server. This is the way this
     * class is usually instantiated.
     */
    static KJavaAppletServer *getDefaultServer();
    
    /**
     * Create an applet context with the specified id.
     */
    void createContext( int contextId );

    /**
     * Create an applet in the specified context with the specified id. The applet
     * name, class etc. are specified in the same way as in the HTML APPLET tag.
     */
    void createApplet( int contextId, int appletId,
		       const QString name, const QString clazzName,
		       const QString base );
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
     * Shut down the KJAS server.
     */
    void quit();

protected:
    KJavaProcess *process;
};

#endif // KJAVAAPPLETSERVER_H
