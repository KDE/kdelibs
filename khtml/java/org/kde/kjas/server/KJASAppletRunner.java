package org.kde.kjas.server;

import java.util.*;
import java.net.*;
import java.applet.*;
import java.awt.*;

/**
 * Manages the running contexts and applets.
 *
 */

public class KJASAppletRunner
{
    Hashtable contexts;
    KJASAppletRunnerListener listener;

    public KJASAppletRunner()
    {
        contexts = new Hashtable();
    }

    public void setAppletRunnerListener( KJASAppletRunnerListener listener )
    {
        this.listener = listener;
    }


    // Methods that are invoked in respose to requests by the
    // embedding application.
    public void createContext( String contextId )
        throws IllegalArgumentException
    {
        if ( contexts.get( contextId ) != null )
            throw new IllegalArgumentException( "Invalid contextId passed to createContext() "
                                                + contextId );

        KJASAppletContext context = new KJASAppletContext( contextId );
        contexts.put( contextId, context );
    }

    public void destroyContext( String contextId )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( contexts == null )
            throw new IllegalArgumentException( "Invalid contextId passed to destroyContext() "
                                                + contextId );

        context.destroy();
        contexts.remove( contextId );
    }

    public void createApplet( String contextId,
                              String appletId,
                              String name,
                              String className,
                              String docBase,
                              String codeBase,
                              String jars,
                              Dimension size,
                              Hashtable params )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( context == null )
            throw new IllegalArgumentException( "Invalid contextId passed to createApplet() "
                                                + contextId );
        URL docBaseURL = null;
        try
        {
            docBaseURL = new URL( docBase );
        }
        catch ( MalformedURLException mue )
        {
            throw new IllegalArgumentException( mue.toString() );
        }

        URL codeBaseURL = null;
        if(codeBase != null)
        {
            if(!codeBase.endsWith("/"))
                codeBase = codeBase + "/";
            try
            {
                codeBaseURL = new URL( docBaseURL, codeBase );
            }
            catch(Exception e) {}
        }

        if(codeBaseURL == null)
        {
            String urlString = docBaseURL.getFile();
            int i = urlString.lastIndexOf('/');
            if (i >= 0 && i < urlString.length() - 1)
            {
                try
                {
                    codeBaseURL = new URL(docBaseURL, urlString.substring(0, i + 1));
                }
                catch (Exception e) {}
            }
        }

        // codeBaseURL can not be null. This should not happen,
        // but if it does we fall back to document base
        if(codeBaseURL == null)
            codeBaseURL = docBaseURL;

        context.createApplet( appletId, className,
                              codeBaseURL, docBaseURL,
                              jars, name, size, params );
    }

    public void destroyApplet( String contextId, String appletId )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( context == null )
            throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                                + contextId );

        context.destroyApplet( appletId );
    }

    public void showApplet( String contextId, String appletId, String title )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( context == null )
            throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                                + contextId );

        context.showApplet( appletId, title );
    }

    public void startApplet( String contextId, String appletId )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( context == null )
            throw new IllegalArgumentException( "Invalid contextId passed to startApplet() "
                                                + contextId );

        context.startApplet( appletId );
    }

    public void stopApplet( String contextId, String appletId )
        throws IllegalArgumentException
    {
        KJASAppletContext context = (KJASAppletContext) contexts.get( contextId );
        if ( context == null )
            throw new IllegalArgumentException( "Invalid contextId passed to stopApplet() "
                                                + contextId );

        context.stopApplet( appletId );
    }

}
