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
    public static KJASAppletClassLoader createLoader( URL url )
    {
        URL[] urls = new URL[1];
        urls[0] = url;

        return new KJASAppletClassLoader( urls );
    }

    public KJASAppletClassLoader( URL[] urls )
    {
        super( urls );
    }

    public void addJar( URL baseURL, String jarname )
    {
        try
        {
            URL newurl = new URL( baseURL, jarname );
            addURL( newurl );
        }
        catch ( MalformedURLException e )
        {
            Main.kjas_err( "bad url creation: " + e, e );
            throw new IllegalArgumentException( jarname );
        }
    }

    public Class loadClass( String name )
        throws ClassNotFoundException
    {
        //We need to be able to handle foo.class, so strip off the suffix
        if( name.endsWith( ".class" ) )
        {
            name = name.substring( 0, name.length() - 6 );
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
    }


    /**
     *  Emergency class loading function- the last resort
     */
    public Class findClass(String name)
        throws ClassNotFoundException
    {
        //All we need to worry about here are classes
        //with kde url's other than those handled by
        //the URLClassLoader

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

    public void addCodeBase( URL url )
    {
        URL[] urls = getURLs();

        boolean inthere = false;
        for( int i = 0; i < urls.length; i++ )
        {
            if( urls[i].equals( url ) )
            {
                inthere = true;
                break;
            }
        }

        if( !inthere )
        {
            Main.kjas_debug( "KJASAppletClassLoader::addCodeBase, just added: " + url );
            addURL( url );
        }
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

            KJASAppletClassLoader loader = KJASAppletClassLoader.createLoader( new URL(args[0]) );
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
