/* This file is part of the KDE libraries
   Copyright (C) 2000 Max Judin <novaprint@mtu-net.ru>
   Copyright (C) 2000 Falk Brettschneider <gigafalk@yahoo.com>

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

/*
   activities:
   -----------
   05/2000                   : bufixes, patches by Christoph Cullmann <crossfire@babylon2k.de>,
                               Joseph Wenninger <jowenn@bigfoot.com> and Falk Brettschneider
   03/2000 - 04/2000         : bugfixes, patches by Falk Brettschneider
   03/2000                   : now maintained by Falk Brettschneider <falk@kdevelop.org>
   03/2000                   : class documentation added by Falk Brettschneider <gigafalk@yahoo.com>
   10/1999 - 05/2000         : programmed by Max Judin <novaprint@mtu-net.ru>
   3/2000 - 5/2000           : patches and bugfixes by Falk Brettschneider <gigafalk@yahoo.com>
   
   C++ classes in this file:
   -------------------------
   - KDockTabBar           - minor class (but the actual tab bar)
   - KDockTabCtl           - helper class for a tab-bar mode of centered docked KDockWidgets
   - KDockTabBarPainter    - minor class (paints the tab header)
   
   IMPORTANT Note: This file compiles also in Qt-only mode by using the NO_KDE2 precompiler definition!
*/

#ifndef KDOCKTABCTL_H
#define KDOCKTABCTL_H

#include <qwidget.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qtooltip.h>

#ifndef NO_KDE2
#undef  EXPORT_DOCKCLASS
#define EXPORT_DOCKCLASS
#else
#include "exportdockclass.h"
#endif

class QWidgetStack;
class QBoxLayout;
class QPushButton;

struct KDockTabBar_PrivateStruct;
class KDockTabBarPainter;
class KDockDynTabBarToolTip;

/**
 * Actually an internal struct. But it is used in a QList of KDockTabCtl (see below).
 * So we cannot use class forward declaration since QList demands to know the destructor method.
 */
struct KDockTabCtl_PrivateStruct
{
  KDockTabCtl_PrivateStruct( QWidget* _widget, int _id )
  { widget = _widget; id = _id; enabled = true; }
  ~KDockTabCtl_PrivateStruct(){;}

  QWidget* widget;
  int      id;
  bool     enabled;
};

/**
 * The actual tab bar for dockwidgets  (and member of the dockwidget class set). 
 *
 * This class is covered by its manager, @ref KDockTabCtl.
 * It provides some set/get methods and managment
 * methods like insert, remove, activate for tab widgets.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class EXPORT_DOCKCLASS KDockTabBar : public QWidget
{
  Q_OBJECT
  
  friend class KDockTabBarPainter;

public:
  
  /**
   * Constructs a tab bar widget.
   * The parent and name argument are sent to the QWidget constructor.
   */
  KDockTabBar( QWidget * parent = 0, const char * name = 0 );

  /**
   * Destructs this.
   */
  ~KDockTabBar();

  /**
   * For clearer setting (or asking for) the current tab page position.
   */
  enum TabPos
  {
    TAB_TOP,
    TAB_RIGHT
  };

  /**
   * Set the position to tabpos and repaints this.
   *
   * @param tabpos Either @p TAB_TOP or @p TAB_RIGHT, just where you
   *  want it to be
   */
  void setTabPos( TabPos tabpos);

  /**
   * Insert a new empty tab page to the tab bar.
   *
   * If @p id is not specified, the tab is simply added. Otherwise
   * it's inserted at the specified position.
   *
   * @param label The title in the tab page header.
   * @param id Suggest an identification number for access operations but it will be shifted to the next free value.
   * @param index The tab page position (default=-1 means append)
   * @return The new allocated id.
   */
  int insertTab( const QString &label, int id = -1, int index = -1 );

  /**
   * Set an icon for the tab page with that @p  id.
   *
   * @param id The identification number of that desired page.
   * @param pix The new pixmap.
   */
  void setPixmap( int id, const QPixmap &pix );

  /**
   * Set a tooltip for the tab page with that @p id.
   *
   * @param id The identification number of that desired page.
   * @param toolTipStr The new tooltip.
   */
  void setToolTip( int id, const QString &toolTipStr );

  /**
   * Set the text colour for the tab page with that @p id.
   *
   * @param id The identification number of that desired page.
   * @param color The new text colour.
   */
  void setTextColor( int id, const QColor &color );

  /**
   * Retrieve the text colour for the tab page with that @p id.
   *
   * @param id the identification number of that desired tab page
   * @return the current text colour of that tab page
   */
  const QColor& textColor( int id ) const;

  /**
   * Removes the tab page with that @p id.
   *
   * @param id The identification number of that desired page.
   */
  void removeTab( int id);

  /**
   * Retrieve the current tab page.
   *
   * @return The @p id of the tab page.
   */
  int  currentTab() const { return _currentTab; }

  /**
   * Set the current tab page to the page with that @p id.
   *
   * @param id The identification number of that desired page.
   * @param allowDisable Disables the tab page.
   */
  void setCurrentTab( int id, bool allowDisable = false );

  /**
   * Enable or disable the tab page with that @p id.
   */
  void setTabEnabled( int id, bool e);

  /**
   * Returns if the tab page with that @p id is enabled or disabled.
   */
  bool isTabEnabled( int id) const;

  /**
   * Set the title of the tab page with that @p id.
   *
   * @param id The identification number of that desired page.
   * @param caption A string for the title.
   */
  void setTabCaption( int id, const QString &caption );

  /**
   * Retrieve the title of the tab page with that @p id.
   *
   * @param id the identification number of that desired page
   */
  QString tabCaption( int id ) const;

  /**
   * Calls @ref QWidget::show() and @ref showPage() for the current tab
   *  page, additionally.
   */
  virtual void show();

  /**
   * Set the font of @p this.
   */
  virtual void setFont( const QFont & );

  /**
   * Show or hide the icons for the tab pages in the header.
   */
  void showTabIcon( bool );

  /**
   * Returns if the icons for the tab pages are shown in the header.
   */
  bool isShowTabIcon() const { return iconShow; }

signals:

  /**
   * Signals that a tab page with that id is selected.
   *
   * @param id the identification number of that desired page
   */
  void tabSelected( int id);

  /**
   * Signals that the right mouse buttons is pressed on the tab page
   * with that @pid.
   *
   * @param id The identification number of that desired page.
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
   * Handles paint events for this widgets
   * Reimplemented from QWidget
   */
  virtual void paintEvent( QPaintEvent* );

  /**
   * Handles resize events for this widgets
   * Reimplemented from QWidget
   */
  virtual void resizeEvent( QResizeEvent* );

  /**
   * Handles key press events for this widgets
   * Reimplemented from QWidget
   */
  virtual void keyPressEvent( QKeyEvent* );

private:
  /** For internal use */
  void setButtonPixmap();
  /** For internal use */
  void updateHeight();

  /** For internal use */
  KDockTabBar_PrivateStruct* findData( int id ) const;
  /** For internal use */
  int tabsWidth();
  /** For internal use */
  void tabsRecreate();

  TabPos tabPos;
  KDockTabBarPainter* barPainter;
  QList<KDockTabBar_PrivateStruct> *mainData;
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

  class KDockTabBarPrivate;
	KDockTabBarPrivate *d;
};

/**
 * The draw helper for the @ref KDockTabBar (and member of the dockwidget class set).
 * Minor importance for application programmers who uses the dockwidget class set.
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class EXPORT_DOCKCLASS KDockTabBarPainter : public QWidget
{
  Q_OBJECT
  friend class KDockTabBar;

public:
  /** returns the tooltip string of the tab at this position */
  QString tip( const QPoint & p ) const;

  /** returns the tab rectangle at this position */
  QRect findBarRectByPos( int x, int y) const;

private:

  KDockTabBarPainter( KDockTabBar* parent );
  ~KDockTabBarPainter();

  void drawBuffer();
  int findBarByPos( int x, int y ) const;

  QPixmap* buffer;
  int mousePressTab;
  int delta;
  KDockDynTabBarToolTip* dynToolTip;

  class KDockTabBarPainterPrivate;
	KDockTabBarPainterPrivate *d;

protected:

  /**
   * Handles mouse press events for this widgets
   * Reimplemented from QWidget
   */
  virtual void mousePressEvent ( QMouseEvent * );

  /**
   * Handles mouse release events for this widgets
   * Reimplemented from QWidget
   */
  virtual void mouseReleaseEvent ( QMouseEvent * );

  /**
   * Handles resize events for this widgets
   * Reimplemented from QWidget
   */
  virtual void resizeEvent( QResizeEvent * );

  /**
   * Handles paint events for this widgets
   * Reimplemented from QWidget
   */
  virtual void paintEvent( QPaintEvent * );
};

/**
 * A manager for a single @ref KDockTabBar  (and member of the dockwidget class set).
 * The @ref KDockWidget class ever use this class instead of accessing the KDockTabBar directly.
 *
 * For some reasons it's more practical for the Dockwidget class set than @ref QTabBar or @ref KTabBar .
 *
 * @author Max Judin (documentation: Falk Brettschneider).
 */
class EXPORT_DOCKCLASS KDockTabCtl : public QWidget
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
   * Calls @ref KDockTabBar::setTabPos of its embedded tab bar but does layout actions, additionally.
   *
   */
  virtual void setTabPos( KDockTabBar::TabPos );
  
  /**
   * Returns the current tab position of its docktabbar.
   */
  KDockTabBar::TabPos getTabPos() const { return tabPos; }

  /**
   * Inserts a new tab page in the encapsulated docktabbar.
   * Mainly it calls @ref KDockTabBar::insertTab and does additional management operations.
   *
   * @param w the widget that is inserted
   * @param label the title for the caption bar of the tab page
   * @param id suggest an identification number for access operations but it will be shifted to the next free value
   * @param index the tab page position (default=-1 means append)
   * @return the id of the new inserted page
   */
  int insertPage( QWidget * w, const QString &label, int id = -1, int index = -1 );

  /**
   * Sets the title of the tab page.
   * Mainly it calls @ref KDockTabBar::setPageCaption but calls it internally by the page id.
   * 
   * @param w the widget we want to give a new caption
   * @param s well, what the heck could that be? ;-)
   */
  void setPageCaption( QWidget* w, const QString &s );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageCaption( int id, const QString &caption ){ setPageCaption( page(id), caption ); }

  /**
   * Returns the title of the tab page.
   * Mainly it calls @ref KDockTabBar::setPageCaption but calls it internally by the page id.
   * 
   * @param w the widget for that we want to know its caption
   * @return a string containing the title of the page
   */
  QString pageCaption( QWidget* w) const;

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  QString pageCaption( int id ) const { return pageCaption( page(id) ); }
  
  void captionAutoSet( bool autoSet ){ m_autoSetCaption = autoSet; }
  bool isCaptionAutoSet() const { return m_autoSetCaption; }

  /**
   * Sets an icon for the tab page (shown in the tab header).
   * 
   * @param w the widget we want to give a new icon
   * @param pix the new icon
   */
  void setPixmap( QWidget* w, const QPixmap &pix );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPixmap( int id, const QPixmap &pix ){ setPixmap( page(id), pix ); }

  /**
   * Sets a toolTip for the tab page (shown in the tab header).
   *
   * @param w the widget we want to give the tooltip
   * @param toolTipStr the new tooltip string
   */
  void setToolTip( QWidget* w, const QString &toolTipStr );

  /**
   * Sets the text colour of the tab page.
   * Mainly it calls @ref KDockTabBar::setTextColor but calls it internally by the page id.
   * 
   * @param w the widget we want to give a new text colour
   * @param color the colour of the tab header
   */
  void setTabTextColor( QWidget*, const QColor &color );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setTabTextColor( int id, const QColor &color ){ setTabTextColor( page(id), color ); }

  /**
   * Returns the text colour of the tab page.
   * Mainly it calls @ref KDockTabBar::textColor but calls it internally by the page id.
   * 
   * @param w the widget we want to ask for its text colour
   */
  const QColor& tabTextColor( QWidget* w) const;

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  const QColor& tabTextColor( int id ) const { return tabTextColor( page(id) ); }
  
  /**
   * Returns if the tab page is enabled (if it can get the focus).
   * 
   * @param w the widget we want to ask if its page is enabled
   */
  bool isPageEnabled( QWidget* w) const;

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  bool isPageEnabled( int id ) const { return isPageEnabled( page(id) ); }

  /**
   * Enable or disable the tab page (whether it can get the focus or not).
   * Mainly it calls @ref KDockTabBar::setPageEnabled but calls it internally by the page id.
   * 
   * @param w the widget that should be enabled
   * @param e boolean value whether enabled or disabled
   */
  void setPageEnabled( QWidget* w, bool e);

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageEnabled( int id, bool enabled  ){ setPageEnabled( page(id), enabled ); }
/***********************************************************************/

  /**
   * Returns the widget that embedded in a page.
   * 
   * @param id the identification number of the page
   */
  QWidget* page( int id) const;

  /**
   * Returns the widget's Id.
   *
   * @param w the widget that should return its id
   */
  int id( QWidget* w) const;

  /**
   * Returns the widget's index. (tab page position)
   *
   * @param w the widget that should return its index
   */
  int index( QWidget* w) const;

  /**
   * Removes the tab page from the covered tab bar.
   * Mainly it calls @ref KDockTabBar::removeTab but calls it internally by the page id.
   * 
   * @param w the tab page that should be removed
   */
  void removePage( QWidget* w);

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void removePage( int id ){ removePage( page(id) ); }

  /**
   * Returns the current (visible) tab page.
   */
  QWidget* visiblePage() const;

  /**
   * Returns the id of the current (visible) tab page.
   */
  int visiblePageId() const { return id( visiblePage() ); }

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
   * It simply calls @ref KDockTabBar::tabFont .
   */
  QFont tabFont() const;

  /**
   * Enables or disables the showing of the icons for every tab page.
   * It simply calls @ref KDockTabBar::showTabIcon .
   */
  void showTabIcon( bool );

  /**
   * Returns if the tab icon is shown.
   * It simply calls @ref KDockTabBar::isShowTabIcon .
   */
  bool isShowTabIcon() const;

  /**
   * Returns the number of inserted tab pages.
   */
  int  pageCount() const { return mainData->count(); }

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
   * Handles paint events for this widgets
   * Reimplemented from QWidget
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
   * Returns the appropriate data for the widget from the parameter list.
   */
  KDockTabCtl_PrivateStruct* findData( QWidget* ) const;

  /**
   * Returns the appropriate data for the widget from the parameter list described by its id.
   */
  KDockTabCtl_PrivateStruct* findData( int id ) const;

  /**
   * Shows the tab page.
   */
  void showPage( QWidget*, bool allowDisable = false );

  /**
   * a data structure that contains all embedded widgets.
   */
  QWidgetStack* stack;

  /**
   * The layout manager for automatic positioning and resizing of the embedded tab pages.
   */
  QBoxLayout* layout;

  /**
   * The layout manager for the widget stack.
   */
  QBoxLayout* stack_layout;

  /**
   * An internal list for managing the tab pages. It stores data items for every tab page in the tab bar.
   * An data item is of type KDockTabCtl_PrivateStruct (a struct) that contains the widget, its id and information
   * about whether it is enabled or disabled.
   */
  QList<KDockTabCtl_PrivateStruct> *mainData;

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

  bool m_autoSetCaption;

private:
  class KDockTabCtlPrivate;
  KDockTabCtlPrivate *d;
};

#endif
