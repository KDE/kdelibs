package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.JFrame;


/**
 * The stub used by Applets to communicate with their environment.
 *
 */
public final class KJASAppletStub
    implements AppletStub
{
    private KJASAppletContext context;    // The containing context.
    private Hashtable         params;     // Maps parameter names to values
    private URL               codeBase;   // The URL directory where files are
    private URL               docBase;    // The document that referenced the applet
    private boolean           active;     // Is the applet active?
    private String            appletName; // The name of this applet instance
    private String            appletID;   // The id of this applet- for use in callbacks
    private Dimension         appletSize;
    private String            windowName;
    private String            className;
    private Class             appletClass;
    private JFrame            frame;
    private boolean           failed = false;
   
    
    /**
    * applet state unknown
    */
    public static final int UNKNOWN = 0;
    /**
    * the applet class has been loaded 
    */
    public static final int CLASS_LOADED = 1;
    /**
    * the applet has been instanciated 
    */
    public static final int INSTANCIATED = 2;
    /**
    * the applet has been initialized 
    */
    public static final int INITIALIZED = 3;
    /**
    * the applet has been started 
    */
    public static final int STARTED = 4;
    /**
    * the applet has been stopped 
    */
    public static final int STOPPED = 5;
    /**
    * the applet has been destroyed 
    */
    public static final int DESTROYED = 6;
   
    
    private int state = UNKNOWN;  
    private KJASAppletClassLoader loader;
    private KJASAppletPanel       panel;
    private Applet                app;
    private Thread                runThread;
    private Thread                appletThread = null;
    private Thread                destroyThread = null;
    KJASAppletStub                me;

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
        context    = _context;
        appletID   = _appletID;
        codeBase   = _codeBase;
        docBase    = _docBase;
        active     = false;
        state      = UNKNOWN;
        appletName = _appletName;
        className  = _className.replace( '/', '.' );
        appletSize = _appletSize;
        params     = _params;
        windowName = _windowName;
        loader     = _loader;
 
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
        
        
    }

    /**
     * Helper function for ending the runThread and appletThread
     **/
    private void tryToStopThread(Thread thread) {
        try {
            thread.interrupt();
        } catch (SecurityException se) {}
        if (thread.isAlive()) {
            try {
                thread.join(5000);
            } catch (InterruptedException ie) {}
        }
    }

    private void stateChange(int newState) {
        if (failed)
            return;
        state = newState;
        Main.protocol.sendAppletStateNotification(
            context.getID(),
            appletID,
            newState);
    }
    
    private void setFailed(String why) {
        failed = true;
        loader.removeStatusListener(panel);
        panel.stopAnimation();
        panel.showFailed();
        Main.protocol.sendAppletFailed(context.getID(), appletID, why); 
    }
    
    void createApplet() {
        panel = new KJASAppletPanel();
        frame = new JFrame(windowName);
        // under certain circumstances, it may happen that the
        // applet is not embedded but shown in a separate window.
        // think of konqueror running under fvwm or gnome.
        // than, the user should have the ability to close the window.
        
        frame.addWindowListener
        (
            new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    me.destroyApplet();
                }
            }
        );
        frame.getContentPane().add( panel, BorderLayout.CENTER );
        if (Main.java_version > 1.399)
            frame.setUndecorated(true);
        frame.setLocation( 0, 0 );
        frame.pack();
        // resize frame for j2sdk1.5beta1..
        if (appletSize.getWidth() > 0)
            frame.setBounds( 0, 0, appletSize.width, appletSize.height );
        else
            frame.setBounds( 0, 0, 50, 50 );
        frame.setVisible(true);
        loader.addStatusListener(panel);
        runThread = new Thread
        (
        new Runnable() {
            public void run() {
                //this order is very important and took a long time
                //to figure out- don't modify it unless there are
                //real bug fixes
                
                // till 2002 09 18
                // commented out the synchronized block because
                // it leads to a deadlock of the JVM with
                // j2re 1.4.1
                //synchronized( me ) {
                    try {
                        appletClass = loader.loadClass( className );
                    } catch (Exception e) {
                        Main.kjas_err("Class could not be loaded: " + className, e);
                        setFailed(e.toString());
                        return;
                    }
                    if (Thread.interrupted())
                        return;
                    stateChange(CLASS_LOADED);
                    Object object = null;
                    try {
                        synchronized (appletClass) {
                           object = appletClass.newInstance();
                           app = (Applet) object;
                        }
                    }
                    catch( InstantiationException e ) {
                        Main.kjas_err( "Could not instantiate applet", e );
                        setFailed(e.toString());
                        return;
                    }
                    catch( IllegalAccessException e ) {
                        Main.kjas_err( "Could not instantiate applet", e );
                        setFailed(e.toString());
                        return;
                    }
                    catch ( ClassCastException e ) {
                        if ( object != null && object instanceof java.awt.Component) {
                            app = new Applet();
                            app.setLayout(new BorderLayout());
                            app.add( (Component) object, BorderLayout.CENTER);
                        } else {
                            setFailed(e.toString());
                            return;
                        }
                    }
                    catch ( Throwable e ) {
                        setFailed(e.toString());
                        return;
                    }
                //} // synchronized
                if (Thread.interrupted())
                    return;
                app.setStub( me );
                stateChange(INSTANCIATED);
                app.setVisible(false);
                panel.setApplet( app );
                if (appletSize.getWidth() > 0)
                    app.setBounds( 0, 0, appletSize.width, appletSize.height );
                else
                    app.setBounds( 0, 0, panel.getSize().width, panel.getSize().height );

                try {
                    app.init();
                } catch (Error er) {
                    Main.info("Error " + er.toString() + " during applet initialization"); 
                    er.printStackTrace();
                    setFailed(er.toString());
                    return;
                } catch (Exception ex) {
                    Main.info("Exception " + ex.toString() + " during applet initialization"); 
                    ex.printStackTrace();
                    setFailed(ex.toString());
                    return;
                }

                if (Thread.interrupted())
                     return;
                stateChange(INITIALIZED);
                loader.removeStatusListener(panel);
                app.setVisible(true);
                //panel.validate();
               
                // stop the loading... animation 
                panel.stopAnimation();
                // create a new thread, so we know, when the applet was started
                /*Thread appletThread = new KJASAppletThread(me, "KJAS-Applet-" + appletID + "-" + appletName); 
                appletThread.start();
                state = STARTED;
                context.showStatus("Applet " + appletName + " started.");*/
           }
        }
        , "KJAS-AppletStub-" + appletID + "-" + appletName);
        runThread.setContextClassLoader(loader);
        runThread.start();
    }

    /**
    * starts the applet managed by this stub by calling the applets start() method.
    * Also marks this stub as active.
    * @see java.applet.Applet#start()
    * @see java.applet.AppletStub#isActive()
    * 
    */
    void startApplet()
    {
        if( app != null && state == INITIALIZED) {
            active = true;
            if (appletThread == null) {
                appletThread = new Thread("KJAS-Applet-" + appletID + "-" + appletName) {
                    public void run() {
                        frame.validate();
                        app.start();
                        app.repaint();
                        appletThread = null;
                    }
                };
                appletThread.start();
            } /*else {
                frame.validate();
                app.start();
                app.repaint();
            }*/
            stateChange(STARTED);
       }
    }

    /**
    * stops the applet managed by this stub by calling the applets stop() method.
    * Also marks this stub as inactive.
    * @see java.applet.Applet#stop()
    * @see java.applet.AppletStub#isActive()
    * 
    */
    void stopApplet()
    {
        if( app != null ) {
            if( appletThread != null && appletThread.isAlive() ) {
                Main.debug( "appletThread is active when stop is called" );
                tryToStopThread(appletThread);
            }
            appletThread = null;
            active = false;
            app.stop();
            stateChange(STOPPED);
            if (Main.java_version > 1.399) {
                // kill the windowClosing listener(s)
                WindowListener[] wl = frame.getWindowListeners();
                for (int i = 0; wl != null && i < wl.length; i++)
                    frame.removeWindowListener(wl[i]);
            }
            frame.hide();
        }
    }

    /**
    * initialize the applet managed by this stub by calling the applets init() method.
    * @see java.applet.Applet#init()
    */
    void initApplet()
    {
        if( app != null ) {
            app.init();
        }
        stateChange(INITIALIZED);
   }

    /**
    * destroys the applet managed by this stub by calling the applets destroy() method.
    * Also marks the the applet as inactive.
    * @see java.applet.Applet#init()
    */
    synchronized void destroyApplet()
    {
        if (state >= DESTROYED || destroyThread != null)
            return;
        destroyThread = new Thread("applet destroy thread") {
            public void run() {
                if( runThread != null && runThread.isAlive() ) {
                    Main.debug( "runThread is active when stub is dying" );
                    tryToStopThread(runThread);
                    runThread = null;
                    panel.stopAnimation();
                    loader.removeStatusListener(panel);
                }
                if( app != null ) {
                    synchronized (app) {
                        if (active) {
                            try {
                                stopApplet();
                            } catch (Exception e) {
                            }
                        }
                        try {
                            app.destroy();
                        } catch (Exception e) {
                        }
                    }
                    frame.dispose();
                    app = null;
                    stateChange(DESTROYED);
                }

                loader = null;
                context = null;
                destroyThread = null;
            }
        };
        destroyThread.start();
    }

    static void waitForDestroyThreads()
    {
        Thread [] ts = new Thread[Thread.activeCount() + 5];
        int len = Thread.enumerate(ts);
        for (int i = 0; i < len; i++) {
            try {
                if (ts[i].getName() != null && 
                        ts[i].getName().equals("applet destroy thread")) {
                    try {
                        ts[i].join(10000);
                    } catch (InterruptedException ie) {}
                }
            } catch (Exception e) {}
        }
    }

    /**
    * get the Applet managed by this stub.
    * @return the Applet or null if the applet could not be loaded
    * or instanciated.
    */
    Applet getApplet()
    {
            return app;
    }

    /**
    * get a parameter value given in the &lt;APPLET&gt; tag 
    * @param name the name of the parameter
    * @return the value  or null if no parameter with this name exists.
    */
    
    public String getParameter( String name )
    {
        return (String) params.get( name.toUpperCase() );
    }

    /**
    * implements the isActive method of the AppletStub interface.
    * @return if the applet managed by this stub is currently active. 
    * @see java.applet.AppletStub#isActive()
    */
    public boolean isActive()
    {
        return active;
    }

    /**
    * determines if the applet has been loaded and instanciated
    * and can hence be used.
    * @return true if the applet has been completely loaded.
    */
    boolean isLoaded() {
        return state >= INSTANCIATED;
    }
    
    public void appletResize( int width, int height )
    {
        if( active )
        {
            if ( (width >= 0) && (height >= 0))
            {
                Main.debug( "Applet #" + appletID + ": appletResize to : (" + width + ", " + height + ")" );
                Main.protocol.sendResizeAppletCmd( context.getID(), appletID, width, height );
                appletSize = new Dimension( width, height );
                //pack();
            }
        }
    }

    /*************************************************************************
     ********************** AppletStub Interface *****************************
     *************************************************************************/
    /**
    * implements the getAppletContext method of the AppletStub interface.
    * @return the AppletContext to which this stub belongs.
    * @see java.applet.AppletStub#getAppletContext()
    */
    public AppletContext getAppletContext()
    {
        return context;
    }

    /**
    * implements the getCodeBase method of the AppletStub interface.
    * @return the code base of the applet as given in the &lt;APPLET&gt; tag.
    * @see java.applet.AppletStub#getCodeBase()
    */
    public URL getCodeBase()
    {
        return codeBase;
    }

    /**
    * implements the getDocumentBase method of the AppletStub interface.
    * @return the code base of the applet as given in the 
    * &lt;APPLET&gt; tag or determined by the containing page.
    * @see java.applet.AppletStub#getDocumentBase()
    */
    public URL getDocumentBase()
    {
        return docBase;
    }

    /**
    * get the applet's name
    * @return the name of the applet as given in the 
    * &lt;APPLET&gt; tag or determined by the <em>code</em> parameter.
    */
    public String getAppletName()
    {
        return appletName;
    }

    
    /******************************************************************/
    /**
    * Helper class
    */
    class KJASAppletThread extends Thread {
        private KJASAppletStub  stub;
        public KJASAppletThread(KJASAppletStub stub, String name) {
            super(name);
            this.stub = stub;
        }
        public void run() {
            stub.startApplet();
        }
    }
 }
