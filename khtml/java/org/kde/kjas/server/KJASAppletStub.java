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
    implements AppletStub
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

    KJASAppletClassLoader loader;
    KJASAppletPanel       panel;
    Applet                app;
    Thread                runThread;
    KJASAppletStub        me;

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
        className  = _className.replace( '/', '.' );
        appletSize = _appletSize;
        params     = _params;
        windowName = _windowName;
        loader     = _loader;

        // fix the applet name
        int idx = appletName.indexOf(".class");
        if (idx > 0) {
            appletName = appletName.substring(0, idx);
        }
            
        appletClass = null;
        me = this;
    }

    /*************************************************************************
     *********************** Runnable Interface ******************************
     *************************************************************************/
    public void createApplet() {
        panel = new KJASAppletPanel( appletSize );
        add( "Center", panel );
        pack();
        runThread = new Thread
        (
        new Runnable() {
            public void run() {
                //this order is very important and took a long time
                //to figure out- don't modify it unless there are
                //real bug fixes
                
                active = true;
                synchronized( loader ) {
                    try {
                        appletClass = loader.loadClass( className );
                    } catch (Exception e) {
                        Main.kjas_err("Class could not be loaded " + className, e);
                    }
                }
                if( appletClass == null ) {
                    panel.add( "Center", new Label( "Applet Failed", Label.CENTER ) );
                    return;
                }
                
                try {
                    app = (Applet) appletClass.newInstance();
                    app.setStub( me );
                }
                catch( InstantiationException e ) {
                    Main.kjas_err( "Could not instantiate applet", e );
                    panel.add( "Center", new Label( "Applet Failed", Label.CENTER ) );
                    return;
                }
                catch( IllegalAccessException e ) {
                    Main.kjas_err( "Could not instantiate applet", e );
                    panel.add( "Center", new Label( "Applet Failed", Label.CENTER ) );
                    return;
                }
        
                //app.setVisible( false );
                // with the preceding line, IllegalArgumentExceptions occur
                // with certain applets (eg. Animator applet from 1.4 demos)
                // so, don't do this!
                remove(panel);
                add( "Center", app );
                invalidate();
                setVisible(true);
                context.showStatus("Initializing Applet: " + appletName + " ...");
                // Main.info("Initializing Applet................");
                app.init();
                setVisible(true);
                context.showStatus("Starting Applet: " + appletName + " ...");
                // Main.info("Starting Applet................");
                app.start();  //We're already in a thread, so don't create a new one
            }
        }
        , "KJAS-Applet-" + appletID + "(" + appletName + ")");
        // Main.debug("starting runThread................");
        runThread.setContextClassLoader(loader);
        runThread.start();
    }

    public void startApplet()
    {
        if( app != null )
            app.start();
    }

    public void stopApplet()
    {
        if( app != null )
            app.stop();
    }

    public void initApplet()
    {
        if( app != null )
            app.init();
    }

    public void die()
    {
        if( app != null )
            app.stop();

        if( runThread != null && runThread.isAlive() )
            Main.debug( "runThread is active when stub is dying" );
        
        if (loader != null) {
            loader.setInactive();
        }
        active = false;
        dispose();
    }

    public Applet getApplet()
    {
        return app;
    }

    public Dimension getAppletSize()
    {
        return appletSize;
    }


    /*************************************************************************
     ********************** AppletStub Interface *****************************
     *************************************************************************/
    public void appletResize( int width, int height )
    {
        if( active )
        {
            if ( (width >= 0) && (height >= 0))
            {
                Main.debug( "Applet #" + appletID + ": appletResize to : (" + width + ", " + height + ")" );
                Main.protocol.sendResizeAppletCmd( context.getID(), appletID, width, height );
                appletSize = new Dimension( width, height );
                panel.setAppletSize( appletSize );
                pack();
            }
        }
    }

    public AppletContext getAppletContext()
    {
        if( active )
            return context;

        return null;
    }

    public URL getCodeBase()
    {
        if( active )
            return codeBase;

        return null;
    }

    public URL getDocumentBase()
    {
        if( active )
            return docBase;

        return null;
    }

    public String getAppletName()
    {
        if( active )
            return appletName;

        return null;
    }

    public String getParameter( String name )
    {
        if( active )
            return (String) params.get( name.toUpperCase() );

        return null;
    }

    public boolean isActive()
    {
        return active;
    }

    /*************************************************************************
     ************************* Layout methods ********************************
     *************************************************************************/
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

        public Dimension getPreferredSize()
        {
            return size;
        }

        public Dimension getMinimumSize()
        {
            return size;
        }
    }

}
