package org.kde.kjas.server;

public class Main {
   public static void main( String[] args )
   {
       try {
         KJASAppletRunner runner = new KJASAppletRunner();
         KJASProtocolHandler handler = new KJASProtocolHandler( System.in, System.out,
                                                                runner, "friend" );
         System.err.println( "Entering commandLoop()" );
         handler.commandLoop();
      }
      catch ( Exception e ) {
         System.err.println( "Error: " + e );
         e.printStackTrace();
	 System.exit(1);
      }
   }
}
