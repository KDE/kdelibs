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
   void setAppletClass( QString clazzName );
   QString appletClass();

   /**
    * Specify the location of the jar file containing the class.
    * (unimplemented)
    */
   void setJARFile( QString jar );
   QString jarFile();

   /**
    * Specify a parameter to be passed to the applet.
    */
   void setParameter( QString name, QString value );

   /**
    * Set the URL of the document embedding the applet.
    */
   void setBaseURL( QString base );
   QString baseURL();

    void setAppletName( QString name );
    QString appletName();

   void create();
    bool isCreated();

   void show( QString title );

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

