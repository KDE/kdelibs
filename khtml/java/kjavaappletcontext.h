// -*- c++ -*-

#ifndef KJAVAAPPLETCONTEXT_H
#define KJAVAAPPLETCONTEXT_H

#include <qobject.h>
#include <kurl.h>

class KJavaAppletServer;
class KJavaApplet;

/**
 * @short Provides a context for KJavaAppletWidgets
 *
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
class KJavaAppletContext : public QObject
{
Q_OBJECT

public:
   /**
    * If server is zero then the default server is used.
    */
   KJavaAppletContext( KJavaAppletServer *server = 0 );

   /**
    * Returns a singleton context. This should only be used for simple
    * purposes, more complex applications require multiple contexts.
    */
   static KJavaAppletContext *getDefaultContext();

   int contextId();
   void setContextId( int id );

  // Forwarded from the applets
  void create( KJavaApplet * );
  void setParameter( KJavaApplet *applet, QString name, QString value );
  void show( KJavaApplet *, QString title );
  void start( KJavaApplet * );
  void stop( KJavaApplet * );

protected:
   //* The applet server this context is attached to.
   KJavaAppletServer *server;

   int id;
};

#endif // KJAVAAPPLETCONTEXT_H



