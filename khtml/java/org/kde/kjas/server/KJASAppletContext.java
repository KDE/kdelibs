package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;

/**
 * The context in which applets live.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.8  2000/05/21 19:27:28  rogozin
 *
 * Fix reload exception
 *
 * Revision 1.7  2000/03/23 03:20:38  rogozin
 *
 * Fix resize issues for jdk 1.1.8.
 * This is was quite a hack.
 *
 * Revision 1.6  2000/03/22 05:19:38  rogozin
 *
 * Window geometry is now handled correctly.
 *
 * Revision 1.5  2000/01/29 04:22:28  rogozin
 * Preliminary support for archive tag.
 * Fix size problem.
 *
 * Revision 1.4  2000/01/27 23:41:57  rogozin
 * All applet parameters are passed to KJAS now
 * Next step - make use of them.
 *
 * Revision 1.3  1999/11/12 02:58:05  rich
 * Updated KJAS server
 *
 * Revision 1.2  1999/10/09 21:55:27  rich
 * More consts
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
public class KJASAppletContext implements AppletContext
{
   //* All the applets in this context
   Hashtable applets;

   Hashtable stubList;

   /**
    * Create a KJASAppletContext. This is shared by all applets (though perhaps
    * there should be one for each web page).
    */
    public KJASAppletContext() {
       applets = new Hashtable();
       stubList = new Hashtable();
    };

   KJASAppletStub getAppletStub( String appletId )
   {
      KJASAppletStub stb = (KJASAppletStub) stubList.get( appletId );
      if ( stb == null )
         throw new IllegalArgumentException( "Invalid appletId passed to getAppletStub() "
                                             + appletId );

      return stb;
   }

   public KJASAppletStub createApplet( String appletId,
				       String className, URL codeBase,
                                       URL docBase, String jars, 
                                       String name, Dimension size ) {
      Applet app;

      try {
         KJASAppletClassLoader loader = new KJASAppletClassLoader( codeBase );
         if(jars != null)
             loader.loadJars(jars);
         Class appletClass = loader.loadClass( className );

         // Load and instantiate applet
	 app = (Applet) appletClass.newInstance();
         app.setSize(size);

         KJASAppletStub stub = new KJASAppletStub( this, app, codeBase, docBase, name );

         applets.put( name, app );
         stubList.put( appletId, stub );

         return stub;
      }
      catch ( Exception e ) {
         System.err.println( "Applet instantiation error: " + e );
      }
      return null;
   }

   public void destroy()
   {
      Enumeration e = applets.elements();
      while ( e.hasMoreElements() ) {
         Applet app = (Applet) e.nextElement();
         app.stop();
      }
      applets.clear();
      stubList.clear();
   }

   public void destroyApplet( Applet app )
   {
      app.stop();
   }

   public void show( Applet app, String title )
   {
      if ( applets.contains( app ) ) {
         Frame f = new Frame( title );
         AppletPanel p = new AppletPanel( app.getSize() );
         
         p.add("Center", app);
         f.add("Center", p);
         f.pack();
         
         app.init();
         app.start();

         f.setVisible( true );
      }
   }

   //
   // AppletContext interface
   //

   public Applet getApplet( String name ) {
      return (Applet) applets.get( name );
   }

   public Enumeration getApplets() {
      Enumeration e = applets.elements();
      return e;
   }

   public AudioClip getAudioClip( URL url )
   {
      return null;
   }

   public Image getImage( URL url )
   {
      Toolkit kit = Toolkit.getDefaultToolkit();
      return kit.getImage( url );
   }

   public void showDocument( URL url )
   {
      System.out.println( "NOT IMPLEMENTED: showdocument!" + url );
   }

   public void showDocument( URL url, String targetFrame )
   {
       if ( ( url != null ) && ( targetFrame != null ) )
	   System.out.println( "NOT IMPLEMENTED: showurlinframe!" + url + "!" + targetFrame );
       else
	   System.err.println( "Warning applet attempted to show " + url + " in frame " + targetFrame );
   }

   public void showStatus( String message )
   {
      System.out.println( "NOT IMPLEMENTED: showstatus!" + message );
   }

   class AppletPanel 
      extends Panel 
   {
      Dimension appletSize;

      AppletPanel(Dimension size) 
      {
         super(new BorderLayout());
         appletSize = size;
      }
      
      public Dimension preferredSize()
      {
         return appletSize;
      }
   }
}
