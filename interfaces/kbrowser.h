/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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

#include <qwidget.h>
#include <qpoint.h>
#include <qlist.h>
#include <qdatastream.h>

#include <kaction.h>
#include <kpart.h>

class KFileItem;
typedef QList<KFileItem> KFileItemList;

class QString;
class KConfig;

class ViewPropertiesExtension : public QObject
{
  Q_OBJECT
public:
  ViewPropertiesExtension( QObject *parent, const char *name = 0 ) : QObject( parent, name ) {}

  virtual void reparseConfiguration() = 0;
  virtual void saveLocalProperties() = 0;
  virtual void savePropertiesAsDefault() = 0;
  // Reimplement if the view shows icons (Icon/Tree/DirTree...)
  virtual void refreshMimeTypes() {}
};

class PrintingExtension : public QObject
{
  Q_OBJECT
public:
  PrintingExtension( QObject *parent, const char *name = 0L ) : QObject( parent, name ) {}

  virtual void print() = 0;

};

class EditExtension : public QObject
{
  Q_OBJECT
public:
  EditExtension( QObject *parent, const char *name = 0L ) : QObject( parent, name ) {}

  virtual void can( bool &cut, bool &copy, bool &paste, bool &move ) = 0;

  virtual void cutSelection() = 0;
  virtual void copySelection() = 0;
  virtual void pasteSelection( bool move = false ) = 0;
  virtual void moveSelection( const QString &destinationURL = QString::null ) = 0;
  // reimplement if the view deals with urls (currently only used by slotDelete)
  virtual QStringList selectedUrls() { return QStringList(); }

signals:
  void selectionChanged();

};

class BrowserView : public QObject
{
  Q_OBJECT
public:
  BrowserView( KParts::ReadOnlyPart *parent = 0L, const char *name = 0L ) : QObject( parent, name ) { m_part = parent; }

  virtual ~BrowserView() { }

  virtual void setXYOffset( int x, int y ) = 0;
  virtual int xOffset() = 0;
  virtual int yOffset() = 0;

  virtual void saveState( QDataStream &stream )
  { stream << m_part->url() << (Q_INT32)xOffset() << (Q_INT32)yOffset(); }

  virtual void restoreState( QDataStream &stream )
  { KURL u; Q_INT32 xOfs, yOfs; stream >> u >> xOfs >> yOfs;
    m_part->openURL( u ); setXYOffset( xOfs, yOfs ); }

signals:
  void openURLRequest( const QString &url, bool reload, int xOffset, int yOffset, const QString &serviceType = QString::null );
  void setStatusBarText( const QString &text );
  void setLocationBarURL( const QString &url );
  void createNewWindow( const QString &url );
  void loadingProgress( int percent );
  void speedProgress( int bytesPerSecond );
  void popupMenu( const QPoint &_global, const KFileItemList &_items );

private:
  KParts::ReadOnlyPart *m_part;
};

#endif

