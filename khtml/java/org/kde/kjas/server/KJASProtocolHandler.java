package org.kde.kjas.server;

import java.io.*;
import java.util.*;
import java.awt.*;

/**
 * Encapsulates the KJAS protocol and manages the contexts
 *
 */
public class KJASProtocolHandler
{
    //Command codes
    private static final int CreateContextCode   = 1;
    private static final int DestroyContextCode  = 2;
    private static final int CreateAppletCode    = 3;
    private static final int DestroyAppletCode   = 4;
    private static final int StartAppletCode     = 5;
    private static final int StopAppletCode      = 6;
    private static final int InitAppletCode      = 7;
    private static final int ShutdownServerCode  = 9;
    private static final int ShowDocumentCode    = 12;
    private static final int ShowURLInFrameCode  = 13;
    private static final int ShowStatusCode      = 14;
    private static final int ResizeAppletCode    = 15;

    //Holds contexts in contextID-context pairs
    private Hashtable contexts;

    private PushbackInputStream commands;    //Stream for reading in commands
    private PrintStream                  signals;          //Stream for writing out callbacks

    //used for parsing each command as it comes in
    private int cmd_index;

    public KJASProtocolHandler( InputStream  _commands,
                                OutputStream _signals )
    {
        commands = new PushbackInputStream( _commands );
        signals = new PrintStream( _signals );
        contexts = new Hashtable();
    }

    public void commandLoop()
    {
        Main.kjas_debug( "Start commandLoop" );
        try
        {
            while( true )
            {
                try
                {
                    int cmd_length = readPaddedLength( 8 );

                    char[] cmd = new char[cmd_length];
                    for( int i = 0; i < cmd_length; i++ )
                    {
                        cmd[i] = (char) commands.read();
                    }

                    //parse the rest of the command and execute it
                    processCommand( cmd );
                }
                catch( NumberFormatException e )
                {
                    Main.kjas_err( "Could not parse out message length", e );
                    System.exit( 1 );
                }
                catch( Throwable t )
                {
                }
            }
        }
        catch( Exception i )
        {
            Main.kjas_err( "exception: ", i );
            System.exit( 1 );
        }
    }

    public void processCommand( char[] command )
        throws IllegalArgumentException
    {
        // Sanity checks
        if ( command == null )
           throw new IllegalArgumentException( "processCommand() received null" );

        //do all the parsing here and pass arguments as individual variables to the
        //handler functions
        cmd_index = 0;

        int cmd_code_value = (int) command[cmd_index++];
        if( cmd_code_value == CreateContextCode )
        {
            //parse out contextID- 1 argument
            String contextID = getArg( command );
            Main.kjas_debug( "createContext, id = " + contextID );

            KJASAppletContext context = new KJASAppletContext( contextID );
            contexts.put( contextID, context );
            Main.showConsole();
        } else
        if( cmd_code_value == DestroyContextCode )
        {
            //parse out contextID- 1 argument
            String contextID = getArg( command );
            Main.kjas_debug( "destroyContext, id = " + contextID );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if( contexts != null )
            {
                context.destroy();
                contexts.remove( contextID );
                if( contexts.size() == 0 )
                    Main.hideConsole();
            }
        } else
        if( cmd_code_value == CreateAppletCode )
        {
            //9 arguments- this order is important...
            String contextID = getArg( command );
            String appletID = getArg( command );
            String appletName = getArg( command );
            String className = getArg( command );
            String baseURL = getArg( command );
            String codeBase = getArg( command );
            String archives = getArg( command );

            String width = getArg( command );
            String height = getArg( command );
            String title = getArg( command );
            //get the number of parameter pairs...
            String str_params = getArg( command );
            int num_params = Integer.parseInt( str_params.trim() );
            Hashtable params = new Hashtable();
            for( int i = 0; i < num_params; i++ )
            {
                String name  = getArg( command );
                if( name == null )
                    name = new String();

                String value = getArg( command );
                if( value == null )
                    value = new String();
                Main.kjas_debug( "parameter, name = " + name + ", value = " + value );

                if( name.equalsIgnoreCase( "archive" ) && archives == null )
                    archives = value;
                else
                if( name.equalsIgnoreCase( "codebase" ) && codeBase == null )
                    codeBase = value;
                else
                    params.put( name, value );
            }

            Main.kjas_debug( "createApplet, context = " + contextID + ", applet = " + appletID );
            Main.kjas_debug( "              name = " + appletName + ", classname = " + className );
            Main.kjas_debug( "              baseURL = " + baseURL + ", codeBase = " + codeBase );
            Main.kjas_debug( "              archives = " + archives + ", width = " + width + ", height = " + height );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if( context != null )
                context.createApplet( appletID, appletName, className,
                                      baseURL, codeBase, archives,
                                      new Dimension( Integer.parseInt(width), Integer.parseInt(height) ),
                                      title, params );
        } else
        if( cmd_code_value == DestroyAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );
            Main.kjas_debug( "destroyApplet, context = " + contextID + ", applet = " + appletID );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if ( context != null )
                context.destroyApplet( appletID );
        } else
        if( cmd_code_value == StartAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );
            Main.kjas_debug( "startApplet, context = " + contextID + ", applet = " + appletID );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if ( context != null )
                context.startApplet( appletID );
        } else
        if( cmd_code_value == StopAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );
            Main.kjas_debug( "stopApplet, context = " + contextID + ", applet = " + appletID );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if ( context != null )
                context.stopApplet( appletID );
        } else
        if( cmd_code_value == InitAppletCode )
        {
            String contextID = getArg( command );
            String appletID = getArg( command );
            Main.kjas_debug( "InitApplet, context = " + contextID + ", applet = " + appletID );

            KJASAppletContext context = (KJASAppletContext) contexts.get( contextID );
            if ( context != null )
                context.initApplet( appletID );
        }
        else
        if( cmd_code_value == ShutdownServerCode )
        {
            Main.kjas_debug( "shutDownServer" );
            System.exit( 1 );
        }
        else
        {
           throw new IllegalArgumentException( "Unknown command code" );
        }
    }

    private String getArg( char[] command )
    {
        Vector arg_chars = new Vector();

        char curr = command[cmd_index++];
        while( 0 != (int) curr )
        {
            arg_chars.add( new Character(curr) );
            curr = command[cmd_index++];
        }

        if( arg_chars.size() > 0 )
        {
            char[] char_bytes = new char[arg_chars.size()];

            for( int i = 0; i < arg_chars.size(); i++ )
            {
                Character ch = (Character) arg_chars.elementAt( i );

                char_bytes[i] = ch.charValue();
            }
            return new String( char_bytes );
        }
        else
        {
            return null;
        }
    }

    public void sendShowDocumentCmd( String contextID, String url )
    {
        Main.kjas_debug( "sendShowDocumentCmd, contextID, url" );

        //figure out how long this will be, 4 extra for 2 seps, end, and code
        int length = contextID.length() + url.length() + 4;
        char[] chars = new char[ length + 8 ]; //8 for the length of this message
        int index = 0;
        char sep = (char) 0;
        char[] tmpchar = getPaddedLength( length );

        //fill in the length of the command
        for( int i = 0; i < 8; i++ )
        {
            chars[index++] = tmpchar[i];
        }

        //fill chars array to print it with the PrintStream
        chars[index++] = (char) ShowDocumentCode;
        chars[index++] = sep;

        tmpchar = contextID.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = url.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        signals.print( chars );
    }

    public void sendShowDocumentCmd( String contextID, String url, String frame)
    {
        Main.kjas_debug( "sendShowDocumentCmd, contextID, url, frame" );

        //length = length of args plus code, 3 seps, end
        int length = contextID.length() + url.length() + frame.length() + 5;
        char[] chars = new char[ length + 8 ]; //for length of message
        int index = 0;
        char sep = (char) 0;
        char[] tmpchar = getPaddedLength( length );

        //fill in the length of the command
        for( int i = 0; i < 8; i++ )
        {
            chars[index++] = tmpchar[i];
        }

        //fill chars array to print it with the PrintStream
        chars[index++] = (char) ShowURLInFrameCode;
        chars[index++] = sep;

        tmpchar = contextID.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = url.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = frame.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        signals.print( chars );
    }

    public void sendShowStatusCmd( String contextID, String msg )
    {
        Main.kjas_debug( "sendShowStatusCmd, msg = " + msg );

        int length = contextID.length() + msg.length() + 4;
        char[] chars = new char[ length + 8 ]; //for length of message
        int index = 0;
        char sep = (char) 0;
        char[] tmpchar = getPaddedLength( length );

        //fill in the length of the command
        for( int i = 0; i < 8; i++ )
        {
            chars[index++] = tmpchar[i];
        }

        //fill chars array to print it with the PrintStream
        chars[index++] = (char) ShowStatusCode;
        chars[index++] = sep;

        tmpchar = contextID.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = msg.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        signals.print( chars );
    }

    public void sendResizeAppletCmd( String contextID, String appletID,
                                     int width, int height )
    {
        Main.kjas_debug( "sendResizeAppletCmd, contextID = " + contextID + ", appletID = " + appletID + ", width = " + width + ", height = " + height );

        String width_str = String.valueOf( width );
        String height_str = String.valueOf( height );

        //lenght = length of args plus code, 4 seps, end
        int length = contextID.length() + appletID.length() + width_str.length() +
                     height_str.length() + 6;
        char[] chars = new char[ length + 8 ]; //for length of message
        int index = 0;
        char sep = (char) 0;
        char[] tmpchar = getPaddedLength( length );

        //fill in the length of the command
        for( int i = 0; i < 8; i++ )
        {
            chars[index++] = tmpchar[i];
        }

        //fill chars array to print it with the PrintStream
        chars[index++] = (char) ResizeAppletCode;
        chars[index++] = sep;

        tmpchar = contextID.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = appletID.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = width_str.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        tmpchar = height_str.toCharArray();
        for( int i = 0; i < tmpchar.length; i++ )
            chars[index++] = tmpchar[i];
        chars[index++] = sep;

        signals.print( chars );
    }

    private char[] getPaddedLength( int length )
    {
        String length_str = String.valueOf( length );

        int pads = 8 - length_str.length();
        String space = new String( " " );
        String rval = length_str;
        for( int i = 0; i < pads; i++ )
        {
            rval = space.concat( rval );
        }

        if( rval.length() != 8 )
        {
           throw new IllegalArgumentException( "can't create string number of length = 8" );
        }

        return rval.toCharArray();
    }

    private int readPaddedLength( int string_size )
        throws IOException
    {
            //read in 8 bytes for command length- length will be sent as a padded string
            char[] length = new char[string_size];
            for( int i = 0; i < string_size; i++ )
            {
                length[i] = (char) commands.read();
            }
            String length_str = new String( length );

            return Integer.parseInt( length_str.trim() );
    }

}
