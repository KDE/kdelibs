// -*- c++ -*-

#ifndef KJAVAAPPLETWIDGET_H
#define KJAVAAPPLETWIDGET_H

#include "kjavaappletcontext.h"
#include "kjavaapplet.h"
#include <qxembed.h>


/**
 * @short A widget for displaying Java applets
 *
 * KJavaAppletWidget provides support for the inclusion of Java applets
 * in Qt and KDE applications. To create an applet, you must first create
 * a context object in which it will run. There can be several applets and
 * contexts in operation at a given time, for example in a web browser there
 * would be one context object for each web page. Applets in the same context
 * can communicate with each other, applets in different contexts cannot. (Well
 * actually, they can, but only via some very evil tricks). Once you have
 * created a KJavaAppletContext, you can create as many applets in it as you
 * want.
 *
 * Once you have created the applet widget, you should call the various setXXX
 * methods to configure it, they pretty much correspond to the HTML tags used
 * to embed applets in a web page. Once the applet is configured call the
 * create() method to set things in motion. The applet is running when it
 * first appears, but you can start or stop it when you like (for example
 * if it scrolls off the screen).
 *
 * This widget works by firing off a Java server process with which it
 * communicates using the KDE Java Applet Server (KJAS) protocol via a pipe.
 * The applet windows are swallowed and attached to the QWidget, but they are
 * actually running in a different process. This has the advantage of robustness
 * and reusability. The details of the communication are hidden from the user
 * in the KJASClient class. Normally only a single server process is used for
 * all of the applets in a given application, this is all sorted automatically.
 * The KJAS server is 100% pure Java, and should also prove useful for people
 * wishing to add java support to other systems (for example a perl/Tk binding
 * is perfectly feasible). All you need to do is implement the protocol and
 * (optionally) swallow the applet windows.
 *
 * Note that the KJAS protocol is not yet stable - it will certainly change for
 * a while before settling down. This will not affect you unless you use the
 * KJAS protocol directly.
 *
 * @author Richard J. Moore, rich@kde.org
 */

class KWinModule; 
 
class KJavaAppletWidget : public QXEmbed
{
Q_OBJECT

public:
    KJavaAppletWidget( KJavaAppletContext *context,
                       QWidget *parent=0, const char *name=0 );

    KJavaAppletWidget( KJavaApplet *applet,
                       QWidget *parent=0, const char *name=0 );

    KJavaAppletWidget( QWidget *parent=0, const char *name=0 );

   ~KJavaAppletWidget();

    //
    // Stuff to do with the applet
    //
    void setAppletName( const QString &appletName );
    QString &appletName();

    /**
     * Specify the name of the class file to run. For example 'Lake.class'.
     */
    void setAppletClass( const QString &clazzName );

    /**
     * Get the name of the class file to run. For example 'Lake.class'.
     */
    QString &appletClass();

    /**
     * Specify the location of the jar file containing the class.
     * (unimplemented)
     */
    void setJARFile( const QString &jar );

    /**
     * Get the location of the jar file containing the class.
     * (unimplemented)
     */
    QString &jarFile();

    /**
     * Specify a parameter to be passed to the applet.
     */
    void setParameter( const QString &name, const QString &value );

    /**
     * Get the value of a parameter to be passed to the applet.
     */
    QString &parameter( const QString &name );

    /**
     * Set the URL of the document embedding the applet.
     */
    void setBaseURL( const QString &base );

    /**
     * Get the URL of the document embedding the applet.
     */
    QString &baseURL();

   /**
     * Set the codebase of the applet classes.
     */
    void setCodeBase( const QString &codeBase );

    /**
     * Get the codebase of the applet classes.
     */
    QString &codeBase();

    /**
     * Create the applet.
     */
    void create();

    /**
     * Shows applet on the screen
     */
    void showApplet();

    /**
     * Run the applet.
     */
    void start();

    /**
     * Pause the applet.
     */
    void stop();

    void resize( int, int );
  
protected slots:
     //
     // Stuff to do with swallowing the applets Frame
     //
     void setWindow( WId w );

protected:
    void uniqueTitle();

private:
    // Applet info
    KJavaApplet *applet;
    bool shown;

    /** Used to find out when the applet window is mapped. */
    KWinModule *kwm;

    // Swallowing info
    QString swallowTitle;

    struct KJavaAppletWidgetPrivate *d;

    void init();
};

#endif // KJAVAAPPLETWIDGET_H

