package org.kde.kjas.server;

import java.applet.*;
import java.util.*;
import java.net.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import org.kde.javascript.JSObject;

final class KJASAuthenticator extends Authenticator {
    private Hashtable authentication;

    KJASAuthenticator() {
        authentication = new Hashtable();
        setDefault(this);
    }
    final void addURL(URL url, String user, String password, String authname) {
        String key = new String(url.getProtocol() + ":" + url.getHost() + ":" +
                url.getPort() + "_" + authname);
        String [] auths = { user, password };
        authentication.put(key, auths);
    }
    final protected PasswordAuthentication getPasswordAuthentication() {
        URL url;
        String key = new String(getRequestingProtocol() + ":" + getRequestingHost() + ":" + getRequestingPort() + "_" + getRequestingPrompt());
        String [] auths = (String []) authentication.get(key);
        if (auths != null) {
            char [] pw = new char[auths[1].length()];
            auths[1].getChars(0, auths[1].length(), pw, 0);
            return new PasswordAuthentication(auths[0], pw);
        }
        return null;
    }
}

/**
 * The context in which applets live.
 */
public class KJASAppletContext implements AppletContext
{
    private Hashtable stubs;
    private Hashtable images;
    private Vector    pendingImages;
    private Hashtable streams;
    private Stack jsobjects;

    private String myID;
    private KJASAppletClassLoader loader;
    private boolean active;
    // a mapping JS referenced Java objects
    private Hashtable jsReferencedObjects;
    private final static KJASAuthenticator authenticator = new KJASAuthenticator();
    // keep this in sync with KParts::LiveConnectExtension::Type
    private final static int JError    = -1;
    private final static int JVoid     = 0;
    private final static int JBoolean  = 1;
    private final static int JFunction = 2;
    private final static int JNumber   = 3;
    private final static int JObject   = 4;
    private final static int JString   = 5;

    /**
     * Create a KJASAppletContext
     */
    public KJASAppletContext( String _contextID )
    {
        stubs  = new Hashtable();
        images = new Hashtable();
        pendingImages = new Vector();
        streams = new Hashtable();
        jsobjects = new Stack();
        jsReferencedObjects = new Hashtable();
        myID   = _contextID;
        active = true;
    }

    public String getID()
    {
        return myID;
    }

    public String getAppletID(Applet applet) 
    {
        Enumeration e = stubs.keys();
        while ( e.hasMoreElements() )
        {
            String appletID = (String) e.nextElement();
            KJASAppletStub stub = (KJASAppletStub) stubs.get(appletID);
            if (stub.getApplet() == applet)
                return appletID;
        }
        return null;
    }

    public Applet getAppletById(String appletId) {
        return ((KJASAppletStub) stubs.get( appletId )).getApplet();
    }

    public Object getJSReferencedObject(int objid) {
        return jsReferencedObjects.get(new Integer(objid));
    }

    public String getAppletName(String appletID) {
        KJASAppletStub stub = (KJASAppletStub) stubs.get(appletID);
        if (stub == null)
            return null;
        return stub.getAppletName();
    }
    public void createApplet( String appletID, String name,
                              String className, String docBase,
                              String username, String password, String authname,
                              String codeBase, String archives,
                              String width, String height,
                              String windowName, Hashtable params )
    {
        //do kludges to support mess with parameter table and
        //the applet variables
        String key = new String( "archive" ).toUpperCase();
        if (params.containsKey(key)) {
            String param_archive = (String)params.get(key);
            if (archives == null) {
                // There is no 'archive' attribute
                // but a 'archive' param. fix archive list 
                // from param value
                archives = param_archive;
            } else {
                // there is already an archive attribute.
                // just add the value of the param to the list.
                // But ignore bill$ personal archive format called
                // .cab because java doesn't understand it.
                if (!param_archive.toLowerCase().endsWith(".cab")) {
                    archives =  param_archive + "," + archives;
                }
            }
        } else if (archives != null) {
            // add param if it is not present
            params.put( key, archives);
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

        if (username != null && !username.equals("")) {
            try {
                URL url = new URL(docBase);
                int port = url.getPort();
                if (port < 0)
                    port = url.getDefaultPort();
                authenticator.addURL(new URL(url.getProtocol(), url.getHost(), port, ""), username, password, authname);
            } catch (MalformedURLException muex) {
            }
        }
        try
        {
            String sorted_archives = "";
            TreeSet archive_set = new TreeSet();
            if( archives != null )
            {
                StringTokenizer parser = new StringTokenizer( archives, ",", false );
                while( parser.hasMoreTokens() )
                    archive_set.add ( parser.nextToken().trim() );
            }
            Iterator it = archive_set.iterator();
            while (it.hasNext())
                sorted_archives += (String) it.next();
            KJASAppletClassLoader loader =
                KJASAppletClassLoader.getLoader( docBase, codeBase, sorted_archives );
            it = archive_set.iterator();
            while (it.hasNext())
                loader.addArchiveName( (String) it.next() );
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
            //Main.debug( "stopping applet: " + appletID );
            stubs.remove( appletID );

            stub.destroyApplet();
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
            stub.destroyApplet();
        }

        stubs.clear();
        jsReferencedObjects.clear();
        jsobjects.clear();
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
        //AudioClip clip = java.applet.Applet.newAudioClip(url); 
        AudioClip clip = new KJASAudioClip(url); 
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
                    //Main.debug("Resulting URL:" + url);
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
        //Main.debug( "showDocument, url = " + url );

        if( active && (url != null) )
        {
            Main.protocol.sendShowDocumentCmd( myID, url.toString()  );
        }
    }

    public void showDocument( URL url, String targetFrame )
    {
        //Main.debug( "showDocument, url = " + url + " targetFrame = " + targetFrame );

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
    public void evaluateJavaScript(String script, String appletID, JSObject jso) {
        if( active ) {
            if( jso != null ) {
                synchronized (jsobjects) {
                    jsobjects.push(jso);
                }
            }
            int [] types = { JString };
            String [] arglist = { script };
            Main.protocol.sendJavaScriptEventCmd(myID, appletID, 0, "eval", types, arglist);
        }
    }
    private int[] getJSTypeValue(Object obj, int objid, StringBuffer value) {
        String val = obj.toString();
        int[] rettype = { JError, objid };
        String type = obj.getClass().getName();
        if (type.equals("boolean") || type.equals("java.lang.Boolean"))
            rettype[0] = JBoolean;
        else if (type.equals("int") || type.equals("long") || 
                type.equals("float") || type.equals("double") ||
                type.equals("byte") || obj instanceof java.lang.Number)
            rettype[0] = JNumber;
        else if (type.equals("java.lang.String"))
            rettype[0] = JString;
        else {
            rettype[0] = JObject;
            rettype[1] = obj.hashCode();
            jsReferencedObjects.put(new Integer(rettype[1]), obj);
        }
        value.insert(0, val);
        return rettype;
    }

    public int[] getMember(String appletID, int objid, String name, StringBuffer value)
    {
        Main.debug("getMember: " + name);
        Object o = null;
        KJASAppletStub stub = null;
        if (objid != 0)
            o = jsReferencedObjects.get(new Integer(objid));
        else {
            stub = (KJASAppletStub) stubs.get( appletID );
            if (stub != null)
                o = ((KJASAppletStub) stubs.get( appletID )).getApplet();
        } 
        int ret[] = { JError, objid };
        if (o == null || (stub != null && !stub.isLoaded()))
            return ret;

        Class c = o.getClass();
        try {
            Field field = c.getField(name);
            ret = getJSTypeValue(field.get(o), objid, value);
        } catch (Exception ex) {
            if (name.equals("getClass"))
                return ret;
            Method [] m = c.getMethods();
            for (int i = 0; i < m.length; i++)
                if (m[i].getName().equals(name)) {
                    ret[0] = JFunction;
                    break;
                }
        }
        return ret;
    }

    public boolean putMember(String appletID, int objid, String name, String value)
    {
        if (name.equals("__lc_ret")) {
            // special case; return value of JS script evaluation
            Main.debug("putValue: applet " + name + "=" + value);
            JSObject jso = null;
            synchronized (jsobjects) {
                if (!jsobjects.empty())
                    jso = (JSObject) jsobjects.pop();
            }
            if (jso == null)
                return false;
            jso.returnvalue = value;
            try {
                jso.thread.interrupt();
            } catch (SecurityException ex) {}
            return true;
        }
        KJASAppletStub stub = null;
        Object o = null;
        if (objid != 0)
            o = jsReferencedObjects.get(new Integer(objid));
        else {
            stub = (KJASAppletStub) stubs.get( appletID );
            if (stub != null)
                o = ((KJASAppletStub) stubs.get( appletID )).getApplet();
        }
        if (o == null || (stub != null && !stub.isLoaded())) {
            Main.debug("Error in putValue: applet " + appletID + " not found");
            return false;
        }
        Field f;
        try {
            f = o.getClass().getField(name);
        } catch (Exception e) {
            return false;
        }
        if (f == null) {
            Main.debug("Error in putValue: " + name + " not found");
            return false;
        }
        try {
            String type = f.getType().getName();
            Main.debug("putValue: (" + type + ")" + name + "=" + value);
            if (type.equals("boolean"))
                f.setBoolean(o, Boolean.getBoolean(value));
            else if (type.equals("java.lang.Boolean"))
                f.set(o, Boolean.valueOf(value));
            else if (type.equals("int"))
                f.setInt(o, Integer.parseInt(value));
            else if (type.equals("java.lang.Integer"))
                f.set(o, Integer.valueOf(value));
            else if (type.equals("byte"))
                f.setByte(o, Byte.parseByte(value));
            else if (type.equals("java.lang.Byte"))
                f.set(o, Byte.valueOf(value));
            else if (type.equals("char"))
                f.setChar(o, value.charAt(0));
            else if (type.equals("java.lang.Character"))
                f.set(o, new Character(value.charAt(0)));
            else if (type.equals("double"))
                f.setDouble(o, Double.parseDouble(value));
            else if (type.equals("java.lang.Double"))
                f.set(o, Double.valueOf(value));
            else if (type.equals("float"))
                f.setFloat(o, Float.parseFloat(value));
            else if (type.equals("java.lang.Float"))
                f.set(o, Float.valueOf(value));
            else if (type.equals("long"))
                f.setLong(o, Long.parseLong(value));
            else if (type.equals("java.lang.Long"))
                f.set(o, Long.valueOf(value));
            else if (type.equals("short"))
                f.setShort(o, Short.parseShort(value));
            else if (type.equals("java.lang.Short"))
                f.set(o, Short.valueOf(value));
            else if (type.equals("java.lang.String"))
                f.set(o, value);
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
        
        try {
            Method[] methods = c.getMethods();
            for (int i = 0; i < methods.length; i++) {
                Method m = methods[i];
                if (m.getName().equals(name)) {
                    Main.debug("Candidate: " + m);
                    Class [] parameterTypes = m.getParameterTypes();
                    if (argcls == null) {
                        if (parameterTypes.length == 0) {
                           return m;
                        } 
                    } else {
                        if (argcls.length == parameterTypes.length) {
                          for (int j = 0; j < argcls.length; j++) {
                            // Main.debug("Parameter " + j + " " + parameterTypes[j]);
                            argcls[j] = parameterTypes[j];
                          }
                          return m;                        
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return null;
    }

    /**
    * converts Object <b>arg</b> into an object of class <b>cl</b>.
    * @param arg Object to convert
    * @param cl Destination class
    * @return An Object of the specified class with the value specified
    *  in <b>arg</b>
    */
    private static final Object cast(Object arg, Class cl) throws NumberFormatException {
        Object ret = arg;
        if (arg == null) {
            ret = null;
        }
        else if (cl.isAssignableFrom(arg.getClass())) {
            return arg;
        }
        else if (arg instanceof String) {
            String s = (String)arg;
            Main.debug("Argument String: \"" + s + "\"");
            if (cl == Boolean.TYPE || cl == Boolean.class) {
                ret = new Boolean(s);
            } else if (cl == Integer.TYPE || cl == Integer.class) {
                ret = new Integer(s);
            } else if (cl == Long.TYPE || cl == Long.class) {
                ret = new Long(s);
            } else if (cl == Float.TYPE || cl == Float.class) {
                ret = new Float(s);
            } else if (cl == Double.TYPE || cl == Double.class) {
                ret = new Double(s);
            } else if (cl == Short.TYPE || cl == Short.class) {
                ret = new Short(s);
            } else if (cl  == Byte.TYPE || cl == Byte.class) {
                ret = new Byte(s);
            } else if (cl == Character.TYPE || cl == Character.class) {
                ret = new Character(s.charAt(0));
            }
        }
        return ret;
    }
    
    public int[] callMember(String appletID, int objid, String name, StringBuffer value, java.util.List args)
    {
        Object o = null;
        KJASAppletStub stub = null;
        if (objid != 0)
            o = jsReferencedObjects.get(new Integer(objid));
        else {
            stub = (KJASAppletStub) stubs.get( appletID );
            if (stub != null)
                o = ((KJASAppletStub) stubs.get( appletID )).getApplet();
        }

        int [] ret = { JError, objid };
        if (o == null || (stub != null && !stub.isLoaded()))
            return ret;

        try {
            Main.debug("callMember: " + name);
            Object obj;
            Class c = o.getClass();
            String type;
            Class [] argcls = new Class[args.size()];
            for (int i = 0; i < args.size(); i++)
                argcls[i] = name.getClass(); // String for now, will be updated by findMethod
            Method m = findMethod(c, (String) name, argcls);
            Main.debug("Found Method: " + m);
            if (m != null) {
                Object [] argobj = new Object[args.size()];
                for (int i = 0; i < args.size(); i++) {
                    argobj[i] = cast(args.get(i), argcls[i]);
                }
                Object retval =  m.invoke(o, argobj);
                if (retval == null)
                    ret[0] = JVoid;
                else
                    ret = getJSTypeValue(retval, objid, value);
             }
        } catch (Exception e) {
            Main.debug("callMember threw exception: " + e.toString());
            e.printStackTrace();
        }
        return ret;
    }
    public void derefObject(int objid) {
        if (objid == 0)
            return; // that's an applet
        if (jsReferencedObjects.remove(new Integer(objid)) == null)
            Main.debug("couldn't remove referenced object");
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
