/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __kbrowser_h__
#define __kbrowser_h__

#include <qpoint.h>
#include <qlist.h>
#include <qdatastream.h>

#include <qaction.h>
#include <kpart.h>

class KFileItem;
typedef QList<KFileItem> KFileItemList;

class QString;

namespace KParts {

 /**
  * The following standard actions are defined by the host of the view :
  * 
  * cut : copy selected items to clipboard and notifies that a cut has been done, using DCOP
  * copy : copy selected items to clipboard (and notifies it's not a cut)
  * pastecut : called when doing a paste after a cut
  * pastecopy : called when doing a paste after a copy
  * del : delete selected items (couldn't call it delete!)
  * trash : move selected items to trash
  * print : print :-)
  *
  * reparseConfiguration : re-read configuration and apply it
  * saveLocalProperties : save current configuration into .directory
  * savePropertiesAsDefault : save current configuration as default
  * refreshMimeTypes : if the view uses mimetypes it should re-determine them
  * 
  * 
  * The view should emit enableAction when an action should be enabled/disabled,
  * and should define a slot with the name of the action in order to implement the action.
  * The browser will detect the slot automatically and connect its action to it when
  * appropriate (i.e. when the view is active)
  */
class BrowserExtension : public QObject
{
  Q_OBJECT
public:
  /**
   * Constructor
   *
   * @param parent the KParts::ReadOnlyPart that this extension ... "extends" :)
   * @param name an optionnal name for the extension
   */
  BrowserExtension( KParts::ReadOnlyPart *parent,
                    const char *name = 0L ) :
    QObject( parent, name ), m_part( parent ) {}

  virtual ~BrowserExtension() { }

  /**
   * Move the view to the position (x,y)
   * (This allows the browser to restore a view at the exact
   *  position it was when we left it, during navigation)
   * For a scrollview, implement this using setContentsPos()
   */
  virtual void setXYOffset( int /* x */, int /* y */ ) {};
  /**
   * @return the current x offset
   * For a scrollview, implement this using contentsX()
   */
  virtual int xOffset() { return 0; }
  /**
   * @return the current y offset
   * For a scrollview, implement this using contentsY()
   */
  virtual int yOffset() { return 0; }

  /**
   * Used by the browser to save the current state of the view
   * (in order to restore it if going back in navigation)
   * If you want to save additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void saveState( QDataStream &stream )
  { stream << m_part->url() << (Q_INT32)xOffset() << (Q_INT32)yOffset(); }

  /**
   * Used by the browser to restore the view in the state
   * it was when we left it.
   * If you saved additionnal properties, reimplement it
   * but don't forget to call the parent method (probably first).
   */
  virtual void restoreState( QDataStream &stream )
  { KURL u; Q_INT32 xOfs, yOfs; stream >> u >> xOfs >> yOfs;
    m_part->openURL( u ); setXYOffset( xOfs, yOfs ); }

signals:
  /**
   * Enable or disable a standard action held by the browser.
   * See class documentation for the list of standard actions.
   */
  void enableAction( const char * name, bool enabled );

  /**
   * Open @p url in the browser, optionnally forcing @p reload, and 
   * optionnally setting the x and y offsets.
   * The @serviceType allows to ...
   */
  void openURLRequest( const KURL &url, bool reload, int xOffset, int yOffset, const QString &serviceType = QString::null );

  void setStatusBarText( const QString &text );

  /**
   * Update the URL shown in the browser's location bar to @p url
   */
  void setLocationBarURL( const QString &url );
  void createNewWindow( const KURL &url );
  void loadingProgress( int percent );
  void speedProgress( int bytesPerSecond );
  void popupMenu( const QPoint &_global, const KFileItemList &_items );

private:
  KParts::ReadOnlyPart *m_part;
};

};

#endif

