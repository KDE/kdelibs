package org.kde.kjas.server;

import java.io.*;
import java.util.*;

/**
 * Encapsulates the KJAS protocol.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * </PRE>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
public class KJASProtocolHandler
{
   BufferedReader commands;
   PrintStream signals;
   KJASAppletRunner runner;

   public KJASProtocolHandler( InputStream commands,
                               OutputStream signals,
                               KJASAppletRunner runner,
                               String password )
   {
      this.commands = new BufferedReader( new InputStreamReader( commands ) );
      this.runner = runner;
   }

   public void commandLoop()
      throws IOException
   {
      String line = commands.readLine();
      while ( line != null ) {
         processCommand( line );
         line = commands.readLine();
      }
   }

   public void processCommand( String command )
      throws IllegalArgumentException
   {
      // Sanity checks
      if ( command == null )
         throw new IllegalArgumentException( "processCommand() received null" );
      if ( command.length() > 1024 )
         throw new IllegalArgumentException( "processCommand() received suspiciously large command string (ignoring)" );

      int index = command.indexOf( '!' );
      String commandName;
      String args;

      if ( index != -1) {
         commandName = command.substring( 0, index );
         args = command.substring( index + 1, command.length() );
      }
      else {
         commandName = command;
         args = "";
      }
      
      if ( commandName.equals( "createContext" ) ) {
         createContext( args );
      }
      else if ( commandName.equals( "destroyContext" ) ) {
         destroyContext( args );
      }
      else if ( commandName.equals( "createApplet" ) ) {
         createApplet( args );
      }
      else if ( commandName.equals( "createJARApplet" ) ) {
         //         createJARApplet( args );
      }
      else if ( commandName.equals( "destroyApplet" ) ) {
         destroyApplet( args );
      }
      else if ( commandName.equals( "showApplet" ) ) {
         showApplet( args );
      }
      else if ( commandName.equals( "startApplet" ) ) {
         startApplet( args );
      }
      else if ( commandName.equals( "stopApplet" ) ) {
         stopApplet( args );
      }
      else if ( commandName.equals( "setParameter" ) ) {
         setParameter( args );
      }
      else if ( commandName.equals( "quit" ) ) {
         System.exit(0);
      }
      else {
         throw new IllegalArgumentException( "Unknown command: " + commandName );
      }
   }

   public void createContext( String args )
      throws IllegalArgumentException
   {
      try {
         String idStr = args.trim();
         int id = Integer.parseInt( idStr );
         runner.createContext( id );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "createContext() " + e.toString() );
      }
   }

   public void destroyContext( String args )
      throws IllegalArgumentException
   {
      try {
         String idStr = args.trim();
         int id = Integer.parseInt( idStr );
         runner.destroyContext( id );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "destroyContext() " + e.toString() );
      }
   }

   public void createApplet( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();
         String name = tok.nextToken();
         String className = tok.nextToken();
         String base = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.createApplet( contextId, appletId, name, className, base );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "createContext() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "createApplet(): Wrong number of args" );
      }
   }

   public void destroyApplet( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.destroyApplet( contextId, appletId );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "destroyApplet() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "destroyApplet(): Wrong number of args" );
      }
   }

   public void showApplet( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();
         String title = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.showApplet( contextId, appletId, title );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "showApplet() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "showApplet(): Wrong number of args" );
      }
   }

   public void startApplet( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.startApplet( contextId, appletId );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "startApplet() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "startApplet(): Wrong number of args" );
      }
   }

   public void stopApplet( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.stopApplet( contextId, appletId );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "stopApplet() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "stopApplet(): Wrong number of args" );
      }
   }

   public void setParameter( String args )
      throws IllegalArgumentException
   {
      try {
         String buf = args.trim();
         StringTokenizer tok = new StringTokenizer( buf, "!" );

         String contextIdStr = tok.nextToken();
         String appletIdStr = tok.nextToken();
         String name = tok.nextToken();
         String value = tok.nextToken();

         int contextId = Integer.parseInt( contextIdStr );
         int appletId = Integer.parseInt( appletIdStr );
         runner.setParameter( contextId, appletId, name, value );
      }
      catch ( NumberFormatException e ) {
         throw new IllegalArgumentException( "stopApplet() " + e.toString() );
      }
      catch ( NoSuchElementException nsee ) {
         throw new IllegalArgumentException( "stopApplet(): Wrong number of args" );
      }
   }

   public static void main( String[] args )
   {
      try {
         KJASAppletRunner runner = new KJASAppletRunner();
         KJASProtocolHandler handler = new KJASProtocolHandler( System.in, System.out,
                                                                runner, "friend" );
         handler.processCommand( "createContext!0" );
         handler.processCommand( "createApplet!0!0!fred!Lake.class!http://127.0.0.1/applets/" );
         handler.processCommand( "setParameter!0!0!image!logo.gif" );
         handler.processCommand( "showApplet!0!0!unique_title_one" );
         System.err.println( "Entering commandLoop()" );
         handler.commandLoop();
      }
      catch ( Exception e ) {
         System.err.println( "Error: " + e );
         e.printStackTrace();
      }
   }
}
