package org.kde.kjas.server;

/**
 * An interface that should be implemented by classes that want to respond to
 * messages sent by applets.
 *
 * <H3>Change Log</H3>
 * <PRE>
 * $Log$
 * Revision 1.1.1.1  1999/07/22 17:28:08  rich
 * This is a current snapshot of my work on adding Java support
 * to KDE. Applets now work!
 *
 * </PRE>
 *
 * @version $Id$
 * @author Richard Moore, rich@kde.org
 */
public interface KJASAppletRunnerListener
{
   public void showStatus( String contextId, String message );

   public void showDocument( String contextId, String url );
   public void showDocumentInFrame( String contextId, String url, String target );
}
