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

class KDockTabBar : public QWidget
{
  Q_OBJECT
  
  friend KDockTabBarPainter;

public:
  
  KDockTabBar( QWidget * parent = 0, const char * name = 0 );
  ~KDockTabBar();

  enum TabPos
  {
    TAB_TOP,
    TAB_RIGHT
  };

  void setTabPos( TabPos );

  int insertTab( const QString &label, int id = -1 );

  void setPixmap( int id, const QPixmap &pix );

  void setTextColor( int id, const QColor &color );
  const QColor& textColor( int id );

  void removeTab( int );

  int  currentTab(){ return _currentTab; }
  void setCurrentTab( int id, bool allowDisable = false );

  void setTabEnabled( int, bool );
  bool isTabEnabled( int );

  void setTabCaption( int id, const QString &caption );
  QString tabCaption( int id );

  virtual void show();
  virtual void setFont( const QFont & );

  void showTabIcon( bool );
  bool isShowTabIcon(){ return iconShow; }

signals:
  void tabSelected( int );
  void rightButtonPress( int, QPoint );

protected slots:
  void leftClicked();
  void rightClicked();

protected:
  virtual void paintEvent( QPaintEvent* );
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
  
  virtual void mousePressEvent ( QMouseEvent * );
  
  virtual void mouseReleaseEvent ( QMouseEvent * );

  virtual void resizeEvent( QResizeEvent * );
  
  virtual void paintEvent( QPaintEvent * );
};

class KDockTabCtl : public QWidget
{
  Q_OBJECT
  
public:
  
  KDockTabCtl( QWidget *parent = 0, const char *name = 0 );
  ~KDockTabCtl();

  void setTabPos( KDockTabBar::TabPos );
  
  KDockTabBar::TabPos getTabPos(){ return tabPos; }

  int insertPage( QWidget *, const QString &label, int id = -1 );

  void setPageCaption( QWidget*, const QString & );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageCaption( int id, const QString &caption ){ setPageCaption( page(id), caption ); }

  QString pageCaption( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  QString pageCaption( int id ){ return pageCaption( page(id) ); }
  
  void setPixmap( QWidget* , const QPixmap &pix );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPixmap( int id, const QPixmap &pix ){ setPixmap( page(id), pix ); }

  void setTabTextColor( QWidget*, const QColor &color );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setTabTextColor( int id, const QColor &color ){ setTabTextColor( page(id), color ); }

  const QColor& tabTextColor( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  const QColor& tabTextColor( int id ){ return tabTextColor( page(id) ); }
  
  bool isPageEnabled( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  bool isPageEnabled( int id ){ return isPageEnabled( page(id) ); }

  void setPageEnabled( QWidget*, bool );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void setPageEnabled( int id, bool enabled  ){ setPageEnabled( page(id), enabled ); }
/***********************************************************************/

  QWidget* page( int );
  int id( QWidget* );

  void removePage( QWidget* );

  /** This is an overloaded member function, provided for convenience.
   *  It differs from the above function only in what argument(s) it accepts. 
   */
  void removePage( int id ){ removePage( page(id) ); }

  QWidget* visiblePage();
  int visiblePageId(){ return id( visiblePage() ); }

  void setVisiblePage( QWidget*, bool allowDisable = false );

  void setTabFont( const QFont &font );
  QFont tabFont();

  void showTabIcon( bool );
  bool isShowTabIcon();

  int  pageCount(){ return mainData->count(); }
  QWidget* getFirstPage();
  QWidget* getLastPage();
  QWidget* getNextPage( QWidget* );
  QWidget* getPrevPage( QWidget* );

  virtual void show();
  virtual bool eventFilter( QObject*, QEvent* ); 

public slots:
  virtual void setEnabled( bool );
  void setVisiblePage(int id){ setVisiblePage( page(id) ); }

protected:
  void paintEvent(QPaintEvent *);

signals:
  void aboutToShow( QWidget* );
  void pageSelected( QWidget* );
  void tabShowPopup( int, QPoint );

protected slots:
  void rightButtonPress( int, QPoint );

protected:
  KDockTabCtl_Private* findData( QWidget* );
  KDockTabCtl_Private* findData( int id );
  void showPage( QWidget*, bool allowDisable = false );

  QWidgetStack* stack;
  QBoxLayout* layout;
  QBoxLayout* stack_layout;

  QList<KDockTabCtl_Private> *mainData;
  QWidget* currentPage;
  KDockTabBar* tabs;
  KDockTabBar::TabPos tabPos;
};

#endif
