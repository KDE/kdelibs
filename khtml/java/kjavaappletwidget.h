// -*- c++ -*-

#ifndef KJAVAAPPLETWIDGET_H
#define KJAVAAPPLETWIDGET_H

#include <qwidget.h>
#include <kjavaappletcontext.h>
#include <kjavaapplet.h>

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
 * The window swallowing code used in this widget is based on KSwallowWidget by
 * Matthias Hoelzer.
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

  void resize( int, int );
  
protected slots:
    //
    // Stuff to do with swallowing the applets Frame
    //

    void setWindow( WId w );
    void showApplet();

protected:
   void swallowWindow( WId w );
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

  struct KJavaAppletWidgetPrivate *d;
};

#endif // KJAVAAPPLETWIDGET_H

/*
 * $Log$
 * Revision 1.8  2000/06/08 22:40:15  pfeiffer
 * s/kwm/kwin/
 *
 * Revision 1.7  2000/06/06 22:53:36  gehrmab
 * Beauty and wellness for the API documentation
 *
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
 * Revision 1.3  1999/10/09 13:20:45  rich
 * Changed Wid to WId
 *
 * Revision 1.2  1999/10/09 11:20:55  rich
 * Const clean and no longer needs KWMModuleApp
 *
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 */
