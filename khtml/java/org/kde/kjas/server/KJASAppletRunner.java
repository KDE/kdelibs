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
 * Revision 1.4  2000/05/21 19:27:28  rogozin
 *
 * Fix reload exception
 *
 * Revision 1.3  2000/01/29 04:22:28  rogozin
 * Preliminary support for archive tag.
 * Fix size problem.
 *
 * Revision 1.2  2000/01/27 23:41:57  rogozin
 * All applet parameters are passed to KJAS now
 * Next step - make use of them.
 *
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
   Hashtable contexts;
   KJASAppletRunnerListener listener;

   public KJASAppletRunner()
   {
      contexts = new Hashtable();
   }

   public void setAppletRunnerListener( KJASAppletRunnerListener listener )
   {
      this.listener = listener;
   }

   //
   // Methods that are invoked in respose to requests by the
   // embedding application.
   //

   public void createContext( String contextId )
      throws IllegalArgumentException
   {
      if ( contexts.get( contextId ) != null )
         throw new IllegalArgumentException( "Invalid contextId passed to createContext() "
                                             + contextId );

      KJASAppletContext context = new KJASAppletContext();
      contexts.put( contextId, context );
   }

   public void destroyContext( String contextId )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( contexts == null )
         throw new IllegalArgumentException( "Invalid contextId passed to destroyContext() "
                                             + contextId );
      
      context.destroy();
      contexts.remove( contextId );
   }

   public void createApplet( String contextId,
                             String appletId,
                             String name,
                             String className,
                             String docBase,
                             String codeBase,
                             String jars,
                             Dimension size)
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null )
	 throw new IllegalArgumentException( "Invalid contextId passed to createApplet() "
                                             + contextId );
      URL docBaseURL = null;
      try {
	  docBaseURL = new URL( docBase );
      }
      catch ( MalformedURLException mue ) {
	  throw new IllegalArgumentException( mue.toString() );
      }

      URL codeBaseURL = null;
      if(codeBase != null) {
	  if(!codeBase.endsWith("/"))
	      codeBase = codeBase + "/";
	  try {
	      codeBaseURL = new URL( docBaseURL, codeBase );
	  }
	  catch(Exception e) {}
      }
      
      if(codeBaseURL == null) {
	  String urlString = docBaseURL.getFile();
	  int i = urlString.lastIndexOf('/');
	  if (i >= 0 &&
	      i < urlString.length() - 1) {
	      try {
		  codeBaseURL = new URL(docBaseURL, urlString.substring(0, i + 1));
	      } 
	      catch (Exception e) {}
	  }
      }
      
      // codeBaseURL can not be null. This should not happen, 
      // but if it does we fall back to document base
      if(codeBaseURL == null)
	  codeBaseURL = docBaseURL;
      
      context.createApplet( appletId, className, 
			    codeBaseURL, docBaseURL, 
			    jars, name, size );
   }

   public void destroyApplet( String contextId, String appletId )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      Applet app = context.getAppletStub( appletId ).getApplet();
      context.destroyApplet( app );
   }

   public void showApplet( String contextId, String appletId, String title )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      Applet app = context.getAppletStub( appletId ).getApplet();
      context.show( app, title );
   }

   public void startApplet( String contextId, String appletId )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null )
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

      final Applet app = context.getAppletStub( appletId ).getApplet();

      Thread t = new Thread( new Runnable() {
         public void run()
         {
            app.start();
         }
      });
      t.start();
   }

   public void stopApplet( String contextId, String appletId )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null )
         throw new IllegalArgumentException( "Invalid contextId passed to stopApplet() "
                                             + contextId );

      Applet app = context.getAppletStub( appletId ).getApplet();

      app.stop();
   }

   public void setParameter( String contextId, String appletId,
                             String name, String value )
      throws IllegalArgumentException
   {
      KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
      if ( context == null)
         throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                             + contextId );

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
