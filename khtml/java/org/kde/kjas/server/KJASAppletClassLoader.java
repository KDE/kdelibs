package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.util.jar.*;
import java.security.*;

/**
 * ClassLoader used to download and instantiate Applets.
 * <P>
 * NOTE: The class loader extends Java 1.2 specific class.
 */
public final class KJASAppletClassLoader
    extends URLClassLoader
{
    private static Hashtable loaders = new Hashtable();
    public static synchronized KJASAppletClassLoader getLoader( String docBase, String codeBase )
    {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkCreateClassLoader();
        }
        URL docBaseURL;
        KJASAppletClassLoader loader = null;
        try
        {
            docBaseURL = new URL( docBase );
        
            URL codeBaseURL = getCodeBaseURL( docBaseURL, codeBase );
            Main.debug( "CL: getLoader: key = " + codeBaseURL );

            loader = (KJASAppletClassLoader) loaders.get( codeBaseURL.toString() );
            if( loader == null )
            {
                URL [] urlList = {};
                loader = new KJASAppletClassLoader( urlList, docBaseURL, codeBaseURL);
                loaders.put( codeBaseURL.toString(), loader );
            }
            else
            {
                Main.debug( "CL: reusing classloader" );
            }
        } catch( MalformedURLException e ) { Main.kjas_err( "bad DocBase URL", e ); }
        return loader;
    }

    public static URL getCodeBaseURL( URL docBaseURL, String codeBase )
    {
        URL codeBaseURL = null;
        try
        {
            //first determine what the real codeBase is: 3 cases
            //#1. codeBase is absolute URL- use that
            //#2. codeBase is relative to docBase, create url from those
            //#3. last resort, use docBase as the codeBase
            if(codeBase != null)
            {
                //we need to do this since codeBase should be a directory
                if( !codeBase.endsWith("/") )
                    codeBase = codeBase + "/";

                try
                {
                    codeBaseURL = new URL( codeBase );
                } catch( MalformedURLException mue )
                {
                    try
                    {
                        codeBaseURL = new URL( docBaseURL, codeBase );
                    } catch( MalformedURLException mue2 ) {}
                }
            }

            if(codeBaseURL == null)
            {
                //fall back to docBase but fix it up...
                String file = docBaseURL.getFile();
                if( file == null || (file.length() == 0)  )
                    codeBaseURL = docBaseURL;
                else if( file.endsWith( "/" ) )
                    codeBaseURL = docBaseURL;
                else
                {
                    //delete up to the ending '/'
                    String urlString = docBaseURL.toString();
                    int dot_index = urlString.lastIndexOf( '/' );
                    String newfile = urlString.substring( 0, dot_index+1 );
                    codeBaseURL = new URL( newfile );
                }
            }
        }catch( Exception e ) { Main.kjas_err( "CL: exception ", e ); }
        return codeBaseURL;    
    }

    public static KJASAppletClassLoader getLoader( String key )
    {
        SecurityManager security = System.getSecurityManager();
        if (security != null) {
            security.checkCreateClassLoader();
        }
        if( loaders.containsKey( key ) )
            return (KJASAppletClassLoader) loaders.get( key );
        
        return null;
    }

    /*********************************************************************************
     ****************** KJASAppletClassLoader Implementation *************************
     **********************************************************************************/
    private URL docBaseURL;
    private URL codeBaseURL;
    private Vector archives;
    private String dbgID;
    private static int globalId = 0;
    private int myId = 0;
    private KJASAppletContext appletContext = null;
    
    public KJASAppletClassLoader( URL[] urlList, URL _docBaseURL, URL _codeBaseURL)
    {
        super(urlList);
        synchronized(KJASAppletClassLoader.class) {
            myId = ++globalId;
        }
        docBaseURL   = _docBaseURL;
        codeBaseURL  = _codeBaseURL;
        archives     = new Vector();
        
        appletContext   = null;
        dbgID = "CL-" + myId + "(" + codeBaseURL.toString() + "): ";
    }
    
    protected void addURL(URL url) {
        Main.debug(this + " add URL: " + url);
        super.addURL(url);
    }
    
    public void setAppletContext(KJASAppletContext context) {
        appletContext = context;
    }
    
    public void paramsDone() {
        // simply builds up the search path
        // put the archives first because they are 
        // cached.
        for( int i = 0; i < archives.size(); ++i ) {
            String jar = (String)archives.elementAt( i );
            try {
                URL jarURL = new URL(codeBaseURL, jar);
                addURL(jarURL);
                Main.debug("added archive URL \"" + jarURL + "\" to KJASAppletClassLoader");
            } catch (MalformedURLException e) {
                Main.kjas_err("Could not construct URL for jar file: " + codeBaseURL + " + " + jar, e);
            }
        }
        // finally add code base url and docbase url
        addURL(codeBaseURL);
        
        // the docBaseURL has to be fixed.
        // strip file part from end otherwise this
        // will be interpreted as an archive
        // (should this perhaps be done generally ??)       
        String dbs = docBaseURL.toString();
        int idx = dbs.lastIndexOf("/");
        if (idx > 0) {
            dbs = dbs.substring(0, idx+1);
        }
        URL docDirURL = null; 
        try {
            docDirURL = new URL(dbs);
        } catch (MalformedURLException e) {
            Main.debug("Could not make a new URL from docBaseURL=" + docBaseURL);
        }
        if (docDirURL != null && !codeBaseURL.equals(docDirURL)) {
            addURL(docDirURL);
        }
    }

    void addArchiveName( String jarname )
    {
        if( !archives.contains( jarname ) )
        {
            archives.add( jarname );
        }
    }
    

    public URL getDocBase()
    {
        return docBaseURL;
    }

    public URL getCodeBase()
    {
        return codeBaseURL;
    }

    /***************************************************************************
     **** Class Loading Methods
     **************************************************************************/
    public synchronized Class findClass( String name ) throws ClassNotFoundException
    {
        Class rval = null;
        try
        {
            //check for a system class
            rval = findSystemClass( name );
        } catch (ClassNotFoundException e )
        {
            if (appletContext != null) {
                String displayName = name;
                // we lie a bit and do not display the inner
                // classes because their names are ugly
                int idx = name.indexOf('$');
                if (idx > 0) {
                    displayName = name.substring(0, idx);
                }
                appletContext.showStatus("Loading: " + displayName + " Java Class ...");
            }
            //check the loaded classes 
            rval = findLoadedClass( name );
            if( rval == null ) {
                rval =  super.findClass(name);
            }
        }
        if (rval == null) {
            throw new ClassNotFoundException("Class:" + name);
        }
        return rval;
    }
    
    public synchronized Class loadClass( String name ) throws ClassNotFoundException
    {
        Main.debug( dbgID + "loadClass, class name = " + name );
        //We need to be able to handle foo.class, so strip off the suffix
        String fixed_name = name;
        if( name.endsWith( ".class" ) )
        {
            fixed_name = name.substring( 0, name.lastIndexOf( ".class" ) );
        }
        else if( name.endsWith( ".java" ) )
        {
            // be smart, some applets specify code=XyzClass.java
            fixed_name = name.substring( 0, name.lastIndexOf( ".java" ) );
        }
        Class cl = super.loadClass(fixed_name);
        Main.debug(dbgID + " returns class " + cl.getName());
        return cl;
    }

    public InputStream getResourceAsStream( String name )
    {
        Main.debug( dbgID + "getResourceAsStream, name = " + name );
        InputStream stream = super.getResourceAsStream(name);
        Main.debug("got stream " + stream);
        return stream;
    }
    
    public URL getResource( String name )
    {
        Main.debug( dbgID + "getResource, name = " + name );
        return super.getResource( name );
    }
    
    public URL findResource( String name)
    {
        Main.debug( dbgID + "findResource, name = " + name );
        if (appletContext != null) {
            appletContext.showStatus("Loading: " + name);
        }
        URL url =  super.findResource( name );
        Main.debug("findResource for " + name + " returns " + url);
        return url;
    }
   
    protected PermissionCollection getPermissions(CodeSource cs) {
        Main.debug(dbgID + " getPermissions(" + cs + ")");
        PermissionCollection permissions = super.getPermissions(cs);
        Enumeration enum = permissions.elements();
        while (enum.hasMoreElements()) {
            Main.debug(this + " Permission: " + enum.nextElement());
        }
        return permissions;
    }
    
}
