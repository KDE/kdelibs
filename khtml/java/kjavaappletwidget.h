// -*- c++ -*-

#ifndef KJAVAAPPLETWIDGET_H
#define KJAVAAPPLETWIDGET_H

#include <qwidget.h>
#include <kjavaappletcontext.h>
#include <kjavaapplet.h>
#include <kwm.h>

class KWinModule;

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
 * <P>
 * Once you have created the applet widget, you should call the various setXXX
 * methods to configure it, they pretty much correspond to the HTML tags used
 * to embed applets in a web page. Once the applet is configured call the
 * create() method to set things in motion. The applet is running when it
 * first appears, but you can start or stop it when you like (for example
 * if it scrolls off the screen).
 * <P>
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
 * <P>
 * Note that the KJAS protocol is not yet stable - it will certainly change for
 * a while before settling down. This will not affect you unless you use the
 * KJAS protocol directly.
 * <P>
 * The window swallowing code used in this widget is based on KSwallowWidget by
 * Matthias Hoelzer.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.2  1999/10/09 11:20:55  rich
 * Const clean and no longer needs KWMModuleApp
 *
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaAppletWidget : public QWidget
{
Q_OBJECT

public:
   KJavaAppletWidget( KJavaAppletContext *context,
                      QWidget *parent=0, const char *name=0 );

   KJavaAppletWidget( KJavaApplet *applet,
                      QWidget *parent=0, const char *name=0 );

   KJavaAppletWidget( QWidget *parent=0, const char *name=0 );

   //
   // Stuff to do with the applet
   //

   void setAppletName( const QString appletName );
   const QString appletName();

   /**
    * Specify the name of the class file to run. For example 'Lake.class'.
    */
   void setAppletClass( const QString clazzName );

   /**
    * Get the name of the class file to run. For example 'Lake.class'.
    */
   const QString appletClass();

   /**
    * Specify the location of the jar file containing the class.
    * (unimplemented)
    */
   void setJARFile( const QString jar );

   /**
    * Get the location of the jar file containing the class.
    * (unimplemented)
    */
   const QString jarFile();

   /**
    * Specify a parameter to be passed to the applet.
    */
   void setParameter( const QString name, const QString value );

   /**
    * Get the value of a parameter to be passed to the applet.
    */
   const QString parameter( const QString name );

   /**
    * Set the URL of the document embedding the applet.
    */
   void setBaseURL( const QString base );

   /**
    * Get the URL of the document embedding the applet.
    */
   const QString baseURL();

   /**
    * Create the applet.
    */
   void create();

   /**
    * Overridden to make sure the applet is created.
    */
   virtual void show();

   /**
    * Run the applet.
    */
   void start();

   /**
    * Pause the applet.
    */
   void stop();

protected slots:
    //
    // Stuff to do with swallowing the applets Frame
    //

    void setWindow( WId w );
    void showApplet();

protected:
   void swallowWindow( WId w );
   //   void sendClientMessage( Window w, Atom a, long x );
   void focusInEvent( QFocusEvent * );
   void resizeEvent(QResizeEvent *);
   void closeEvent(QCloseEvent *);
   void uniqueTitle();

private:
    // Applet info
    KJavaApplet *applet;
    bool shown;

    /** Used to find out when the applet window is mapped. */
    KWinModule *kwm;

    // Swallowing info
    WId window;
    QString swallowTitle;
};

#endif // KJAVAAPPLETWIDGET_H

