package org.kde.kjas.server;

import java.io.*;

/**
 *  KJAS server recognizes these variablers:
 *    kjas.debug - makes server actions verbose
 *    kjas.showConsole - shows Java Console window
 */

public class Main
{
    public static final boolean debug;

    //We need to save a reference to the original stdout
    //for sending messages back
    public static final PrintStream protocol_stdout;

    public static KJASProtocolHandler protocol;
    public static KJASConsole console;

    static
    {
        debug = System.getProperty("kjas.debug") != null;
        protocol_stdout = System.out;

        console = new KJASConsole();
    }

    public static void kjas_debug( String msg )
    {
        if( debug )
        {
            System.out.println( "KJAS: " + msg );
        }
    }

    public static void kjas_err( String msg, Exception e )
    {
        System.err.println( msg );
        e.printStackTrace();
    }

    public static void kjas_err( String msg, Throwable t )
    {
        System.err.println( msg );
        t.printStackTrace();
    }

    public static void main( String[] args )
    {
        // Check for Java version. We do not support Java 1.1
        boolean bad_jdk = checkForJavaVersion();

        if( bad_jdk || System.getProperty("kjas.showConsole") != null )
            console.show();
	
        if( bad_jdk )
        {
            System.err.println("\nERROR: This version of Java is not supported for security reasons.\nERROR: Please use Java version 1.2 or higher.");
            return;
        }

        KJASAppletRunner runner = new KJASAppletRunner();
        protocol = new KJASProtocolHandler( System.in, protocol_stdout,
                                            runner, "friend" );

        while( true )
        {
            try
            {
                protocol.commandLoop();
            }
            catch ( IOException e )
            {
                kjas_err( "IO Error: " + e, e );
            }
            catch ( Throwable t )
            {
                kjas_err( "Serious error: " + t, t );
            }
        }
    }

    private static boolean checkForJavaVersion()
    {
        String version  = System.getProperty("java.version");;
        return version.startsWith("1.1");	
    }
}
