// -*- c++ -*-

#ifndef KJAVAAPPLET_H
#define KJAVAAPPLET_H

#include <qobject.h>
#include <qdict.h>
#include <kurl.h>
#include <kjavaappletcontext.h>

/**
 * @short A Java applet
 *
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.2  1999/10/09 11:20:55  rich
 * Const clean and no longer needs KWMModuleApp
 *
 * Revision 1.1.1.1  1999/07/22 17:28:06  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaApplet : public QObject
{
Q_OBJECT

public:
   KJavaApplet( KJavaAppletContext *context = 0 );

   //
   // Stuff to do with the applet
   //

   /**
    * Specify the name of the class file to run. For example 'Lake.class'.
    */
   void setAppletClass( const QString clazzName );
   const QString appletClass();

   /**
    * Specify the location of the jar file containing the class.
    * (unimplemented)
    */
   void setJARFile( const QString jar );
   const QString jarFile();

   /**
    * Specify a parameter to be passed to the applet.
    */
   void setParameter( const QString name, const QString value );

   /**
    * Set the URL of the document embedding the applet.
    */
   void setBaseURL( const QString base );
   const QString baseURL();

    void setAppletName( const QString name );
    const QString appletName();

    void create();
    bool isCreated();

   void show( const QString title );

   /**
    * Run the applet.
    */
   void start();

   /**
    * Pause the applet.
    */
   void stop();

   int appletId();
   void setAppletId( int id );

private:
   // Applet info
    bool reallyExists;
   QString clazzName;
   QString appName;
   QString jar;
   QString base;
   QDict<char> params;
   KJavaAppletContext *context;
   int id;
};

#endif // KJAVAAPPLET_H

