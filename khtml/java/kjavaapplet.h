// -*- c++ -*-

#ifndef KJAVAAPPLET_H
#define KJAVAAPPLET_H

#include <qobject.h>
#include <qmap.h>
#include <kurl.h>

class KJavaAppletContext;

/**
 * @short A Java applet
 *
 * @version $Id$
 * @author Richard J. Moore, rich@kde.org
 */
class KJavaApplet : public QObject
{
Q_OBJECT

public:
  KJavaApplet( KJavaAppletContext *context = 0 );
  virtual ~KJavaApplet();

   //
   // Stuff to do with the applet
   //

   /**
    * Specify the name of the class file to run. For example 'Lake.class'.
    */
   void setAppletClass( const QString &clazzName );
   QString &appletClass();

   /**
    * Specify the location of the jar file containing the class.
    * (unimplemented)
    */
   void setJARFile( const QString &jar );
   QString &jarFile();

   /**
    * Specify a parameter to be passed to the applet.
    */
   void setParameter( const QString &name, const QString &value );
   QString &parameter( const QString &name );

   /**
    * Set the URL of the document embedding the applet.
    */
   void setBaseURL( const QString &base );
   QString &baseURL();

   /**
    * Set the codebase of the applet classes.
    */
   void setCodeBase( const QString &codeBase );
   QString &codeBase();

   void setAppletName( const QString &name );
   QString &appletName();
 
   /**
    * Set the URL of the document embedding the applet.
    */
   void setSize( QSize size );
   QSize size();
  
   void create();
   bool isCreated();
 
   void show( const QString &title );

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
   struct KJavaAppletPrivate *d;
   QMap<QString, QString> params;
   KJavaAppletContext *context;
   int id;
};

#endif // KJAVAAPPLET_H

/**
 * $Log$
 * Revision 1.5  2000/01/27 23:41:56  rogozin
 * All applet parameters are passed to KJAS now
 * Next step - make use of them.
 *
 * Revision 1.4  1999/12/14 19:56:59  rich
 * Many fixes, see changelog
 *
 * Revision 1.3  1999/10/09 18:09:52  rich
 * Const QString fixes
 *
 * Revision 1.2  1999/10/09 11:20:55  rich
 * Const clean and no longer needs KWMModuleApp
 *
 * Revision 1.1.1.1  1999/07/22 17:28:06  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 */
