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
#ifndef KDOCKTABCTL_H
#define KDOCKTABCTL_H

#include <qwidget.h>
#include <qlist.h>
#include <qpixmap.h>

class QWidgetStack;
class QBoxLayout;
class QPushButton;
class KDockTabBarPainter;

struct KDockTabCtl_Private;
struct KDockTabBar_Private;

/**
 * The actual tab bar. This class is covered by its manager, KDockTabCtl.
 * It provides some set/get methods and managment
 * methods like insert, remove, activate for tab widgets.
 */
class KDockTabBar : public QWidget
{
  Q_OBJECT
  
  friend KDockTabBarPainter;

public:
  
  /**
   * Constructs a tab bar widget.
   */
  KDockTabBar( QWidget * parent = 0, const char * name = 0 );

  /**
   * Destructs this.
   */
  ~KDockTabBar();

  /**
   * For setting (or asking for) the current tab page position.
   */
  enum TabPos
  {
    TAB_TOP,
    TAB_RIGHT
  };

  /**
   * Sets the position to tabpos and repaints this.
   */
  void setTabPos( TabPos tabpos);

  /**
   * Inserts a new tab page.
   */
  int insertTab( const QString &label, int id = -1 );

  /**
   * Sets an icon for the tab page with that id.
   */
  void setPixmap( int id, const QPixmap &pix );

  /**
   * Sets the text color for the tab page with that id.
   */
  void setTextColor( int id, const QColor &color );

  /**
   * Returns the text color for the tab page with that id.
   */
  const QColor& textColor( int id );

  /**
   * Removes the tab page with that id.
   */
  void removeTab( int id);

  /**
   * Returns the current tab page.
   */
  int  currentTab(){ return _currentTab; }

  /**
   * Sets the current tab page to the page with that id.
   */
  void setCurrentTab( int id, bool allowDisable = false );

  /**
   * Enables or disables the tab page with that id.
   */
  void setTabEnabled( int, bool );

  /**
   * Returns if the tab page with that id is enabled or disabled.
   */
  bool isTabEnabled( int );

  /**
   * Sets the title of the tab page with that id.
   */
  void setTabCaption( int id, const QString &caption );

  /**
   * Returns the title of the tab page with that id.
   */
  QString tabCaption( int id );

  /**
   * Calls QWidget::show() and showPage(..) for the current tab page, additionally.
   */
  virtual void show();

  /**
   * Sets the font of this.
   */
  virtual void setFont( const QFont & );

  /**
   * Shows the icons for the tab pages in the header.
   */
  void showTabIcon( bool );

  /**
   * Returns if the icons for the tab pages are shown in the header.
   */
  bool isShowTabIcon(){ return iconShow; }

signals:

  /**
   * Signals that a tab page with that id is selected.
   */
  void tabSelected( int id);

  /**
   * Signals that the right mouse buttons is pressed on the tab page with that id.
   */
  void rightButtonPress( int id, QPoint );

protected slots:

  /**
   * Does things that should be done if someone has clicked the left mouse button.
   */
  void leftClicked();

  /**
   * Does things that should be done if someone has clicked the right mouse button.
   */
  void rightClicked();

protected:

  /**
   * Draws this.
   */
  virtual void paintEvent( QPaintEvent* );

  /**
   * Resizes this.
   */
  virtual void resizeEvent( QResizeEvent* );

private:
  void setButtonPixmap();
  void updateHeight();

  KDockTabBar_Private* findData( int id );
  int tabsWidth();
  void tabsRecreate();

  TabPos tabPos;
  KDockTabBarPainter* barPainter;
  QList<KDockTabBar_Private> *mainData;
  int _currentTab;
  int leftTab;

  QPixmap* up_xpm;
  QPixmap* down_xpm;
  QPixmap* left_xpm;
  QPixmap* right_xpm;

  QPushButton *right;
  QPushButton *left;
  bool rightscroll;
  bool leftscroll;
  bool iconShow;
};

/**
 * The draw helper for the KDockTabBar.
 */
class KDockTabBarPainter : public QWidget
{
  Q_OBJECT
  friend KDockTabBar;

private:
  
  KDockTabBarPainter( KDockTabBar* parent );
  ~KDockTabBarPainter();

  void drawBuffer();
    int findBarByPos( int x, int y );

  QPixmap* buffer;
  int mousePressTab;
  int delta;

protected:
  
  /**
   *
   */
  virtual void mousePressEvent ( QMouseEvent * );
  
  /**
   *
   */
  virtual void mouseReleaseEvent ( QMouseEvent * );

  /**
   *
   */
  virtual void resizeEvent( QResizeEvent * );
  
  /**
   *
   */
  virtual void paintEvent( QPaintEvent * );
};

/**
 * A manager for a single KDockTabBar. The dockwidgets ever use this class instead of 
 * accessing the KDockTabBar directly.
 */
class KDockTabCtl : public QWidget
{
  Q_OBJECT
  
public:
  
  /**
   * Constructs a dock-tab control object.
   */
  KDockTabCtl( QWidget *parent = 0, const char *name = 0 );

  /**
   * Destructs this.
   */
  ~KDockTabCtl();

  /**
   * Calls setTabPos(..) of its KDockTabBar but does layout actions, additionally.
   */
  void setTabPos( KDockTabBar::TabPos );
  
  /**
   * Returns the current tab position of its docktabbar.
   */
  KDockTabBar::TabPos getTabPos(){ return tabPos; }

  /**
   * Inserts a new tab page in the encapsulated docktabbar.
   */
  int insertPage( QWidget *, const QString &label, int id = -1 );

  /**
   * Sets the title of the tab page.
   */
  void setPageCaption( QWidget*, const QString & );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageCaption( int id, const QString &caption ){ setPageCaption( page(id), caption ); }

  /**
   * Returns the title of the tab page.
   */
  QString pageCaption( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  QString pageCaption( int id ){ return pageCaption( page(id) ); }
  
  /**
   * Sets an icon for the tab page (shown in the tab header).
   */
  void setPixmap( QWidget* , const QPixmap &pix );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPixmap( int id, const QPixmap &pix ){ setPixmap( page(id), pix ); }

  /**
   * Sets the text color of the tab page.
   */
  void setTabTextColor( QWidget*, const QColor &color );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setTabTextColor( int id, const QColor &color ){ setTabTextColor( page(id), color ); }

  /**
   * Returns the text color of the tab page.
   */
  const QColor& tabTextColor( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  const QColor& tabTextColor( int id ){ return tabTextColor( page(id) ); }
  
  /**
   * Returns if the tab page is enabled (if it can get the focus).
   */
  bool isPageEnabled( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  bool isPageEnabled( int id ){ return isPageEnabled( page(id) ); }

  /**
   * Enable or disable the tab page (whether it can get the focus or not).
   */
  void setPageEnabled( QWidget*, bool );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageEnabled( int id, bool enabled  ){ setPageEnabled( page(id), enabled ); }
/***********************************************************************/

  /**
   * Returns the widget for a page number.
   */
  QWidget* page( int );

  /**
   * Returns the widget's Id.
   */
  int id( QWidget* );

  /**
   * Removes the tab page from the covered tab bar.
   */
  void removePage( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void removePage( int id ){ removePage( page(id) ); }

  /**
   * Returns the current (visible) tab page.
   */
  QWidget* visiblePage();

  /**
   * Returns the id of the current (visible) tab page.
   */
  int visiblePageId(){ return id( visiblePage() ); }

  /**
   * Sets the given tab page as the active (and visible) one.
   */
  void setVisiblePage( QWidget*, bool allowDisable = false );

  /**
   * Sets the font for the tab pages. 
   * It just calls the appropriate method of its KDockTabBar.
   */
  void setTabFont( const QFont &font );

  /**
   * Returns the font for the tab pages.
   * It just calls the appropriate method of its KDockTabBar.
   */
  QFont tabFont();

  /**
   * Enables or disables the showing of the icons for every tab page.
   */
  void showTabIcon( bool );

  /**
   * Returns if the tab icon is shown.
   */
  bool isShowTabIcon();

  /**
   * Returns the number of inserted tab pages.
   */
  int  pageCount(){ return mainData->count(); }

  /**
   * Returns the first tab page in its KDockTabBar.
   */
  QWidget* getFirstPage();

  /**
   * Returns the last tab page in its KDockTabBar.
   */
  QWidget* getLastPage();

  /**
   * Returns the tab page that is at the right side of the given tab widget.
   */
  QWidget* getNextPage( QWidget* );

  /**
   * Returns the tab page that is at the left side of the given tab widget.
   */
  QWidget* getPrevPage( QWidget* );

  /**
   * Shows its encapsulated tab bar and shows the current tab page.
   */
  virtual void show();

  /**
   * Catches some events that are of interest for this class.
   */
  virtual bool eventFilter( QObject*, QEvent* ); 

public slots:

  /**
   * Enables or disables this and its KDockTabBar.
   */
  virtual void setEnabled( bool );

  /**
   * Sets the visible page by its id.
   */
  void setVisiblePage(int id){ setVisiblePage( page(id) ); }

protected:

  /**
   * Overrides the QWidget::paintEvent. Draws some additional lines.
   */
  void paintEvent(QPaintEvent *);

signals:

  /**
   * Signals that the given widget is about to show.
   */
  void aboutToShow( QWidget* );

  /**
   * Signals that that tab page is selected.
   */
  void pageSelected( QWidget* );

  /**
   * Signals that the right mouse button is pressed on this.
   */
  void tabShowPopup( int, QPoint );

protected slots:

  /**
   * Do some things that should be done when someone has pressed the right mouse button on this.
   */
  void rightButtonPress( int, QPoint );

protected:

  /**
   * 
   */
  KDockTabCtl_Private* findData( QWidget* );

  /**
   *
   */
  KDockTabCtl_Private* findData( int id );

  /**
   * Shows the tab page.
   */
  void showPage( QWidget*, bool allowDisable = false );

  /**
   *
   */
  QWidgetStack* stack;

  /**
   *
   */
  QBoxLayout* layout;

  /**
   *
   */
  QBoxLayout* stack_layout;

  /**
   *
   */
  QList<KDockTabCtl_Private> *mainData;

  /**
   * A pointer to the widget that is the current tab page in the covered tab bar.
   */
  QWidget* currentPage;

  /**
   * The actual tab bar widget that is encapsulated by this control class.
   * Often this manager puts only method calls through and does some additional management
   * functions.
   */
  KDockTabBar* tabs;

  /**
   * The current tab position.
   */
  KDockTabBar::TabPos tabPos;
};

#endif
