/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __browser_h__
#define __browser_h__

#include <qwidget.h>
#include <qpoint.h>
#include <qlist.h>

#include <kaction.h>

class KFileItem;
typedef QList<KFileItem> KFileItemList;

class QString;

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

  virtual void can( bool &copy, bool &paste, bool &move ) = 0;

  virtual void copySelection() = 0;
  virtual void pasteSelection() = 0;
  virtual void moveSelection( const QString &destinationURL = QString::null ) = 0;

signals:
  void selectionChanged();  

};

class BrowserView : public QWidget
{
  Q_OBJECT
public:
  BrowserView( QWidget *parent = 0L, const char *name = 0L ) : QWidget( parent, name ) {}

  virtual ~BrowserView() { }

  enum ActionFlags
  {
    MenuView  = 0x01,
    MenuEdit  = 0x02,
    ToolBar   = 0x04
  };
  
  struct ViewAction
  {
    ViewAction() : m_action( 0L ) { }
    ViewAction( QAction *action, int flags )
    : m_action( action ), m_flags( flags ) { }
    
    QAction *m_action;
    int m_flags;
  };

  virtual void openURL( const QString &url, bool reload = false,
                        int xOffset = 0, int yOffset = 0 ) = 0;

  virtual QString url() = 0;
  virtual int xOffset() = 0;
  virtual int yOffset() = 0;
  virtual void stop() = 0;

  QValueList<ViewAction> *actions() { return &m_actionCollection; }

signals:
  void openURLRequest( const QString &url, bool reload, int xOffset, int yOffset );
  void started();
  void completed();
  void canceled();
  void setStatusBarText( const QString &text );
  void setLocationBarURL( const QString &url );
  void createNewWindow( const QString &url );
  void loadingProgress( int percent );
  void speedProgress( int bytesPerSecond );
  void popupMenu( const QPoint &_global, const KFileItemList &_items );
  
private:
  QValueList<ViewAction> m_actionCollection;
};

#endif

