package org.kde.kjas.server;

/**
 * An interface that should be implemented by classes that want to respond to
 * messages sent by applets.
 *
 */

public interface KJASAppletRunnerListener
{
   public void showStatus( String contextId, String message );

   public void showDocument( String contextId, String url );
   public void showDocumentInFrame( String contextId, String url, String target );
}
