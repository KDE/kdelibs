package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;

/**
 * The stub used by Applets to communicate with their environment.
 *
 */

public class KJASAppletStub implements AppletStub
{
    //** The containing context.
    KJASAppletContext context;

    //** The applet this stub is attached to.
    Applet applet;

    //** Maps parameter names to values
    Hashtable params;

    //** The URL of the class file from which applet was loaded.
    URL codeBase;

    //** The document that referenced the applet
    URL docBase;

    //** Is the applet active?
    boolean active;

    //** The name of this applet instance
    String name;

    //** The id of this applet- for use in callbacks
    String appletID;

    /**
     * Create an AppletStub for the specified applet. The stub will be in
     * the specified context and will automatically attach itself to the
     * passed applet.
     */
    public KJASAppletStub( KJASAppletContext _context, String _appletID,
                           Applet _applet, URL _codeBase, URL _docBase, String _name,
                           Hashtable _params )
    {
        context  = _context;
        applet   = _applet;
        codeBase = _codeBase;
        docBase  = _docBase;
        name     = _name;
        appletID = _appletID;
        params   = new Hashtable( _params );

        applet.setStub( this );
    }

    /**
     * Called by the Applet when it wants to resize itself.
     */
    public void appletResize( int width, int height )
    {
        if ( (width > 0) && (height > 0))
        {
            Main.protocol.sendResizeAppletCmd( context.getID(), appletID, width, height );
        }
        else
            System.err.println( "Warning: applet attempted to resize itself to " + width + "," + height );
    }

    /**
     * Returns the context that created this AppletStub.
     */
    public AppletContext getAppletContext()
    {
        return context;
    }

    public URL getCodeBase()
    {
        return codeBase;
    }

    public URL getDocumentBase()
    {
        return docBase;
    }

    /**
     * Get the value of the named parameter.
     */
    public String getParameter( String name )
    {
        String rval;

        //try same case
        rval = (String) params.get( name );

        if( rval == null )
            rval = (String) params.get( name.toLowerCase() );

        return rval;
    }

    public boolean isActive()
    {
        return active;
    }
}
