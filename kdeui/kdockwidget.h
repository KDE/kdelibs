/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KDOCKWIDGET_H
#define KDOCKWIDGET_H

#include <qpoint.h>
#include <qlist.h>
#include <qframe.h>
#include <ktmainwindow.h>

#include "kdocktabctl.h"

class KDockSplitter;
class KDockMoveManager;
class KDockWidget;
class KDockButton_Private;

class QObjectList;
class QPopupMenu;
class QVBoxLayout;
class QHBoxLayout;
class QPixmap;

class KToolBar;
class KConfig;

typedef QList<QWidget> WidgetList;

class KDockWidgetAbstractHeader : public QFrame
{
  Q_OBJECT
public:
  KDockWidgetAbstractHeader( KDockWidget* parent, const char* name = 0L );
  virtual ~KDockWidgetAbstractHeader(){};

  virtual void setTopLevel( bool ){};
  virtual void saveConfig( KConfig* ){};
  virtual void loadConfig( KConfig* ){};
};

class KDockWidgetAbstractHeaderDrag : public QFrame
{
  Q_OBJECT
public:
  KDockWidgetAbstractHeaderDrag( KDockWidgetAbstractHeader* parent,
                                 KDockWidget* dock, const char* name = 0L );
  virtual ~KDockWidgetAbstractHeaderDrag(){};

  KDockWidget* dockWidget(){ return d; }

private:
  KDockWidget* d;
};

class KDockWidgetHeaderDrag : public KDockWidgetAbstractHeaderDrag
{
  Q_OBJECT
public:
  KDockWidgetHeaderDrag( KDockWidgetAbstractHeader* parent, KDockWidget* dock,
                         const char* name = 0L );
  virtual ~KDockWidgetHeaderDrag(){};

protected:
  virtual void paintEvent( QPaintEvent* );
};

class KDockWidgetHeader : public KDockWidgetAbstractHeader
{
  Q_OBJECT
public:
  KDockWidgetHeader( KDockWidget* parent, const char* name = 0L );
  virtual ~KDockWidgetHeader(){};

  virtual void setTopLevel( bool );
  virtual void saveConfig( KConfig* );
  virtual void loadConfig( KConfig* );

protected slots:
  void slotStayClicked();

protected:
  QHBoxLayout* layout;
  KDockButton_Private* closeButton;
  KDockButton_Private* stayButton;
  KDockWidgetHeaderDrag* drag;
};

class KDockTabGroup : public KDockTabCtl
{
  Q_OBJECT
public:
  KDockTabGroup( QWidget *parent = 0, const char *name = 0 )
  :KDockTabCtl( parent, name ){};
  virtual ~KDockTabGroup(){};
};

/**
 * Floatable widget that can be dragged around with the mouse und encapsulate the actual widget.
 *
 * You just grip the double-lined panel, tear it off its parent widget,
 * drag it somewhere and let it loose. Depending on the position where you leave it, the dockwidget
 * becomes a toplevel window on the desktop (floating mode) or docks to a new widget (dock mode).
 + Note: KDockWidget can only be dock to KDockWidget.
 *
 * @author Max Judin.
 * @version $Id$
*/
class KDockWidget: public QWidget
{
  Q_OBJECT
friend class KDockManager;
friend class KDockSplitter;
friend class KDockMainWindow;

public:
  KDockWidget( KDockManager* dockManager, const char* name,
               const QPixmap &pixmap, QWidget* parent = 0L );
  virtual ~KDockWidget();

  enum DockPosition
  {
    DockNone   = 0,
    DockTop    = 0x0001,
    DockLeft   = 0x0002,
    DockRight  = 0x0004,
    DockBottom = 0x0008,
    DockCenter = 0x0010,
    DockDesktop= 0x0020,

    DockCorner = DockTop | DockLeft | DockRight | DockBottom,
    DockFullSite = DockCorner | DockCenter,
    DockFullDocking = DockFullSite | DockDesktop
  }; 

  /* if target is null  - dock move to desktop at position pos;
   * check - only for internal uses;
   * return result GroupDockWidget
   */
  KDockWidget* manualDock( KDockWidget* target, DockPosition dockPos, int spliPos = 50, QPoint pos = QPoint(0,0), bool check = false );

  void setEnableDocking( int pos );
  int enableDocking(){ return eDocking; }

  void setDockSite( int pos ){ sDocking = pos;}
  int dockSite(){ return sDocking; }

  void setWidget( QWidget* );
  void setHeader( KDockWidgetAbstractHeader* );

  void makeDockVisible();
  bool mayBeHide();
  bool mayBeShow();

  KDockManager* dockManager(){ return manager; }

  virtual bool event( QEvent * );
  virtual void show();

public slots:
  void changeHideShowState();

protected:
  KDockTabGroup* parentTabGroup();
  void updateHeader();

signals:
  //emit for dock when another KDockWidget docking in this KDockWidget
  void docking( KDockWidget*, KDockWidget::DockPosition );
  void setDockDefaultPos();

public slots:
  void undock();

private:
  void setDockTabName( KDockTabGroup* );
  void applyToWidget( QWidget* s, const QPoint& p  = QPoint(0,0) );

  KDockWidgetAbstractHeader* header;
  QWidget* widget;
  QVBoxLayout* layout;
  KDockManager* manager;
  QPixmap* pix;

  int eDocking;
  int sDocking;

  // GROUP data
  QString firstName;
  QString lastName;
  Orientation splitterOrientation;
  bool isGroup;
  bool isTabGroup;

  class KDockWidgetData;
  KDockWidgetData *d;
};

/**
 * The manager that knows all dockwidgets and handles the dock process.
 *
 * @author Max Judin.
 * @version $Id$
*/
class KDockManager: public QObject
{
  Q_OBJECT
friend class KDockWidget;
friend class KDockMainWindow;

public:
  KDockManager( QWidget* mainWindow, const char* name = 0L );
  virtual ~KDockManager();

  void writeConfig( KConfig* c = 0L, QString group = QString::null );
  void readConfig ( KConfig* c = 0L, QString group = QString::null );
  void activate();

  virtual bool eventFilter( QObject *, QEvent * );

  KDockWidget* findWidgetParentDock( QWidget* );
  void makeWidgetDockVisible( QWidget* w ){ findWidgetParentDock(w)->makeDockVisible(); }

  QPopupMenu* dockHideShowMenu(){ return menu; }

  KDockWidget* getDockWidgetFromName( const QString& dockName );

signals:
  void change();
  void replaceDock( KDockWidget* oldDock, KDockWidget* newDock );
  void setDockDefaultPos( KDockWidget* );

private slots:
  void slotMenuPopup();
  void slotMenuActivated( int );

private:
  struct MenuDockData
  {
    MenuDockData( KDockWidget* _dock, bool _hide )
    {
      dock = _dock;
      hide = _hide;
    };
    ~MenuDockData(){};

    KDockWidget* dock;
    bool hide;
  };

  KDockWidget* findDockWidgetAt( const QPoint& pos );
  void findChildDockWidget( QWidget*&, const QWidget* p, const QPoint& pos );
  void findChildDockWidget( const QWidget* p, WidgetList*& );

  void startDrag( KDockWidget* );
  void dragMove( KDockWidget*, QPoint pos );
  void drop();

  QWidget* main;
  KDockMoveManager* mg;
  KDockWidget* currentDragWidget;
  KDockWidget* currentMoveWidget; // widget where mouse moving
  WidgetList* childDockWidgetList;
  KDockWidget::DockPosition curPos;
  QObjectList* childDock;
  QObjectList* autoCreateDock;
  int storeW;
  int storeH;
  bool draging;
  bool undockProcess;
  bool dropCancel;

  QPopupMenu* menu;
  QList<MenuDockData> *menuData;
};

/**
 * A special kind of KTMainWindow that is able to have dockwidget child widgets.
 *
 * The main widget should be a dockwidget where other dockwidgets can be docked to
 * the left, right, top, bottom or to the middle.
 * Furthermore, the KDockMainWindow has got the KDocManager and some data about the dock states.
 *
 * @author Max Judin.
 * @version $Id$
*/
class KDockMainWindow : public KTMainWindow
{
  Q_OBJECT
public:
  KDockMainWindow( const char *name = 0L );
  virtual ~KDockMainWindow();

  KDockManager* manager(){ return dockManager; }

  void setMainDockWidget( KDockWidget* );
  KDockWidget* getMainDockWidget(){ return mainDockWidget; }

  KDockWidget* createDockWidget( const QString& name, const QPixmap &pixmap, QWidget* parent = 0L );

  void writeDockConfig( KConfig* c = 0L, QString group = QString::null );
  void readDockConfig ( KConfig* c = 0L, QString group = QString::null );

  void activateDock(){ dockManager->activate(); }

  QPopupMenu* dockHideShowMenu(){ return dockManager->dockHideShowMenu(); }

  void makeDockVisible( KDockWidget* dock );

  /*! This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void makeWidgetDockVisible( QWidget* widget );

  void setView( QWidget* );

protected slots:
  void slotDockChange();
  void slotToggled( int );
  void slotReplaceDock( KDockWidget* oldDock, KDockWidget* newDock );

protected:
  struct DockPosData
  {
    KDockWidget* dock;
    KDockWidget* dropDock;
    KDockWidget::DockPosition pos;
    int sepPos;
  };

  void toolBarManager( bool toggled, DockPosData &data );

  KDockWidget* mainDockWidget;
  KDockManager* dockManager;

  DockPosData DockL;
  DockPosData DockR;
  DockPosData DockT;
  DockPosData DockB;

  KToolBar* toolbar;
};

#endif


