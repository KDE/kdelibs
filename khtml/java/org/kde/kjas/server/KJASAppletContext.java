package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

/**
 * The context in which applets live.
 */
public class KJASAppletContext implements AppletContext
{
    private Hashtable stubs;
    private Hashtable images;
    private Hashtable streams;

    private String myID;
    private KJASAppletClassLoader loader;
    private boolean active;

    // we need a mapping JS referenced Java objects
    // for now, remember only the last one
    private Object get_member_obj = null;

    private final static int JError    = 0;
    private final static int JBoolean  = 1;
    private final static int JFunction = 2;
    private final static int JNumber   = 3;
    private final static int JObject   = 4;
    private final static int JString   = 5;
    private final static int JVoid     = 6;

    /**
     * Create a KJASAppletContext
     */
    public KJASAppletContext( String _contextID )
    {
        stubs  = new Hashtable();
        images = new Hashtable();
        streams = new Hashtable();
        myID   = _contextID;
        active = true;
    }

    public String getID()
    {
        return myID;
    }

    public void createApplet( String appletID, String name,
                              String className, String docBase,
                              String codeBase, String archives,
                              String width, String height,
                              String windowName, Hashtable params )
    {
        //do kludges to support mess with parameter table and
        //the applet variables
        String key = new String( "archive" ).toUpperCase();
        if( archives == null )
        {
            if( params.containsKey( key ) )
                archives = (String)params.get( key );
        }
        else
        {
            if( !params.containsKey( key ) )
                params.put( key, archives );
        }

        key = new String( "codebase" ).toUpperCase();
        if( codeBase == null )
        {
            if( params.containsKey( key ) )
                codeBase = (String) params.get( key );
        }

        key = new String( "width" ).toUpperCase();
        if( !params.containsKey( key ) )
            params.put( key, width );
        key = new String( "height" ).toUpperCase();
        if( !params.containsKey( key ) )
            params.put( key, height );

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
                    loader.addArchiveName( jar );
                }
            }
            loader.paramsDone();

            KJASAppletStub stub = new KJASAppletStub
            (
                this, appletID, loader.getCodeBase(),
                loader.getDocBase(), name, className,
                new Dimension( Integer.parseInt(width), Integer.parseInt(height) ),
                params, windowName, loader
            );
            stubs.put( appletID, stub );

            stub.createApplet();
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
            Main.debug( "could not init and show applet: " + appletID );
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
            Main.debug( "could not destroy applet: " + appletID );
        }
        else
        {
            Main.debug( "stopping applet: " + appletID );
            stub.die();

            stubs.remove( appletID );
        }
    }

    public void startApplet( String appletID )
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if( stub == null )
        {
            Main.debug( "could not start applet: " + appletID );
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
            Main.debug( "could not stop applet: " + appletID );
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
        active = false;
    }

    /***************************************************************************
    **** AppletContext interface
    ***************************************************************************/
    public Applet getApplet( String appletName )
    {
        if( active )
        {
            Enumeration e = stubs.elements();
            while( e.hasMoreElements() )
            {
                KJASAppletStub stub = (KJASAppletStub) e.nextElement();

                if( stub.getAppletName().equals( appletName ) )
                    return stub.getApplet();
            }
        }

        return null;
    }

    public Enumeration getApplets()
    {
        if( active )
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

        return null;
    }

    public AudioClip getAudioClip( URL url )
    {
        Main.debug( "getAudioClip, url = " + url );

        return new KJASSoundPlayer( url );
    }

    public void addImage( String url, byte[] data )
    {
        Main.debug( "addImage for url = " + url );
        images.put( url, data );
    }
    
    public Image getImage( URL url )
    {
        if( active && url != null )
        {
            //check with the Web Server        
            String str_url = url.toString();
            Main.debug( "getImage, url = " + str_url );
            Main.protocol.sendGetURLDataCmd( myID, str_url );

            while( !images.containsKey( str_url ) && active )
        {
                try { Thread.sleep( 200 ); }
                catch( InterruptedException e ){}
            }
            if( images.containsKey( str_url ) )
            {
                byte[] data = (byte[]) images.get( str_url );
                if( data.length > 0 )
                {
            Toolkit kit = Toolkit.getDefaultToolkit();
                    return kit.createImage( data );
                } else return null;
            }
        }

        return null;
    }

    public void showDocument( URL url )
    {
        Main.debug( "showDocument, url = " + url );

        if( active && (url != null) )
        {
            Main.protocol.sendShowDocumentCmd( myID, url.toString()  );
        }
    }

    public void showDocument( URL url, String targetFrame )
    {
        Main.debug( "showDocument, url = " + url + " targetFrame = " + targetFrame );

        if( active && (url != null) && (targetFrame != null) )
        {
                Main.protocol.sendShowDocumentCmd( myID, url.toString(), targetFrame );
        }
    }

    public void showStatus( String message )
    {
        if( active && (message != null) )
        {
            Main.protocol.sendShowStatusCmd( myID, message );
        }
    }
    public String evaluateJavaScript(String script) {
        if( active && (script != null) ) {
            Main.liveconnect_thread = Thread.currentThread();
            Main.protocol.sendEvaluateJavaScriptCmd(myID, script);
            try {
                Thread.currentThread().sleep(30000);
            } catch (InterruptedException ex) {}
            String retval = Main.liveconnect_returnval;
            Main.liveconnect_returnval = null;
            return retval; 
        }
        return null;
    }
    private int findMember(Class c, Object obj, String name, StringBuffer value)
    {
        if (c == null)
            return JError;
        try {
            try {
                Field field = c.getField(name);
                Object fo = field.get(obj);
                value.insert(0, fo.toString());
                String type = field.getType().getName();
                if (type.equals("boolean") || type.equals("java.lang.Boolean"))
                    return JBoolean;
                if (type.equals("int") || type.equals("long") || 
                    type.equals("float") || type.equals("double") ||
                    fo instanceof java.lang.Number)
                    return JNumber;
                if (type.equals("java.lang.String"))
                    return JString;
                get_member_obj = fo;
                return JObject;
            } catch (Exception ex) {
                Method [] m = c.getDeclaredMethods();
                for (int i = 0; i < m.length; i++)
                    if (m[i].getName().equals(name)) {
                        return JFunction;
                    }
                return findMember(c.getSuperclass(), obj, name, value);
            }
        } catch (Exception e) {
            Main.debug("findMember throwed exception: " + e.toString());
        }
        return JError;
    }
    public int getMember(String appletID, String name, StringBuffer value)
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if(stub == null) {
            Main.debug( "could not get value of applet: " + appletID );
            return 0;
        }
        Main.debug("getMember: " + name);
        Object obj;
        int basename = name.lastIndexOf('.');
        if (basename > -1) {
            obj = get_member_obj;
            name = name.substring(basename + 1);
            Main.debug("getMember basename: " + name);
        } else {
            obj = stub.getApplet();
        }
        return findMember(obj.getClass(), obj, name, value);
    }
    private Method findMethod(Class c, String name, Class [] argcls) {
        if (c == null)
            return null;
        try {
            Method m = c.getDeclaredMethod(name, argcls);
            return m;
        } catch (Exception e) {
            return findMethod(c.getSuperclass(), name, argcls);
        }
    }
    public int callMember(String appletID, String name, StringBuffer value, java.util.List args)
    {
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if(stub == null) {
            Main.debug( "could not get value of applet: " + appletID );
            return JError;
        }
        try {
            Main.debug("callMember: " + name);
            Object obj;
            int basename = name.lastIndexOf('.');
            if (basename > -1) {
                obj = get_member_obj;
                name = name.substring(basename + 1);
                Main.debug("callMember basename: " + name);
            } else {
                obj = stub.getApplet();
            }
	    Class c = obj.getClass();
            String type;
            Class [] argcls = new Class[args.size()];
            for (int i = 0; i < args.size(); i++)
                argcls[i] = name.getClass(); // String for now
            Method m = findMethod(c, name, argcls);
            Object [] argobj = new Object[args.size()];
            for (int i = 0; i < args.size(); i++)
                argobj[i] = args.get(i); //for now
            type = m.getReturnType().getName();
            Object retval =  m.invoke(obj, argobj);
            if (retval == null)
                return JVoid; // void
            value.insert(0, retval.toString());
            Main.debug( "Call value of object: " + value + " " + type );
            if (type.equals("boolean") || type.equals("java.lang.Boolean"))
                return JBoolean;
            if (type.equals("int") || type.equals("java.lang.Integer"))
                return JNumber;
            return JString;
        } catch (Exception e) {
            Main.debug("callMember throwed exception: " + e.toString());
            e.printStackTrace();
        }
        return JError;
    }

    public void setStream(String key, InputStream stream) throws IOException {
        Main.debug("setStream, key = " + key);
        streams.put(key, stream);
    }
    public InputStream getStream(String key){
        Main.debug("getStream, key = " + key);
        return (InputStream) streams.get(key);
    }
    public Iterator getStreamKeys() {
        Main.debug("getStreamKeys");
        return streams.keySet().iterator();
    }

}
