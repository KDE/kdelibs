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
    private Hashtable appletNames;
    private Hashtable appletIDs;

    private Hashtable stubs;

    private String myID;
    private KJASAppletClassLoader loader;


    /**
     * Create a KJASAppletContext. This is shared by all applets (though perhaps
     * there should be one for each web page).
     */
    public KJASAppletContext( String _contextID )
    {
        appletNames = new Hashtable();
        appletIDs = new Hashtable();
        stubs = new Hashtable();

        myID = _contextID;
    };

    public String getID()
    {
        return myID;
    }

    public void createApplet( String appletID,
                              String className, URL codeBase,
                              URL docBase, String jars,
                              String name, Dimension size,
                              Hashtable params )
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

            if( loader == null )
            {
                Main.kjas_debug( "Creating class loader with url = " + real_codeBase );
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

            if( appletClass != null )
            {
                // Load and instantiate applet
                Applet app = (Applet) appletClass.newInstance();
                app.setSize( size );

                KJASAppletStub stub = new KJASAppletStub( this, appletID, app, codeBase, docBase, name, params );

                appletNames.put( name, app );
                appletIDs.put( appletID, app );
                stubs.put( appletID, stub );
            }
        }
        catch ( ClassNotFoundException e )
        {
            Main.kjas_err( "Could not find the needed class" + e, e );
        }
        catch ( Exception e )
        {
            Main.kjas_err( "Something bad happened: " + e, e );
        }
    }

    public void destroy()
    {
        Enumeration e = appletNames.elements();
        while ( e.hasMoreElements() )
        {
            Applet app = (Applet) e.nextElement();
            app.stop();
        }
        appletNames.clear();
        appletIDs.clear();
        stubs.clear();
    }

    public void destroyApplet( String appletID )
    {
        Applet app = (Applet) appletIDs.get( appletID );
        if( app == null )
            Main.kjas_debug( "could not destroy applet: " + appletID );
        else
        {
            Main.kjas_debug( "stopping applet: " + appletID );
            app.stop();

            appletIDs.remove( appletID );
            appletNames.remove( appletID );
            stubs.remove( appletID );
        }
    }

    public void showApplet( String appletID, String title )
    {
        Applet app = (Applet) appletIDs.get( appletID );
        if( app == null )
        {
            Main.kjas_debug( "could not show applet: " + appletID );
        }
        else
        {
            Frame f = new Frame( title );
            AppletPanel p = new AppletPanel( app.getSize() );

            p.add("Center", app);
            f.add("Center", p);
            f.pack();

            app.init();
            startApplet( appletID );

            f.setVisible( true );
        }
    }

    public void startApplet( String appletID )
    {
        final Applet app = (Applet) appletIDs.get( appletID );

        if( app == null )
        {
            Main.kjas_debug( "could not start applet: " + appletID );
        }
        else
        {
            Thread t = new Thread
            (
                new Runnable()
                {
                    public void run()
                    {
                        app.start();
                    }
                }
            );
            t.start();
        }
    }

    public void stopApplet( String appletID )
    {
        Applet app = (Applet) appletIDs.get( appletID );
        if( app == null )
        {
            Main.kjas_debug( "could not stop applet: " + appletID );
        }
        else
        {
            app.stop();
        }
    }

    //
    // AppletContext interface
    //
    public Applet getApplet( String appletID )
    {
        return (Applet) appletIDs.get( appletID );
    }

    public Enumeration getApplets()
    {
        Enumeration e = appletNames.elements();
        return e;
    }

    public AudioClip getAudioClip( URL url )
    {
        //needs to be implemented
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
            Main.protocol.sendShowDocumentCmd( myID, url.toString()  );
    }

    public void showDocument( URL url, String targetFrame )
    {
        if ( ( url != null ) && ( targetFrame != null ) )
            Main.protocol.sendShowDocumentCmd( myID, url.toString(), targetFrame );
    }

    public void showStatus( String message )
    {
        if( message != null )
            Main.protocol.sendShowStatusCmd( myID, message );
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
