package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;

/**
 * The context in which applets live.
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
     * Create a KJASAppletContext
     */
    public KJASAppletContext( String _contextID )
    {
        appletNames   = new Hashtable();
        appletIDs     = new Hashtable();
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
            KJASAppletClassLoader loader =
                KJASAppletClassLoader.getLoader( docBase, codeBase );
            if( archives != null )
            {
                StringTokenizer parser = new StringTokenizer( archives, ",", false );
                while( parser.hasMoreTokens() )
                {
                    String jar = parser.nextToken().trim();
                    loader.addJar( jar );
                }
            }

            KJASAppletStub stub =
                new KJASAppletStub( this, appletID, loader.getCodeBase(),
                                    loader.getDocBase(), name, className,
                                    size, params, windowName, loader );
            stub.show();
            new Thread( stub ).start();
            stubs.put( appletID, stub );
        }
        catch ( Exception e )
        {
            Main.kjas_err( "Something bad happened in createApplet: " + e, e );
        }
    }

    public void initApplet( String appletID )
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if( stub == null )
        {
            Main.kjas_debug( "could not init and show applet: " + appletID );
        }
        else
        {
            stub.initApplet();
        }
    }

    public void destroyApplet( String appletID )
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );

        if( stub == null )
        {
            Main.kjas_debug( "could not destroy applet: " + appletID );
        }
        else
        {
            Main.kjas_debug( "stopping applet: " + appletID );
            stub.die();
            stubs.remove( appletID );
            System.gc();
        }
    }

    public void startApplet( String appletID )
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if( stub == null )
        {
            Main.kjas_debug( "could not start applet: " + appletID );
        }
        else
        {
            stub.startApplet();
        }
    }

    public void stopApplet( String appletID )
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if( stub == null )
        {
            Main.kjas_debug( "could not stop applet: " + appletID );
        }
        else
        {
            stub.stopApplet();
        }
    }

    public void destroy()
    {
        Enumeration e = stubs.elements();
        while ( e.hasMoreElements() )
        {
            KJASAppletStub stub = (KJASAppletStub) e.nextElement();
            stub.die();
        }
        stubs.clear();

        System.gc();
    }

    /***************************************************************************
    **** AppletContext interface
    ***************************************************************************/
    public Applet getApplet( String appletName )
    {
        Enumeration e = stubs.elements();
        while( e.hasMoreElements() )
        {
            KJASAppletStub stub = (KJASAppletStub) e.nextElement();

            if( stub.getAppletName().equals( appletName ) )
                return stub.getApplet();
        }

        return null;
    }

    public Enumeration getApplets()
    {
        Vector v = new Vector();
        Enumeration e = stubs.elements();
        while( e.hasMoreElements() )
        {
            KJASAppletStub stub = (KJASAppletStub) e.nextElement();
            v.add( stub );
        }

        return v.elements();
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
