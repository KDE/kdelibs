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

    public static void removeLoader( KJASAppletClassLoader loader )
    {
        loaders.remove( loader.getKey() );
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
                Main.debug( "Could not create URL from docBase, not creating applet" );
                return;
            }

            if(codeBase != null)
            {
                Main.debug( "codeBase not null, trying to create URL from it" );
                //we need to do this since codeBase should be a directory
                //and URLclassLoader assumes anything without a / on the end
                //is a jar file
                if( !codeBase.endsWith("/") )
                    codeBase = codeBase + "/";

                try
                {
                    codeBaseURL = new URL( codeBase );
                } catch( MalformedURLException mue )
                {
                    try
                    {
                        Main.debug( "could not create URL from codeBase alone" );
                        codeBaseURL = new URL( docBaseURL, codeBase );
                    } catch( MalformedURLException mue2 )
                    {
                        Main.debug( "could not create URL from docBaseURL and codeBase" );
                    }
                }
            }

            if(codeBaseURL == null)
            {
                // codeBaseURL can not be null. This should not happen,
                // but if it does we fall back to document base
                // we do need to make sure that the docBaseURL is fixed if
                // it is something like http://www.foo.com/foo.asp
                // It's got to be a directory.....
                Main.debug( "codeBaseURL still null, defaulting to docBase" );
                String file = docBaseURL.getFile();
                if( file == null )
                    codeBaseURL = docBaseURL;
                else
                if( file.endsWith( "/" ) )
                    codeBaseURL = docBaseURL;
                else
                {
                    //delete up to the ending '/'
                    int dot_index = file.lastIndexOf( '/' );
                    String newfile = file.substring( 0, dot_index+1 );
                    codeBaseURL = new URL( docBaseURL.getProtocol(),
                                           docBaseURL.getHost(),
                                           newfile );
                }
            }

            Main.debug( "codeBaseURL = " + codeBaseURL );
        }catch( Exception e )
        {
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
        throws ClassNotFoundException
    {
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
            Main.debug( "super couldn't load class: " + name + ", exception = " + e );
            throw e;
        }
        catch( ClassFormatError e )
        {
            Main.debug( "Class format error: " + e );
            return null;
        }
    }

    public Class findClass(String name)
        throws ClassNotFoundException
    {
        if( name.endsWith( ".class" ) )
        {
            name = name.substring( 0, name.lastIndexOf( ".class" ) );
        }

        try
        {
            return super.findClass( name );
        }
        catch( ClassNotFoundException e )
        {
            Main.debug( "could not find the class: " + name + ", exception = " + e );
            throw e;
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

    public static void main( String[] args )
    {
        System.out.println( "num args = " + args.length );
        System.out.println( "args[0] = " + args[0] );
        System.out.println( "args[1] = " + args[1] );

        try
        {
            URL location = new URL( args[0] );
            if( location.getFile() == null )
            {
                System.out.println( "getFile returned null" );
            }

            KJASAppletClassLoader loader = new KJASAppletClassLoader( args[1], args[1] );
            Class foo = loader.loadClass( args[1] );

            System.out.println( "loaded class: " + foo );
        }
            catch( Exception e )
        {
            System.out.println( "Couldn't load class " + args[1] );
            e.printStackTrace();
        }
    }
}
