package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;

/**
 * The stub used by Applets to communicate with their environment.
 *
 */

public class KJASAppletStub extends Panel implements AppletStub
{
    KJASAppletContext context;  // The containing context.
    Hashtable         params;   // Maps parameter names to values
    URL               codeBase; // The URL directory where files are
    URL               docBase;  // The document that referenced the applet
    boolean           active;   // Is the applet active?
    String            name;     // The name of this applet instance
    String            appletID; // The id of this applet- for use in callbacks
    Dimension         appletSize;

    /**
     * Create an AppletStub for the specified applet. The stub will be in
     * the specified context and will automatically attach itself to the
     * passed applet.
     */
    public KJASAppletStub( KJASAppletContext _context, String _appletID,
                           URL _codeBase, URL _docBase,
                           String _name, Hashtable _params, Dimension size )
    {
        super( new BorderLayout() );

        context    = _context;
        codeBase   = _codeBase;
        docBase    = _docBase;
        name       = _name;
        appletID   = _appletID;
        params     = new Hashtable( _params );
        appletSize = size;
    }

    /**
     * Called by the Applet when it wants to resize itself.
     */
    public void appletResize( int width, int height )
    {
        if ( (width >= 0) && (height >= 0))
        {
            Main.protocol.sendResizeAppletCmd( context.getID(), appletID, width, height );
            appletSize = new Dimension( width, height );
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

    public String getAppletName()
    {
        return name;
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

    public Dimension getPreferredSize()
    {
        return appletSize;
    }

    public Dimension getMinimumSize()
    {
        return appletSize;
    }

    public Dimension getMaximumSize()
    {
        return appletSize;
    }

}
