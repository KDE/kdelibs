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
    public static final PrintStream stdout;

    public static KJASProtocolHandler protocol;

    static
    {
        debug = System.getProperty("kjas.debug") != null;
        stdout = System.out;
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
        if( System.getProperty("kjas.showConsole") != null )
            (new KJASConsole()).show();

        KJASAppletRunner runner = new KJASAppletRunner();
        protocol = new KJASProtocolHandler( System.in, stdout,
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
