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
   KJavaAppletContext();
   ~KJavaAppletContext();

   /**
    * Returns a singleton context. This should only be used for simple
    * purposes, more complex applications require multiple contexts.
    */
   static KJavaAppletContext *getDefaultContext();

   int contextId();
   void setContextId( int id );

  // Forwarded from the applets
  void create( KJavaApplet * );
  void destroy( KJavaApplet * );
  void setParameter( KJavaApplet *applet, const QString &name, const QString &value );
  void show( KJavaApplet *, const QString &title );
  void start( KJavaApplet * );
  void stop( KJavaApplet * );

signals:
  void showStatus( const QString &txt );
  void showDocument( const QString &url, const QString &target );

protected:
   //* The applet server this context is attached to.
   KJavaAppletServer *server;

protected slots:
   void received( const QString &cmd, const QStringList &arg );

private:
   int id;
   struct KJavaAppletContextPrivate *d;
};

#endif // KJAVAAPPLETCONTEXT_H

/**
 *
 * $Log$
 * Revision 1.4  2000/06/06 22:53:36  gehrmab
 * Beauty and wellness for the API documentation
 *
 * Revision 1.3  2000/03/21 03:44:43  rogozin
 *
 * New Java support has been merged.
 *
 * Revision 1.2  1999/12/14 19:56:59  rich
 * Many fixes, see changelog
 *
 * Revision 1.1.1.1  1999/07/22 17:28:07  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 */
