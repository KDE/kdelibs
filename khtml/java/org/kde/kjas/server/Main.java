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
    private static boolean show_console;

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

    public static void showConsole()
    {
        if( show_console )
            console.setVisible( true );
    }

    public static void hideConsole()
    {
        if( show_console );
            console.setVisible( false );
    }

    private static boolean checkForJavaVersion()
    {
        String version  = System.getProperty("java.version");;
        return version.startsWith("1.1");	
    }

    public static void main( String[] args )
    {
        // Check for Java version. We do not support Java 1.1
        boolean bad_jdk = checkForJavaVersion();

        if( bad_jdk || System.getProperty("kjas.showConsole") != null )
            console.setVisible( true );
	
        if( bad_jdk )
        {
            System.err.println( "ERROR: This version of Java is not supported for security reasons." );
            System.err.println( "       Please use Java version 1.2 or higher." );
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


}
