package org.kde.kjas.server;

import java.io.*;

/**
 *  KJAS server recognizes these variablers:
 *    kjas.debug - makes server actions verbose 
 *    kjas.showConsole - shows Java Console window
 */

public class Main {
   public static final boolean debug;
   public static final PrintStream stdout;
   static {
       debug = System.getProperty("kjas.debug") != null; 
       stdout = System.out;
   }
    
   public static void main( String[] args )
   {
       int retries = 0;
       int LIMIT = 5;

       if(System.getProperty("kjas.showConsole") != null)
           (new KJASConsole()).show();

       if(debug)
           System.getProperty("KJAS: Starting KJAS in debug mode");

       KJASAppletRunner runner = new KJASAppletRunner();
       KJASProtocolHandler handler = new KJASProtocolHandler( System.in, System.out,
							      runner, "friend" );
       while( retries < LIMIT ) {
	   try {
	       if(debug)
                   System.out.println( "KJAS: Entering commandLoop()" );
               handler.commandLoop();
	   }
	   catch ( Exception e ) {
	       System.err.println( "KJAS: Error: " + e );
	       e.printStackTrace();
	   }
	   catch ( Throwable t ) {
	       System.err.println( "KJAS: Serious error: " + t );
	       t.printStackTrace();
	   }
	   retries++;
       }
   }
}
