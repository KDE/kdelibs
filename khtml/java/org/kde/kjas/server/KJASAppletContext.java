package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;

/**
 * The context in which applets live.
 *
 */

public class KJASAppletContext implements AppletContext
{
    //* All the applets in this context
    private Hashtable applets;
    private Hashtable stubList;

    private String contextID;

    private KJASAppletClassLoader loader;


    /**
     * Create a KJASAppletContext. This is shared by all applets (though perhaps
     * there should be one for each web page).
     */
    public KJASAppletContext( String _contextID )
    {
        applets = new Hashtable();
        stubList = new Hashtable();

        contextID = _contextID;
    };

    public String getID()
    {
        return contextID;
    }

    public KJASAppletStub getAppletStub( String appletId )
    {
        KJASAppletStub stb = (KJASAppletStub) stubList.get( appletId );
        if ( stb == null )
            throw new IllegalArgumentException( "Invalid appletId passed to getAppletStub() "
                                                + appletId );
        return stb;
    }

    public KJASAppletStub createApplet( String appletID,
                                        String className, URL codeBase,
                                        URL docBase, String jars,
                                        String name, Dimension size )
    {
        try
        {
            URL real_codeBase = codeBase;

            if( real_codeBase == null )
                real_codeBase = docBase;

            if( real_codeBase == null )
            {
                //we have a serious problem- where do we get classes from ???
                Main.kjas_debug( "Can't create URLClassLoader with a valid url" );
                throw new IllegalArgumentException( "no codeBase for applet" );
            }
            else
            {
                Main.kjas_debug( "checking the url for validity" );
                Main.kjas_debug( "real_codeBase.getFile() == " + real_codeBase.getFile() );

                //if this is a url like http://www.foo.com, it needs a slash on
                //the end otherwise URLClassLoader thinks it's a jar file ??
                if( ( real_codeBase.getFile().trim().length() == 0 ||
                      real_codeBase.getFile() == null ) &&
                    !real_codeBase.toString().endsWith(".jar") )
                {
                    Main.kjas_debug( "real_codeBase has no file, adding a /" );
                    real_codeBase = new URL( real_codeBase.toString().concat( "/" ) );
                }
            }

            Main.kjas_debug( "Creating class loader with url = " + real_codeBase );
            if( loader == null )
            {
                loader = KJASAppletClassLoader.createLoader( real_codeBase );
            }
            else
            {
                loader.addCodeBase( real_codeBase );
            }

            if( jars != null )
            {
                StringTokenizer parser = new StringTokenizer( jars, ",", false );
                while( parser.hasMoreTokens() )
                {
                    String jar = parser.nextToken().trim();
                    loader.addJar( codeBase, jar );
                }
            }

            Class appletClass = loader.loadClass( className );

            // Load and instantiate applet
            Applet app = (Applet) appletClass.newInstance();
            app.setSize( size );

            KJASAppletStub stub = new KJASAppletStub( this, appletID, app, codeBase, docBase, name );

            applets.put( name, app );
            stubList.put( appletID, stub );

            return stub;
        }
        catch ( ClassNotFoundException e )
        {
            Main.kjas_err( "Could not find the needed class" + e, e );
        }
        catch ( Exception e )
        {
            Main.kjas_err( "Something bad happened: " + e, e );
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
        if ( applets.contains( app ) )
        {
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
    public Applet getApplet( String name )
    {
        return (Applet) applets.get( name );
    }

    public Enumeration getApplets()
    {
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
        if( url != null )
            Main.protocol.sendShowDocumentCmd( contextID, url.toString()  );
    }

    public void showDocument( URL url, String targetFrame )
    {
        if ( ( url != null ) && ( targetFrame != null ) )
            Main.protocol.sendShowDocumentCmd( contextID, url.toString(), targetFrame );
    }

    public void showStatus( String message )
    {
        if( message != null )
            Main.protocol.sendShowStatusCmd( contextID, message );
    }

    class AppletPanel
        extends Panel
    {
        Dimension appletSize;

        AppletPanel( Dimension size )
        {
            super(new BorderLayout());
            appletSize = size;
        }

        public Dimension getPreferredSize()
        {
            return appletSize;
        }
    }
}
