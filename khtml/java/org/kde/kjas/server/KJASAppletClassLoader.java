package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;
import java.security.*;

/**
 * ClassLoader used to download and instantiate Applets.
 * <P>
 * NOTE: The class loader extends Java 1.2 specific class.
 */
public class KJASAppletClassLoader
    extends URLClassLoader
{
    private static Hashtable loaders = new Hashtable();
    public static KJASAppletClassLoader getLoader( String docBase, String codeBase )
    {
        String key = docBase + codeBase;
        Main.debug( "getLoader: key = " + key );
        KJASAppletClassLoader loader = (KJASAppletClassLoader) loaders.get( key );
        if( loader == null )
        {
            loader = new KJASAppletClassLoader( docBase, codeBase );
            loaders.put( key, loader );
        }
        else
        {
            Main.debug( "reusing classloader" );
        }

        return loader;
    }

    public static KJASAppletClassLoader getLoader( String key )
    {
        if( loaders.containsKey( key ) )
        {
            return (KJASAppletClassLoader) loaders.get( key );
        }
        else
        {
            return null;
        }
    }

    private URL docBaseURL  = null;
    private URL codeBaseURL = null;
    private Vector archives = null;
    private String key;
    public KJASAppletClassLoader( String docBase, String codeBase )
    {
        super( new URL[0] );
        key = docBase + codeBase;
        archives = new Vector();
        Main.debug( "Creating classloader with docBase = " + docBase +
                    " and codeBase = " + codeBase );

        try
        {
            //first determine what the real codeBase is: 3 cases
            //#1. codeBase is absolute URL- use that
            //#2. codeBase is relative to docBase, create url from those
            //#3. last resort, use docBase as the codeBase
            try
            {
                docBaseURL = new URL( docBase );
            }
            catch ( MalformedURLException mue )
            {
            	Main.kjas_err( "CL: Invalid Document Base = " + docBase, mue );
                return;
            }

            if(codeBase != null)
            {
                //we need to do this since codeBase should be a directory
                //and URLclassLoader assumes anything without a / on the end
                //is a jar file
                if( !codeBase.endsWith("/") )
                    codeBase = codeBase + "/";

                try
                {
                    codeBaseURL = new URL( codeBase );
                    Main.debug( "CL: codeBaseURL is codeBase" );
                } catch( MalformedURLException mue )
                {
                    try
                    {
                        codeBaseURL = new URL( docBaseURL, codeBase );
                        Main.debug( "CL: codeBaseURL is docBaseURL + codeBase" );
                    } catch( MalformedURLException mue2 ) {}
                }
            }

            if(codeBaseURL == null)
            {
                // codeBaseURL can not be null. This should not happen,
                // but if it does we fall back to document base
                // we do need to make sure that the docBaseURL is fixed if
                // it is something like http://www.foo.com/foo.asp
                // It's got to be a directory.....
                String file = docBaseURL.getFile();
                if( file == null )
                {
                	Main.debug( "CL: codeBaseURL = docBaseURL with no modifications, no file part of URL" );
                    codeBaseURL = docBaseURL;
                }
                else
                if( file.length() == 0 )
                {
	                Main.debug( "CL: codeBaseURL = docBaseURL with no modifications, no file part of URL" );
                    codeBaseURL = docBaseURL;
                }
                else
                if( file.endsWith( "/" ) )
                {
                	Main.debug( "CL: codeBaseURL = docBaseURL, with no modifications, file part ends with /" );
                    codeBaseURL = docBaseURL;
                }
                else
                {
                	Main.debug( "CL: codeBaseURL = docBaseURL with modifications, deleting up to last /" );

                    //delete up to the ending '/'
                    String urlString = docBaseURL.toString();
                    int dot_index = urlString.lastIndexOf( '/' );
                    String newfile = urlString.substring( 0, dot_index+1 );
                    codeBaseURL = new URL( newfile );
                }
            }

            Main.debug( "CL: Finally, codeBaseURL = " + codeBaseURL );
        }catch( Exception e )
        {
            Main.debug( "KJASAppletClassLoader caught an exception: " + e );
            e.printStackTrace();
        }
    }

    public void paramsDone()
    {
        super.addURL( codeBaseURL );
    }

    public String getKey()
    {
        return key;
    }

    public void addJar( String jarname )
    {
        if( archives.contains( jarname ) )
            return;
        else
            archives.add( jarname );

        try
        {
            URL newurl = new URL( codeBaseURL, jarname );
            addURL( newurl );
        }
        catch ( MalformedURLException e )
        {
            Main.kjas_err( "bad url creation: " + e, e );
        }
    }

    public void addResource( String url, byte[] data )
    {
        Main.debug( "addResource for url: " + url );
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
     * Class Loading Methods
     **************************************************************************/
    public Class loadClass( String name )
    {
    	if( Main.Debug )
    	{
    		Main.debug( "CL: URL's to search are: " );
    		URL[] urls = getURLs();
    		for( int i = 0; i < urls.length; i++ )
    		{
    			Main.debug( "CL:    " + urls[i] );
    		}
    	}

        //We need to be able to handle foo.class, so strip off the suffix
        if( name.endsWith( ".class" ) )
        {
            name = name.substring( 0, name.lastIndexOf( ".class" ) );
        }

        //try to load it with the parent first...
        try
        {
            return super.loadClass( name );
        }
        catch( ClassNotFoundException e )
        {
            Main.kjas_err( "Couldn't load class: " + name, e );
            return null;
        }
        catch( ClassFormatError e )
        {
            Main.kjas_err( "Class format error for " + name, e );
            return null;
        }
    }

    /***************************************************************************
     * Security Manager stuff
     **************************************************************************/
    protected PermissionCollection getPermissions( CodeSource cs )
    {
        //get the permissions from the SecureClassLoader
        final PermissionCollection perms = super.getPermissions( cs );
        final URL url = cs.getLocation();

        //first add permission to connect back to originating host
        perms.add(new SocketPermission(url.getHost(), "connect,accept"));

        //add ability to read from it's own directory...
        if ( url.getProtocol().equals("file") )
        {
            String path = url.getFile().replace('/', File.separatorChar);

            if (!path.endsWith(File.separator))
            {
                int endIndex = path.lastIndexOf(File.separatorChar);
                if (endIndex != -1)
                {
                    path = path.substring(0, endIndex+1) + "-";
                    perms.add(new FilePermission(path, "read"));
                }
            }

            AccessController.doPrivileged(
                new PrivilegedAction()
                {
                    public Object run()
                    {
                        try
                        {
                            if (InetAddress.getLocalHost().equals(InetAddress.getByName(url.getHost())))
                            {
                                perms.add(new SocketPermission("localhost", "connect,accept"));
                            }
                        } catch (UnknownHostException uhe)
                        {}
                        return null;
                    }
                }
            );
        }

        //permissions for thread access??

        return perms;
    }

}
