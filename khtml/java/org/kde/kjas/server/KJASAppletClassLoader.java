package org.kde.kjas.server;

import java.net.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;

/**
 * ClassLoader used to download and instantiate Applets.
 * <P>
 * <FONT COLOR="red">Warning: No security implemented - do not use unless
 * you *really* know what you're doing.</FONT> 
 * It should work ok with both Java 1.1 and Java 2.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
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
   extends ClassLoader
{
   //* The base URL from which code will be loaded.
   URL codeBase;

   //* Raw class data parsed from Jars. Contains all other resources as well.
   Hashtable rawData;

   public KJASAppletClassLoader( URL codeBase )
   {
      this.codeBase = codeBase;
      this.rawData = new Hashtable();
   }
   /**
    * Loads Jar and Zip archives from the server
    */
   public void loadJars( String jars ) 
   {
      StringTokenizer parser = new StringTokenizer(jars, ",", false);
      while(parser.hasMoreTokens()) {
	 String jar = parser.nextToken().trim();
	 if(Main.debug) 
	    System.out.println("CL: Loading archive: " + jar);
         ZipInputStream zip = null;
         try {
            zip = new ZipInputStream((new URL( codeBase, jar )).openStream());
            
            // For every zip entry put it data to the hash table
	    ZipEntry entry;
            while((entry = zip.getNextEntry()) != null) {

               // Skip directories
	       if(entry.isDirectory())
		  continue; 

	       if(Main.debug) 
		  System.out.println("CL: Loading entry: " + entry.getName());

               // If we know the total length of the entry in advance 
               // allocate the exact array. Otherwise do it bu chunks
               // and reallocated if needed
	       int n, total = 0;
	       int len = (int)entry.getSize();
	       byte data[] = new byte[(len == -1) ? 2024 : len];
	       while((n = zip.read(data, total, data.length - total)) >= 0) {
		  if((total += n) == data.length) {
		     if(len < 0) {
			byte newdata[] = new byte[total + 2024];
			System.arraycopy(data, 0, newdata, 0, total);
			data = newdata;
		     }
		     else
			break;
		  }
	       }

	       // Store the raw data
	       rawData.put(entry.getName(), data);
	    }
	 }
	 catch(Exception e) {
	    System.out.println("Can not load archive " + e);
	 }
         finally {
            try {
               if(zip != null) zip.close();
            }
            catch(Exception e) {}
         }
      }
   }

   public synchronized Class loadClass(String name, boolean resolve)
      throws ClassNotFoundException
   {
      Class c = findClass(name);
      if ( c == null ) 
         throw new ClassNotFoundException(name);
      
      if ( resolve )
         resolveClass( c );
      
      return c;
   }

   public InputStream getResourceAsStream(String name) 
   {
      InputStream inputstream = ClassLoader.getSystemResourceAsStream(name);
      if(inputstream != null)
         return inputstream;
      
      byte data[] = (byte[]) rawData.get(name);
      if(data != null)
         return new ByteArrayInputStream(data);
      
      return null;
   }
   
   /**
    *  General class load function
    */
   public Class findClass(String name)
   {
      Class c;
      
      // 1. Try loaded classes
      c = findLoadedClass(name);
      if(c != null)
         return c;
      
      // 2. Try system (CLASSPATH) classes
      try {
         c = findSystemClass( name );
         if(c != null)
            return c;
      }
      catch (ClassNotFoundException e) {}

      // 3. Try classes from archives
      c = findJarClass( name );
      if(c != null)
         return c;

      // 4. Try classes from Web server
      c = findURLClass(name);
      if(c != null)
         return c;
      
      // Opps!
      return null;
   }  

   /**
    *  Load class from jar table
    */
   Class findJarClass( String name )
   {
      if(rawData.isEmpty())
         return null;
      
      // Convert name and see if we have such a beast
      if ( name.endsWith( ".class" ) )
	  name = name.substring( 0, name.indexOf( ".class" ) );
      String cname = name.replace('.', '/') + ".class";
	       
      if(Main.debug) {
	  System.out.println("CL: findJarClass: name  = " + name);
	  System.out.println("CL: findJarClass: cname = " + cname);
      }

      byte data[] = (byte[]) rawData.get(cname);
      
      if(data != null) {
         // If we found one remove it from the table to save some space
         // and load it into JVM
         rawData.remove(cname);
         return defineClass(name, data, 0, data.length);
      }
      
      return null;
   }
   
   /** 
    *  Load class from Web 
    */ 
   Class findURLClass( String name )
   {
      // name - class name used for class initialization
      // cname - converted name used for class retrival vie URL
 
      if ( name.endsWith( ".class" ) )
	  name = name.substring( 0, name.indexOf( ".class" ) );
      String cname = name.replace('.','/') + ".class";

      if(Main.debug) {
         System.out.println( "CL: findURLClass: name  = " + name );
         System.out.println( "CL: findURLClass: cname = " + cname );
      }

      InputStream in = null;
      
      try {
         URL classURL = new URL( codeBase, cname );

         URLConnection connection = classURL.openConnection();
         int len = connection.getContentLength();
         int n, total = 0;
         byte data[] = new byte[len != -1 ? len : 2048];
         in = connection.getInputStream();

         while((n = in.read(data, total, data.length - total)) >= 0) {
            if((total += n) == data.length) {
               if(len < 0) {
                  byte new_data[] = new byte[total + 2024];
                  System.arraycopy(data, 0, new_data, 0, total);
                  data = new_data;
               }
               else
                  break;
            }
         }
         return defineClass(name, data, 0, total);
      }
      catch(Exception e) 
         { }
      finally {
         try {
            if(in != null) in.close();
         } 
         catch(Exception e) {}
      }
      
      return null;
   }
}
