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
    private Vector    pendingImages;
    private Hashtable streams;

    private String myID;
    private KJASAppletClassLoader loader;
    private boolean active;
    private int refcounter = 0;
    // a mapping JS referenced Java objects
    private Hashtable jsReferencedObjects;

    private final static int JError    = 0;
    private final static int JArray    = 1;
    private final static int JBoolean  = 2;
    private final static int JFunction = 3;
    private final static int JNull     = 4;
    private final static int JNumber   = 5;
    private final static int JObject   = 6;
    private final static int JString   = 7;
    private final static int JVoid     = 8;

    /**
     * Create a KJASAppletContext
     */
    public KJASAppletContext( String _contextID )
    {
        stubs  = new Hashtable();
        images = new Hashtable();
        pendingImages = new Vector();
        streams = new Hashtable();
        jsReferencedObjects = new Hashtable();
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
        if (params.containsKey(key)) {
            if (archives == null) {
                // There is no 'archive' attribute
                // but a 'archive' param. fix archive list 
                // from param value
                archives = (String)params.get(key);
            } else {
                // there is already an archive attribute.
                // just add the value of the param to the list
                archives += "," + (String)params.get(key);
            }
        } else if (archives != null) {
            // add param if it is not present
            params.put( key, archives);
        }
        
        /*
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
        */
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
            loader.setAppletContext(this);
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
                v.add( stub.getApplet() );
            }

            return v.elements();
        }

        return null;
    }

    public AudioClip getAudioClip( URL url )
    {
        Main.debug( "getAudioClip, url = " + url );
        AudioClip clip = java.applet.Applet.newAudioClip(url); 
        Main.debug( "got AudioClip " + clip);
        return clip;
        // return new KJASSoundPlayer( myID, url );
    }

    public void addImage( String url, byte[] data )
    {
        Main.debug( "addImage for url = " + url );
        images.put( url, data );
        if (Main.cacheImages) {
            pendingImages.remove(url);
        }
    }

    public Image getImage( URL url )
    {
        if( active && url != null )
        {
            // directly load images using JVM
            if (true) {
                // Main.info("Getting image using ClassLoader:" + url); 
                if (loader != null) {
                    url = loader.findResource(url.toString());
                    Main.debug("Resulting URL:" + url);
                }
                Toolkit kit = Toolkit.getDefaultToolkit();
                Image img = kit.createImage(url);
                return img;
            }

            //check with the Web Server
            String str_url = url.toString();
            Main.debug( "getImage, url = " + str_url );
            if (Main.cacheImages && images.containsKey(str_url)) {
                Main.debug("Cached: url=" + str_url);
            }
            else
            {
                if (Main.cacheImages) {
                    if (!pendingImages.contains(str_url)) {
                        Main.protocol.sendGetURLDataCmd( myID, str_url );
                        pendingImages.add(str_url);
                    }
                } else {
                    Main.protocol.sendGetURLDataCmd( myID, str_url );
                }
                while( !images.containsKey( str_url ) && active )
                {
                    try { Thread.sleep( 200 ); }
                    catch( InterruptedException e ){}
                }
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
    private int getTypedValue(Object obj, StringBuffer value) {
        String val = obj.toString();
        int jtype;
        String type = obj.getClass().getName();
        if (type.equals("boolean") || type.equals("java.lang.Boolean"))
            jtype = JBoolean;
        else if (type.equals("int") || type.equals("long") || 
                type.equals("float") || type.equals("double") ||
                type.equals("byte") || obj instanceof java.lang.Number)
            jtype = JNumber;
        else if (type.equals("java.lang.String"))
            jtype = JString;
        else {
            jtype = JObject;
            val = Integer.toString(++refcounter);
            jsReferencedObjects.put(new Integer(refcounter), obj);
        }
        value.insert(0, val);
        return jtype;
    }
    private int getMemberAux(Class c, Object obj, String name, StringBuffer value) {
        if (c == null)
            return JError;
        try {
            Field field = c.getField(name);
            return getTypedValue(field.get(obj), value);
        } catch (Exception ex) {
            Method [] m = c.getDeclaredMethods();
            for (int i = 0; i < m.length; i++)
                if (m[i].getName().equals(name)) {
                    return JFunction;
                }
            return getMemberAux(c.getSuperclass(), obj, name, value);
        }
    }
    private Object [] getObjectField(String appletID, String name) {
        Object [] ret = new Object[2];
        KJASAppletStub stub = (KJASAppletStub) stubs.get( appletID );
        if(stub == null) {
            Main.debug( "could not get value of applet: " + appletID );
            return null;
        }
        int basename = name.lastIndexOf('.');
        if (basename > -1) {
            ret[0] = jsReferencedObjects.get(new Integer(name.substring(0, basename)));
            if (ret[0] == null) {
                Main.debug( "could not get referenced object" );
                return null;
            }
            ret[1] = name.substring(basename + 1);
        } else {
            ret[0] = stub.getApplet();
            ret[1] = name;
        }
        Main.debug("getObjectField basename: " + (String) ret[1]);
        return ret;
    }
    public int getMember(String appletID, String name, StringBuffer value)
    {
        Main.debug("getMember: " + name);
        Object [] objs = getObjectField(appletID, name);
        if(objs == null)
            return JError;
        return getMemberAux(objs[0].getClass(), objs[0], (String) objs[1], value);
    }
    private Field findField(Class c, String name) {
        if (c == null)
            return null;
        try {
            return c.getField(name);
        } catch (Exception e) {
            return findField(c.getSuperclass(), name);
        }
    }
    public boolean putMember(String appletID, String name, String value)
    {
        Object [] objs = getObjectField(appletID, name);
        if(objs == null) {
            Main.debug("Error in putValue: applet " + appletID + " not found");
            return false;
        }
        Field f = findField(objs[0].getClass(), (String) objs[1]);
        if (f == null) {
            Main.debug("Error in putValue: " + name + " not found");
            return false;
        }
        try {
            String type = f.getType().getName();
            Main.debug("putValue: (" + type + ")" + name + "=" + value);
            if (type.equals("boolean"))
                f.setBoolean(objs[0], Boolean.getBoolean(value));
            else if (type.equals("java.lang.Boolean"))
                f.set(objs[0], Boolean.valueOf(value));
            else if (type.equals("int"))
                f.setInt(objs[0], Integer.parseInt(value));
            else if (type.equals("java.lang.Integer"))
                f.set(objs[0], Integer.valueOf(value));
            else if (type.equals("byte"))
                f.setByte(objs[0], Byte.parseByte(value));
            else if (type.equals("java.lang.Byte"))
                f.set(objs[0], Byte.valueOf(value));
            else if (type.equals("char"))
                f.setChar(objs[0], value.charAt(0));
            else if (type.equals("java.lang.Character"))
                f.set(objs[0], new Character(value.charAt(0)));
            else if (type.equals("double"))
                f.setDouble(objs[0], Double.parseDouble(value));
            else if (type.equals("java.lang.Double"))
                f.set(objs[0], Double.valueOf(value));
            else if (type.equals("float"))
                f.setFloat(objs[0], Float.parseFloat(value));
            else if (type.equals("java.lang.Float"))
                f.set(objs[0], Float.valueOf(value));
            else if (type.equals("long"))
                f.setLong(objs[0], Long.parseLong(value));
            else if (type.equals("java.lang.Long"))
                f.set(objs[0], Long.valueOf(value));
            else if (type.equals("short"))
                f.setShort(objs[0], Short.parseShort(value));
            else if (type.equals("java.lang.Short"))
                f.set(objs[0], Short.valueOf(value));
            else if (type.equals("java.lang.String"))
                f.set(objs[0], value);
            else {
                Main.debug("Error putValue: unsupported type: " + type);
                return false;
            }
            return true;
        } catch (Exception e) {
            Main.debug("Exception in putValue: " + e.getMessage());
            return false;
        }
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
        Object [] objs = getObjectField(appletID, name);
        if(objs == null)
            return JError;
        try {
            Main.debug("callMember: " + name);
            Object obj;
            Class c = objs[0].getClass();
            String type;
            Class [] argcls = new Class[args.size()];
            for (int i = 0; i < args.size(); i++)
                argcls[i] = name.getClass(); // String for now
            Method m = findMethod(c, (String) objs[1], argcls);
            Object [] argobj = new Object[args.size()];
            for (int i = 0; i < args.size(); i++)
                argobj[i] = args.get(i); //for now
            Object retval =  m.invoke(objs[0], argobj);
            if (retval == null)
                return JVoid; // void
            return getTypedValue(retval, value);
        } catch (Exception e) {
            Main.debug("callMember throwed exception: " + e.toString());
            e.printStackTrace();
        }
        return JError;
    }
    public void derefObject(int objid) {
        if (jsReferencedObjects.remove(new Integer(objid)) == null)
            Main.debug("couldn't remover referenced object");
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
