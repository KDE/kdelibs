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
    private Hashtable appletThreads;
    private Hashtable stubs;

    private String myID;
    private KJASAppletClassLoader loader;

    /**
     * Create a KJASAppletContext. This is shared by all applets (though perhaps
     * there should be one for each web page).
     */
    public KJASAppletContext( String _contextID )
    {
        appletNames   = new Hashtable();
        appletIDs     = new Hashtable();
        appletThreads = new Hashtable();

        stubs = new Hashtable();

        myID = _contextID;
    };

    public String getID()
    {
        return myID;
    }

    public void createApplet( String appletID, String name,
                              String className, String docBase,
                              String codeBase, String jars,
                              Dimension size, String windowName,
                              Hashtable params )
    {
        String realCodeBaseProtocol = null;
        URL docBaseURL = null;
        URL codeBaseURL = null;
        try
        {
            //first check to see if the protocol is supported by java,
            //if it's not http or ftp, we've got to use the kde libraries
            //to handle it.  I don't feel like implementing everything necessary
            //to handle a new URL in java
            String docBaseProtocol = null;
            String codeBaseProtocol = null;
            boolean useNative = false;

            int i = docBase.indexOf( ':' );
            if( i != -1 )
                docBaseProtocol = docBase.substring( 0, i );
            if( codeBase != null )
            {
                int j = codeBase.indexOf( ':' );
                if( j != -1 )
                    codeBaseProtocol = codeBase.substring( 0, j );
            }

            if( codeBaseProtocol != null )
                if( !codeBaseProtocol.equals("http") && !codeBaseProtocol.equals( "ftp" ) )
                {
                    useNative = true;
                    realCodeBaseProtocol = codeBaseProtocol;
                }
            else
            if( !docBaseProtocol.equals( "http" ) && !docBaseProtocol.equals( "ftp" ) )
            {
                useNative = true;
                realCodeBaseProtocol = docBaseProtocol;
            }
            Main.kjas_debug( "protocol for class loading = " + realCodeBaseProtocol );
            if( !useNative )
            {

                //first determine what the real codeBase is: 3 cases
                //#1. codeBase is absolute URL- use that
                //#2. codeBase is relative to docBase, create url from those
                //#3. last resort, use docBase as the codeBase

                URL fixed_docBaseURL = null;
                try
                {
                    docBaseURL = new URL( docBase );
                    String urlString = docBaseURL.toString();
                    int j = urlString.lastIndexOf('/');
                    if (j >= 0 && j < urlString.length() - 1)
                    {
                        fixed_docBaseURL = new URL( urlString.substring(0, j+1) );
                    }

                    Main.kjas_debug( "docBaseURL = " + docBaseURL );
                    Main.kjas_debug( "fixed_docBaseURL = " + fixed_docBaseURL );
                }
                catch ( MalformedURLException mue )
                {
                    Main.kjas_debug( "Could not create URL from docBase, not creating applet" );
                    return;
                }

                if(codeBase != null)
                {
                    Main.kjas_debug( "codeBase not null, trying to create URL from it" );
                    if( !codeBase.endsWith("/") )
                        codeBase = codeBase + "/";

                    try
                    {
                        codeBaseURL = new URL( codeBase );
                    } catch( MalformedURLException mue )
                    {
                        try
                        {
                            Main.kjas_debug( "could not create URL from codeBase alone" );
                            codeBaseURL = new URL( fixed_docBaseURL, codeBase );
                        } catch( MalformedURLException mue2 )
                        {
                            Main.kjas_debug( "could not create URL from docBaseURL and codeBase" );
                        }
                    }
                }

                if(codeBaseURL == null)
                {
                    Main.kjas_debug( "codeBaseURL still null, defaulting to fixed docBase" );
                    codeBaseURL = fixed_docBaseURL;
                }

                // codeBaseURL can not be null. This should not happen,
                // but if it does we fall back to document base
                if(codeBaseURL == null)
                {
                    Main.kjas_debug( "we're falling back to document base = " + docBaseURL );
                    codeBaseURL = docBaseURL;
                }

                //then we need to get the classloader...
                if( loader == null )
                {
                    loader = KJASAppletClassLoader.createLoader( codeBaseURL );
                }
                else
                {
                    //make sure that this codeBase is in the classloader
                    loader.addCodeBase( codeBaseURL );
                }

                if( jars != null )
                {
                    StringTokenizer parser = new StringTokenizer( jars, ",", false );
                    while( parser.hasMoreTokens() )
                    {
                        String jar = parser.nextToken().trim();
                        loader.addJar( codeBaseURL, jar );
                    }
                }
            }
            else
            {
                Main.kjas_debug( "error: unsupported protocol: " + realCodeBaseProtocol );
            }

            Class appletClass = loader.loadClass( className );

            if( appletClass != null )
            {
                // Load and instantiate applet
                Applet app = (Applet) appletClass.newInstance();
                KJASAppletStub stub = new KJASAppletStub( this, appletID, app, codeBaseURL, docBaseURL, name, params );

                app.setSize( size );

                appletNames.put( name, app );
                appletIDs.put( appletID, app );
                stubs.put( appletID, stub );

                Frame f = new Frame( windowName );
                AppletPanel p = new AppletPanel( app.getSize() );

                p.add("Center", app);
                f.add("Center", p);
                f.pack();

                app.init();
                Thread t = runApplet( app );
                if( t != null )
                    appletThreads.put( appletID, t );

                f.setVisible( true );
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

        e = appletThreads.elements();
        while( e.hasMoreElements() )
        {
            Thread t = (Thread) e.nextElement();
            t.destroy();
        }

        appletNames.clear();
        appletIDs.clear();
        appletThreads.clear();
        stubs.clear();
    }

    public void destroyApplet( String appletID )
    {
        Applet app = (Applet) appletIDs.get( appletID );
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );

        if( app == null || stub == null )
            Main.kjas_debug( "could not destroy applet: " + appletID );
        else
        {
            Main.kjas_debug( "stopping applet: " + appletID );
            app.stop();
            Thread t = (Thread) appletThreads.get( appletID );
            t.destroy();

            appletThreads.remove( appletID );
            appletIDs.remove( appletID );
            appletNames.remove( stub.getAppletName() );
            appletThreads.remove( appletID );
            stubs.remove( appletID );
        }
    }

    public void startApplet( String appletID )
    {
        Applet app = (Applet) appletIDs.get( appletID );
        Thread t = runApplet( app );
        if( t != null )
            appletThreads.put( appletID, t );
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

    private Thread runApplet( final Applet app )
    {
        if( app != null )
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

            return t;
        }

        return null;
    }



    /***************************************************************************
    **** AppletContext interface
    ***************************************************************************/
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
