package org.kde.kjas.server;

import java.util.*;
import java.net.*;
import java.applet.*;
import java.awt.*; 
/**
 * Manages the running contexts and applets.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.1.1.1  1999/07/22 17:28:08  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
public class KJASAppletRunner
{
   Vector contexts;
   KJASAppletRunnerListener listener;

   public KJASAppletRunner()
   {
      contexts = new Vector();
   }

   public void setAppletRunnerListener( KJASAppletRunnerListener listener )
   {
      this.listener = listener;
   }

   //
   // Methods that are invoked in respose to requests by the
   // embedding application.
   //

   public void createContext( int contextId )
      throws IllegalArgumentException
   {
      if ( contextId < contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to createContext() "
                                             + contextId );

      KJASAppletContext context = new KJASAppletContext();
      contexts.addElement( context );
   }

   public void destroyContext( int contextId )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to destroyContext() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      context.destroy();
      contexts.setElementAt( null, contextId );
   }

   public void createApplet( int contextId,
                             int appletId,
                             String name,
                             String className,
                             String base,
                             String codeBase,
                             String archive,
                             Dimension size)
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to createApplet() "
                                             + contextId );

      try {
         KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );

         URL baseURL = new URL( base );
         URL classURL = new URL( baseURL, className );

         context.createApplet( classURL, baseURL, name );
      }
      catch ( MalformedURLException mue )
         {
            throw new IllegalArgumentException( mue.toString() );
         }
   }

   public void destroyApplet( int contextId, int appletId )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      Applet app = context.getAppletStub( appletId ).getApplet();
      context.destroyApplet( app );
   }

   public void showApplet( int contextId, int appletId, String title )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      Applet app = context.getAppletStub( appletId ).getApplet();
      context.show( app, title );
   }

   public void startApplet( int contextId, int appletId )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      final Applet app = context.getAppletStub( appletId ).getApplet();

      Thread t = new Thread( new Runnable() {
         public void run()
         {
            app.start();
         }
      });
      t.start();
   }

   public void stopApplet( int contextId, int appletId )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to stopApplet() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      Applet app = context.getAppletStub( appletId ).getApplet();

      app.stop();
   }

   public void setParameter( int contextId, int appletId,
                             String name, String value )
      throws IllegalArgumentException
   {
      if ( contextId >= contexts.size() )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      KJASAppletContext context = (KJASAppletContext) contexts.elementAt( contextId );
      KJASAppletStub stub = context.getAppletStub( appletId );

      stub.setParameter( name, value );
   }

   //
   // Main
   //
  /*
   public static void main( String[] args )
   {
      KJASAppletRunner runner = new KJASAppletRunner();
      runner.createContext( 0 );
      runner.createApplet( 0, 0, "fred",
                           "Lake.class",
                           "http://127.0.0.1/applets/" );
      runner.setParameter( 0, 0, "image", "logo.gif" );
      runner.showApplet( 0, 0, "unique_title_one" );

      runner.createContext( 1 );
      runner.createApplet( 1, 0, "barney",
                           "Lake.class",
                           "http://127.0.0.1/applets/" );
      runner.setParameter( 1, 0, "image", "wolf2.jpg" );
      runner.showApplet( 1, 0, "unique_title_two" );

      //      runner.destroyContext( 0 );
   }
  */
}
