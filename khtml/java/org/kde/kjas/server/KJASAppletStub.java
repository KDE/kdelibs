package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;

/**
 * The stub used by Applets to communicate with their environment.
 *
 */

public class KJASAppletStub extends Frame
    implements AppletStub, Runnable
{
    KJASAppletContext context;    // The containing context.
    Hashtable         params;     // Maps parameter names to values
    URL               codeBase;   // The URL directory where files are
    URL               docBase;    // The document that referenced the applet
    boolean           active;     // Is the applet active?
    String            appletName; // The name of this applet instance
    String            appletID;   // The id of this applet- for use in callbacks
    Dimension         appletSize;
    String            windowName;
    String            className;
    Class             appletClass;
    boolean embedded;
    boolean classLoaded;

    KJASAppletClassLoader loader;
    KJASAppletPanel       panel;
    Applet                app;

    /**
     * Create an AppletStub for the specified applet. The stub will be in
     * the specified context and will automatically attach itself to the
     * passed applet.
     */
    public KJASAppletStub( KJASAppletContext _context, String _appletID,
                           URL _codeBase, URL _docBase,
                           String _appletName, String _className,
                           Dimension _appletSize, Hashtable _params,
                           String _windowName, KJASAppletClassLoader _loader )
    {
        super( _windowName );

        context    = _context;
        appletID   = _appletID;
        codeBase   = _codeBase;
        docBase    = _docBase;
        appletName = _appletName;
        className  = _className;
        appletSize = _appletSize;
        params     = new Hashtable( _params );

        windowName = _windowName;
        loader     = _loader;

        embedded    = false;
        classLoaded = false;

        panel = new KJASAppletPanel( _appletSize );
        add( "Center", panel );
        pack();
    }

    /*************************************************************************
     *********************** Runnable Interface ******************************
     *************************************************************************/
    public void run()
    {
        //load the classes...
        try
        {
            appletClass = loader.loadClass( className );

            if( embedded )
                createApplet();
            else
                classLoaded = true;
        }
        catch( ClassNotFoundException e )
        {
            Main.kjas_err( "Could not load class", e );
        }
    }

    public void initApplet()
    {
        embedded = true;
        if( classLoaded )
            createApplet();
    }

    private void createApplet()
    {
        try
        {
            app = (Applet) appletClass.newInstance();
            app.setStub( this );
            app.resize( appletSize );

            panel.add( "Center", app );
            app.init();
            panel.validate();

            //start the applet in a separate thread...
            final Applet fapp = app;
            Thread t = new Thread
            (
                new Runnable()
                {
                    public void run()
                    {
                        fapp.start();
                    }
                }
            );
        }
        catch( InstantiationException e )
        {
            Main.kjas_err( "Could not instantiate applet", e );
        }
    catch( IllegalAccessException e )
        {
            Main.kjas_err( "Could not instantiate applet", e );
        }
    }

    public void startApplet()
    {
        app.start();
    }

    public void stopApplet()
    {
        app.stop();
    }

    public void die()
    {
        app.stop();
        dispose();
    }

    public Applet getApplet()
    {
        return app;
    }



    /*************************************************************************
     ********************** AppletStub Interface *****************************
     *************************************************************************/
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
        return appletName;
    }

    public String getParameter( String name )
    {
        String rval;

        //try same case, then other case
        rval = (String) params.get( name );

        if( rval == null )
            rval = (String) params.get( name.toLowerCase() );

        return rval;
    }

    public boolean isActive()
    {
        return active;
    }

    /*************************************************************************
     ************************* Layout methods ********************************
     *************************************************************************/
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

    class KJASAppletPanel extends Panel
    {
        private Dimension size;

        public KJASAppletPanel( Dimension _size )
        {
            super( new BorderLayout() );
            size = _size;
        }

        public void setAppletSize( Dimension _size )
        {
            size = _size;
        }

        public Dimension getPreferrredSize()
        {
            return size;
        }
    }

}
