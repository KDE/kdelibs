package org.kde.kjas.server;

/**
 * An interface that should be implemented by classes that want to respond to
 * messages sent by applets.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * </PRE>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
public interface KJASAppletRunnerListener
{
   public void showStatus( int contextId, String message );

   public void showDocument( int contextId, String url );
   public void showDocumentInFrame( int contextId, String url, String target );
}
