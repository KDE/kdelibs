package org.kde.kjas.server;

import java.io.*;


/**
 *  KJAS server recognizes these variablers:
 *    kjas.debug - makes server actions verbose
 *    kjas.showConsole - shows Java Console window
 */

public class Main
{
    //We need to save a reference to the original stdout
    //for sending messages back
    public  static final PrintStream         protocol_stdout;
    public  static final KJASProtocolHandler protocol;
    public  static final KJASConsole         console;
    private static final boolean             show_console;
    public  static final boolean             debug;
    public  static final boolean             log;
    private static boolean                   good_jdk;

    /**************************************************************************
     * Initialization
     **************************************************************************/
    static
    {
        debug        = System.getProperty( "kjas.debug" ) != null;
        show_console = System.getProperty( "kjas.showConsole" ) != null;
        log          = System.getProperty( "kjas.log" ) != null;

        protocol_stdout = System.out;
        console         = new KJASConsole();
        protocol        = new KJASProtocolHandler( System.in, protocol_stdout );

        Main.kjas_debug( "JVM version = " + System.getProperty( "java.version" ) );
        String version = System.getProperty("java.version").substring( 0, 3 );
        Main.kjas_debug( "JVM numerical version = " + version );
        try
        {
            float java_version = Float.parseFloat( version );
            if( java_version >= 1.2 )
                good_jdk = true;
            else
                good_jdk = false;
        } catch( NumberFormatException e )
        {
            good_jdk = false;
        }
    }

    /**************************************************************************
     * Public Utility functions available to the KJAS framework
     **************************************************************************/
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


    /**************************************************************************
     * Main- create the command loop
     **************************************************************************/
    public static void main( String[] args )
    {
        if( !good_jdk )
        {
            console.setVisible( true );
            System.err.println( "ERROR: This version of Java is not supported for security reasons." );
            System.err.println( "       Please use Java version 1.2 or higher." );
            return;
        }

        protocol.commandLoop();
    }

}
