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
        panel.setMessage("Loading...");
        add( "Center", panel );
        pack();
        setVisible(true);
        runThread = new Thread
        (
        new Runnable() {
            public void run() {
                //this order is very important and took a long time
                //to figure out- don't modify it unless there are
                //real bug fixes
                // This whole code is very tricky.
                // some strange blocking occurs when loading applets.
                // Display of the applet takes place very late, so that
                // progress messages etc of applets can not be seen.
                // Repaints never happen when you want them to happen
                // during startup.  
                
                active = true;
                //synchronized( loader ) {
                    try {
                        appletClass = loader.loadClass( className );
                    } catch (Exception e) {
                        Main.kjas_err("Class could not be loaded: " + className, e);
                    }
                //}
                if( appletClass == null ) {
                    Main.info( "Could not load applet class " + className);
                    panel.setMessage("Applet Failed");
                    return;
                }                
                try {
                    synchronized (appletClass) {
                        Main.debug("Applet " + appletName + " id=" + appletID + " instantiating...");
                        app = (Applet) appletClass.newInstance();
                        Main.debug("Applet " + appletName + " id=" + appletID + " instantiated.");
                    }
                    app.setStub( me );
                }
                catch( InstantiationException e ) {
                    Main.kjas_err( "Could not instantiate applet", e );
                    panel.setMessage("Applet Failed");
                    return;
                }
                catch( IllegalAccessException e ) {
                    Main.kjas_err( "Could not instantiate applet", e );
                    panel.setMessage("Applet Failed");
                    return;
                }
                
                app.setVisible(false);
                Main.debug("panel.add( \"Center\", app );");                
                panel.setApplet( app );
                //++Main.debug("app.validate();");                
                //++app.validate();
                Main.debug("app.setSize(appletSize);");
                app.setSize(appletSize);
                                
                context.showStatus("Initializing Applet " + appletName + " ...");
                
                Main.debug("Applet " + appletName + " id=" + appletID + " initializing...");
                app.init();
                app.setVisible(true);
                Main.debug("Applet " + appletName + " id=" + appletID + " initialized.");
                
                Main.debug("app.setSize(appletSize);");                
                app.setSize(appletSize);
                
                app.validate();
                
                Main.debug("app.setVisible(true);");                
                app.setVisible(true);
                Main.debug("panel.setVisible(true);");                
                panel.setVisible(true);
                Main.debug("setVisible(true);");                
                setVisible(true);
                //repaint(1L);
                //panel.repaint(1L);
                Main.debug("app.repaint(1L);");                
                app.repaint(1L);
 
                context.showStatus("Starting Applet " + appletName + " ...");
                // create a new thread, so we know, when the applet was started
                
                Main.debug("Applet " + appletName + " id=" + appletID + " starting...");
                new KJASAppletThread(app, "KJAS-Applet-" + appletID + "-" + appletName).start(); 
                Main.debug("Applet " + appletName + " id=" + appletID + " started.");
                
                context.showStatus("Applet " + appletName + " started.");
                //setVisible(true);
            }
        }
        , "KJAS-AppletStub-" + appletID + "-" + appletName);
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
                //pack();
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
        private Label msgLabel = null;
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
        
        public void paint(Graphics g) {
            Main.debug("panel paint");
            super.paint(g);
        }
        
        public void setApplet(Applet applet) {
            if (msgLabel != null) {
                remove(msgLabel);
            }
            add("Center", applet);
            validate();  
        }
        public void setMessage(String msg) {
            if (msgLabel != null) {
                remove(msgLabel);
            }
            msgLabel = new Label(msg, Label.CENTER);
            add("Center", msgLabel);
            validate();
        }
    }
    
    class KJASAppletThread extends Thread {
        private Applet app;
        public KJASAppletThread(Applet app, String name) {
            super(name);
            this.app = app;
        }
        public void run() {
            app.start();
        }
    }
    
 }
