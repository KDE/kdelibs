package org.kde.kjas.server;

public class Main {
   public static void main( String[] args )
   {
       int retries = 0;
       int LIMIT = 5;

       KJASAppletRunner runner = new KJASAppletRunner();
       KJASProtocolHandler handler = new KJASProtocolHandler( System.in, System.out,
							      runner, "friend" );
       while( retries < LIMIT ) {
	   try {
	       System.err.println( "Entering commandLoop()" );
	       handler.commandLoop();
	   }
	   catch ( Exception e ) {
	       System.err.println( "Error: " + e );
	       e.printStackTrace();
	       //	 System.exit(1);
	   }
	   catch ( Throwable t ) {
	       System.err.println( "Serious error: " + t );
	       t.printStackTrace();
	       //	 System.exit(1);
	   }
	   retries++;
       }
   }
}
