package org.kde.kjas.server;

import java.io.*;
import java.util.*;
import java.awt.*;

/**
 * Encapsulates the KJAS protocol.
 *
 */

public class KJASProtocolHandler
{
    private static final int CreateContextCode  = 1;
    private static final int DestroyContextCode = 2;
    private static final int CreateAppletCode   = 3;
    private static final int DestroyAppletCode  = 4;
    private static final int StartAppletCode    = 5;
    private static final int StopAppletCode     = 6;
    private static final int ShowAppletCode     = 7;

    private static final int ShutdownServerCode = 9;

    private static final int ShowDocumentCode   = 12;
    private static final int ShowURLInFrameCode = 13;
    private static final int ShowStatusCode     = 14;
    private static final int ResizeAppletCode   = 15;

    private KJASAppletRunner  runner;

    //Stream for reading in commands
    private PushbackInputStream commands;

    //Stream for writing out callbacks
    private PrintStream       signals;

    //used for parsing each command as it comes in
    private int cmd_index;

    public KJASProtocolHandler( InputStream  _commands,
                                OutputStream _signals,
                                KJASAppletRunner _runner,
                                String password )
    {
        commands = new PushbackInputStream( _commands );

        signals = new PrintStream( _signals );

        runner = _runner;
    }

    public void commandLoop()
        throws IOException
    {
        /*  The calls to commands.read will block and not return, so we don't need to
         *  sleep, etc.
         */

        while( true )
        {
            Main.kjas_debug( "Start commandLoop" );
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

            runner.createContext( contextID );
        } else
        if( cmd_code_value == DestroyContextCode )
        {
            //parse out contextID- 1 argument
            String contextID = getArg( command );

            Main.kjas_debug( "destroyContext, id = " + contextID );

            runner.destroyContext( contextID );
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

            //get the number of parameter pairs...
            String str_params = getArg( command );
            int num_params = Integer.parseInt( str_params.trim() );

            Hashtable params = new Hashtable();
            for( int i = 0; i < num_params; i++ )
            {
                String name  = getArg( command );
                String value = getArg( command );

                params.put( name, value );
            }

            Main.kjas_debug( "createApplet, context = " + contextID + ", applet = " + appletID );
            Main.kjas_debug( "              name = " + appletName + ", classname = " + className );
            Main.kjas_debug( "              baseURL = " + baseURL + ", codeBase = " + codeBase );
            Main.kjas_debug( "              archives = " + archives );

            runner.createApplet( contextID, appletID, appletName, className,
                                 baseURL, codeBase, archives,
                                 new Dimension( Integer.parseInt(width), Integer.parseInt(height) ),
                                 params );
        } else
        if( cmd_code_value == DestroyAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );

            Main.kjas_debug( "destroyApplet, context = " + contextID + ", applet = " + appletID );

            runner.destroyApplet( contextID, appletID );
        } else
        if( cmd_code_value == StartAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );

            Main.kjas_debug( "startApplet, context = " + contextID + ", applet = " + appletID );

            runner.startApplet( contextID, appletID );
        } else
        if( cmd_code_value == StopAppletCode )
        {
            //2 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );

            Main.kjas_debug( "stopApplet, context = " + contextID + ", applet = " + appletID );

            runner.startApplet( contextID, appletID );
        } else
        if( cmd_code_value == ShowAppletCode )
        {
            //3 arguments
            String contextID = getArg( command );
            String appletID = getArg( command );
            String title = getArg( command );

            Main.kjas_debug( "showApplet, context = " + contextID + ", applet = " + appletID );

            runner.showApplet( contextID, appletID, title );
        } else
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
        Main.kjas_debug( "sendResizeAppletCmd" );

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
