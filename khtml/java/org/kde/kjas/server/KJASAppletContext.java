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
    private Hashtable runThreads;
    private Hashtable appletClasses;

    private String myID;
    private KJASAppletClassLoader loader;

    /**
     * Create a KJASAppletContext
     */
    public KJASAppletContext( String _contextID )
    {
        appletNames   = new Hashtable();
        appletIDs     = new Hashtable();
        appletClasses = new Hashtable();
        runThreads    = new Hashtable();
        stubs         = new Hashtable();
        myID          = _contextID;
    }

    public String getID()
    {
        return myID;
    }

    public void createApplet( String appletID, String name,
                              String className, String docBase,
                              String codeBase, String archives,
                              Dimension size, String windowName,
                              Hashtable params )
    {
        try
        {
            KJASAppletClassLoader loader = new KJASAppletClassLoader( docBase,
                                                                      codeBase );
            if( archives != null )
            {
                StringTokenizer parser = new StringTokenizer( archives, ",", false );
                while( parser.hasMoreTokens() )
                {
                    String jar = parser.nextToken().trim();
                    loader.addJar( jar );
                }
            }
            Class appletClass = loader.loadClass( className );
            appletClasses.put( appletID, appletClass );

            KJASAppletStub stub = new KJASAppletStub( this, appletID,
                                                      loader.getCodeBase(),
                                                      loader.getDocBase(),
                                                      name, params, size );
            stub.setSize( size );
            stubs.put( appletID, stub );

            Frame f = new Frame( windowName );
            f.add( "Center", stub );
            f.pack();
            f.setVisible( true );
        }
        catch( ClassNotFoundException e )
        {
            Main.kjas_err( "Could not find the needed class " + e, e );
        }
        catch ( Exception e )
        {
            Main.kjas_err( "Something bad happened in createApplet: " + e, e );
        }
    }

    public void initApplet( String appletID )
    {
        KJASAppletStub stub     = (KJASAppletStub) stubs.get( appletID );
        Class          theClass = (Class) appletClasses.get( appletID );
        if( stub == null || theClass == null )
        {
            Main.kjas_debug( "could not init and show applet: " + appletID );
        }
        else
        {
            try
            {
                Main.kjas_debug( "theClass = " + theClass );

                Applet app = (Applet) theClass.newInstance();
                app.setStub( stub );
                appletNames.put( stub.getAppletName(), app );
                appletIDs.put( appletID, app );
                app.resize( stub.getPreferredSize() );
                stub.add( "Center", app );

                app.init();
                Main.kjas_debug( "applet done with init, applet = " + app );

                stub.validate();
                Main.kjas_debug( "added applet to panel, applet = " + app );

                runApplet( appletID, app );
                Main.kjas_debug( "applet is running, = " + app );
            }
            catch( InstantiationException e )
            {
                Main.kjas_err( "Could not instantiate applet: " + e, e );
            }
            catch( IllegalAccessException e )
            {
                Main.kjas_err( "Could not Access applet class: " + e, e );
            }
        }
    }

    private Thread runApplet( final String appletID, final Applet app )
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
            if( t != null )
            {
                t.start();
                runThreads.put( appletID, t );
            }
            return t;
        }

        return null;
    }

    public void destroy()
    {
        Enumeration e = appletNames.elements();
        while ( e.hasMoreElements() )
        {
            Applet app = (Applet) e.nextElement();
            app.stop();
        }

        e = runThreads.elements();
        while( e.hasMoreElements() )
        {
            Thread t = (Thread) e.nextElement();
            t.destroy();
        }

        appletNames.clear();
        appletIDs.clear();
        appletClasses.clear();
        runThreads.clear();
        stubs.clear();

        System.gc();
    }

    public void destroyApplet( String appletID )
    {
        Applet         app  = (Applet) appletIDs.get( appletID );
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );

        if( app == null || stub == null )
        {
            Main.kjas_debug( "could not destroy applet: " + appletID );
        }
        else
        {
            Main.kjas_debug( "stopping applet: " + appletID );
            app.stop();
            Thread t = (Thread) runThreads.get( appletID );
            t.destroy();

            runThreads.remove( appletID );
            appletIDs.remove( appletID );
            appletNames.remove( stub.getAppletName() );
            appletClasses.remove( appletID );
            stubs.remove( appletID );

            System.gc();
        }
    }

    public void startApplet( String appletID )
    {
        Applet app = (Applet) appletIDs.get( appletID );

        if( app == null )
        {
            Main.kjas_debug( "could not start applet: " + appletID );
        }
        else
        {
            app.start();
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
        //needs to be implemented- tie in to the artsd somehow?
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

}
