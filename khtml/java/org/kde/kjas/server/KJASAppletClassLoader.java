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
 * Java 1.1 is not supported anymore. 
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.11  2000/11/16 00:32:44  wynnw
 * added a main function to test class loading from a certain url
 *
 * Revision 1.10  2000/11/15 19:54:48  wynnw
 * This update:
 * * Updates the parsing to handle the new KJAS protocol
 * * changes the classloading to use the URLClassLoader
 * * encapsulates callbacks in the KJASProtocolHandler class
 * * adds more debug functionality
 * * fixed the callbacks to use the original PrintStream of stdout
 *
 * Revision 1.9  2000/09/16 03:09:20  rogozin
 * Fix #9558 and probably #9061
 *
 * Revision 1.8  2000/08/31 00:12:52  rogozin
 * Patch for loading applets referenced by full package name applied.
 * Author: Wim van Velthoven (W.vanVelthoven@fi.uu.nl)
 *
 * Revision 1.7  2000/03/22 05:19:38  rogozin
 *
 * Window geometry is now handled correctly.
 *
 * Revision 1.6  2000/02/13 23:05:36  rich
 * Fixed the problem with the lake testcase
 *
 * Revision 1.5  2000/01/29 04:22:28  rogozin
 * Preliminary support for archive tag.
 * Fix size problem.
 *
 * Revision 1.4  1999/12/14 19:57:00  rich
 * Many fixes, see changelog
 *
 * Revision 1.3  1999/11/12 02:58:04  rich
 * Updated KJAS server
 *
 * Revision 1.2  1999/11/12 01:22:36  rich
 * Now trys adding a / to the code base if the class loader could not find the applet class file. Fixed applet start/stop
 *
 * Revision 1.1.1.1  1999/07/22 17:28:08  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
public class KJASAppletClassLoader
   extends URLClassLoader
{
    protected URL baseURL;

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
            addURL( url );
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
