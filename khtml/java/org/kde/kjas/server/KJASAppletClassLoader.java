package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;

/**
 * ClassLoader used to download and instantiate Applets.
 * <P>
 * <FONT COLOR="red">Warning: No security implemented - do not use unless
 * you *really* know what you're doing.</FONT> In addition to the total
 * lack of security there are also other problems (like the fact the cache
 * of loaded classes is never emptied). It should work ok with both Java 1.1
 * and Java 2.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
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
   extends ClassLoader
{
   //* The base URL from which code will be loaded.
   URL codeBase;

   //* Cache the classes to prevent excessive network use.
   Hashtable classCache;

   public KJASAppletClassLoader( URL codeBase )
   {
      this.codeBase = codeBase;
      this.classCache = new Hashtable();
   }

    /**
     * This is used by Java 2.
     */
   public Class findClass( String name )
   {
      try {
         byte[] b = loadClassData(name);
         return defineClass(name, b, 0, b.length);
      }
      catch ( IOException e ) {
         System.err.println( "Error: " + e );
      }
      return null;
   }

    public synchronized Class loadClass(String name,
					boolean resolve)
    {
       try {
          Class c = (Class) classCache.get( name );

          if ( c == null ) {
             try {
                c = findSystemClass( name );
             }
             catch ( ClassNotFoundException cnfe ) {
                // Do nothing
             }
             catch ( NoClassDefFoundError ncdfe ) {
                // Do nothing
             }
          }
          
          if ( c == null ) {
             byte data[] = loadClassData( name );
             c = defineClass( data, 0, data.length );
             classCache.put( name, c );
          }
	  else {
	      try {
		  // TODO: Remove the shit hack
		  Thread.sleep( 1000 );
	      }
	      catch ( InterruptedException ie ) {
		  // Do nothing
	      }
	  }

          if ( resolve )
             resolveClass( c );

          return c;
       }
       catch ( IOException ioe ) {
          System.err.println( "Error: " + ioe );
       }
       return null;
    }


   private byte[] loadClassData( String name )
      throws IOException
   {
       if ( !name.endsWith( ".class" ) )
	   name += ".class";
      URL classURL = new URL( codeBase, name );

      System.err.println( "class data URL = " + classURL );

      InputStream dataStream;
      try {
	  dataStream = classURL.openStream();
      }
      catch( FileNotFoundException fnfe ) {
	  System.err.println( "Caught FileNotFoundException: " + fnfe );
	  String baseTmp = codeBase.toString();
	  baseTmp += "/";
	  URL baseURL = new URL( baseTmp );
	  classURL = new URL( baseURL, name );

	  System.err.println( "Retrying with " + classURL );

	  dataStream = classURL.openStream();
      }

      byte[] dataBytes = new byte[ 128 * 1024 ]; // Hard coded max of 128K classfile
      int count = 0;

      boolean done = false;
      while ( !done ) {
         int dataByte = dataStream.read();
         
         // If there was some data
         if ( dataByte != -1 ) {
            dataBytes[ count ] = (byte) dataByte;
            count++;
         }
         else {
            done = true;
         }
      }

      byte[] classBytes = new byte[ count ];
      System.arraycopy( dataBytes, 0,
                        classBytes, 0, count );

      dataBytes = null; // Help the GC

      return classBytes;
   }
}

