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
    private URL docBaseURL  = null;
    private URL codeBaseURL = null;

    public KJASAppletClassLoader( String docBase, String codeBase )
    {
        super( new URL[0] );

        Main.kjas_debug( "Creating classloader with docBase = " + docBase +
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
                Main.kjas_debug( "Could not create URL from docBase, not creating applet" );
                return;
            }

            if(codeBase != null)
            {
                Main.kjas_debug( "codeBase not null, trying to create URL from it" );
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
                        Main.kjas_debug( "could not create URL from codeBase alone" );
                        codeBaseURL = new URL( docBaseURL, codeBase );
                    } catch( MalformedURLException mue2 )
                    {
                        Main.kjas_debug( "could not create URL from docBaseURL and codeBase" );
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
                Main.kjas_debug( "codeBaseURL still null, defaulting to docBase" );
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

            Main.kjas_debug( "codeBaseURL = " + codeBaseURL );
            super.addURL( codeBaseURL );
        }catch( Exception e )
        {
        }

    }

    public void addJar( String jarname )
    {
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
            Main.kjas_debug( "super couldn't load class: " + name + ", exception = " + e );
            throw e;
        }
        catch( ClassFormatError e )
        {
            Main.kjas_debug( "Class format error: " + e );
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
            Main.kjas_debug( "could not find the class: " + name + ", exception = " + e );
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
