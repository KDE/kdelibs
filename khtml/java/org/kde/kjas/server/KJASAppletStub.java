package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;


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
        String fixedClassName = _className;
        if (_className.endsWith(".class") || _className.endsWith(".CLASS"))
        {
            fixedClassName = _className.substring(0, _className.length()-6);   
        }
        else if (_className.endsWith(".java")|| _className.endsWith(".JAVA"))
        {
            fixedClassName = _className.substring(0, _className.length()-5);   
        }
        className = fixedClassName.replace('/', '.');
            
        appletClass = null;
        me = this;
        
        // under certain circumstances, it may happen that the
        // applet is not embedded but shown in a separate window.
        // think of konqueror running under fvwm or gnome.
        // than, the user should have the ability to close the window.
        
        addWindowListener
        (
            new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    me.die();
                }
            }
        );
        
    }

    /*************************************************************************
     *********************** Runnable Interface ******************************
     *************************************************************************/
    public synchronized void createApplet() {
        panel = new KJASAppletPanel( appletSize );
        add( "Center", panel );
        pack();
        setVisible(true);
        loader.addStatusListener(panel);
        runThread = new Thread
        (
        new Runnable() {
            public void run() {
                //this order is very important and took a long time
                //to figure out- don't modify it unless there are
                //real bug fixes
                
                synchronized( me ) {
                    active = true;
                    try {
                        appletClass = loader.loadClass( className );
                    } catch (Exception e) {
                        Main.kjas_err("Class could not be loaded: " + className, e);
                    }
                    if( appletClass == null ) {
                        Main.info( "Could not load applet class " + className);
                        panel.showFailed();
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
                        panel.showFailed();
                        return;
                    }
                    catch( IllegalAccessException e ) {
                        Main.kjas_err( "Could not instantiate applet", e );
                        panel.showFailed();
                        return;
                    }
                } // synchronized
                app.setVisible(false);
                Main.debug("panel.add( \"Center\", app );");                
                panel.setApplet( app );
                
                Main.debug("app.setSize(appletSize);");
                app.setSize(appletSize);
                                
                context.showStatus("Initializing Applet " + appletName + " ...");
                
                Main.debug("Applet " + appletName + " id=" + appletID + " initializing...");
                try {
                    app.init();
                } catch (Error e) {
                    Main.info("Error " + e.toString() + " during applet initialization"); 
                    e.printStackTrace();
                    return;
                }
                loader.removeStatusListener(panel);
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
                //--Main.debug("app.repaint(1L);");                
                app.repaint(0L);
 
                context.showStatus("Starting Applet " + appletName + " ...");
                // create a new thread, so we know, when the applet was started
                
                Main.debug("Applet " + appletName + " id=" + appletID + " starting...");
                Thread appletThread = new KJASAppletThread(app, "KJAS-Applet-" + appletID + "-" + appletName); 
                panel.stopAnimation();
                appletThread.start();
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
        //synchronized ( this ) {
            return app;
        //}
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
    class KJASAppletPanel extends javax.swing.JPanel implements StatusListener
    {
        private Dimension size;
        private Image img = null;
        private boolean showStatusFlag = true;
        private Font font;
        private String msg = "Loading Applet...";
        public KJASAppletPanel( Dimension _size )
        {
            super( new BorderLayout() );
            size = _size;
            font = new Font("SansSerif", Font.PLAIN, 10);
            URL url = getClass().getClassLoader().getResource("images/animbean.gif");
            img = getToolkit().createImage(url);
            //setBackground(Color.white);
        }

        void setAppletSize( Dimension _size )
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
        
        
        void setApplet(Applet applet) {
            add("Center", applet);
            validate();  
        }
        
        public void showStatus(String msg) {
            this.msg = msg;
            repaint();
        }
        
        public void paint(Graphics g) {
            super.paint(g);
            if (showStatusFlag) {
                int x = getWidth() / 2;
                int y = getHeight() / 2;
                if (img != null) {
                    //synchronized (img) {
                        int w = img.getWidth(this);
                        int h = img.getHeight(this);
                        int imgx = x - w/2;
                        int imgy = y - h/2;
                        //g.setClip(imgx, imgy, w, h);
                        g.drawImage(img, imgx, imgy, this);
                        y += img.getHeight(this)/2;
                    //}
                }
                if (msg != null) {
                    //synchronized(msg) {
                        g.setFont(font);
                        FontMetrics m = g.getFontMetrics();
                        int h = m.getHeight();
                        int w = m.stringWidth(msg);
                        int msgx = x - w/2;
                        int msgy = y + h;
                        //g.setClip(0, y, getWidth(), h);
                        g.drawString(msg, msgx, msgy); 

                    //}
                }
            }
        }
        void showFailed() {
            URL url = getClass().getClassLoader().getResource("images/brokenbean.gif");
            img = getToolkit().createImage(url);
            msg = "Applet Failed.";
            repaint();
        }
        
        void showFailed(String message) {
            showFailed();
            showStatus(message);
        }
        
        public void stopAnimation() {
            showStatusFlag = false;
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
