/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

/**
 * KTabCtl provides a universal tab control. It is in no ways limited to dialogs and
 * can be used for whatever you want. It has no buttons or any other stuff.
 *
 * However, this is based on the original QTabDialog.
 */

#ifdef HAVE_LIMITS_H
#include <limits.h>
#else
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif
#endif

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>
#include <QtGui/QResizeEvent>
#include <QtGui/QTabBar>

#include "ktabctl.h"

class KTabCtl::Private
{
  public:
    Private( KTabCtl *parent )
      : mParent( parent ),
        mShowBorder( true )
    {
    }

    void showTab( int index );
    void setSizes();
    QRect getChildRect() const;

    KTabCtl *mParent;
    QTabBar *mTabs;
    QVector<QWidget*> mPages;
    bool mShowBorder;

};

/*
 * show a single page, depending on the selected tab
 * emit tabSelected(new_pagenumber) BEFORE the page is shown
 */
void KTabCtl::Private::showTab( int index )
{
  for ( int j = 0; j < mPages.size(); j++ ) {
    if ( j != index ) {
      mPages[ j ]->hide();
    }
  }

  if ( index < mPages.size() )
    emit mParent->tabSelected( index );

  if ( mPages.size() >= 2 )
    mPages[ index ]->raise();

  mPages[ index ]->setGeometry( getChildRect() );
  mPages[ index ]->show();
}

void KTabCtl::Private::setSizes()
{
  QSize min( mTabs->sizeHint() ); /* the minimum required size for the tabbar */
  mTabs->resize( min );           /* make sure that the tabbar does not require more space than actually needed. */


  QSize max( INT_MAX, INT_MAX );
  //int th = min.height();       /* the height of the tabbar itself (without mPages and stuff) */

  for ( int i = 0; i < mPages.size(); i++ ) {
    /*
     * check the actual minimum and maximum sizes
     */

    if ( mPages[ i ]->maximumSize().height() < max.height() )
      max.setHeight( mPages[ i ]->maximumSize().height() );

    if ( mPages[ i ]->maximumSize().width() < max.width() )
      max.setWidth( mPages[ i ]->maximumSize().width() );

    if ( mPages[ i ]->minimumSize().height() > min.height() )
      min.setHeight( mPages[ i ]->minimumSize().height() );

    if ( mPages[ i ]->minimumSize().width() > min.width() )
      min.setWidth( mPages[ i ]->minimumSize().width() );
  }

  // BL: min and max are sizes of children, not tabcontrol
  // min.setHeight(min.height() + th);

  if ( max.width() < min.width() )
    max.setWidth( min.width() );

  if ( max.height() < min.height() )
    max.setHeight(min.height());

  /*
   * now, apply the calculated size values to all of the mPages
   */

  for ( int i=0; i<mPages.size(); i++ ) {
    mPages[ i ]->setMinimumSize( min );
    mPages[ i ]->setMaximumSize( max );
  }

  // BL: set minimum size of tabcontrol
  mParent->setMinimumSize( min.width() + 4, min.height() + mTabs->height() + 4 );

  /*
   * generate a resizeEvent, if we're visible
   */

  if ( mParent->isVisible() ) {
    QResizeEvent event( mParent->size(), mParent->size() );
    mParent->resizeEvent( &event );
  }
}

/*
 * return the client rect. This is the maximum size for any child
 * widget (page).
 */
QRect KTabCtl::Private::getChildRect() const
{
  if ( ( mTabs->shape() == QTabBar:: RoundedSouth ) ||
       ( mTabs->shape() == QTabBar:: TriangularSouth ) ) {
    return QRect( 2, 1, mParent->width() - 4, mParent->height() - mTabs->height() - 4 );
  } else {
    return QRect( 2, mTabs->height() + 1, mParent->width() - 4, mParent->height() - mTabs->height() - 4 );
  }
}

KTabCtl::KTabCtl( QWidget *parent )
  : QWidget( parent ),
    d( new Private( this ) )
{
  d->mTabs = new QTabBar( this );
  connect( d->mTabs, SIGNAL( selected( int ) ),
           this, SLOT( showTab( int ) ) );

  d->mTabs->move( 2, 1 );
}

KTabCtl::~KTabCtl()
{
  delete d;
}

void KTabCtl::resizeEvent( QResizeEvent* )
{
  if ( d->mTabs ) {
    int i;

    QRect rect = d->getChildRect();
    for ( i = 0; i < (int)d->mPages.size(); ++i )
      d->mPages[ i ]->setGeometry( rect );

    if ( ( d->mTabs->shape() == QTabBar:: RoundedSouth ) ||
         ( d->mTabs->shape() == QTabBar:: TriangularSouth ) )
      d->mTabs->move( 0, height() - d->mTabs->height() - 4 );
  }
}

void KTabCtl::setFont( const QFont &_font )
{
  QFont font( _font );
  font.setWeight( QFont::Light );

  QWidget::setFont( font );

  d->setSizes();
}

void KTabCtl::setTabFont( const QFont &font )
{
  d->mTabs->setFont( font );

  d->setSizes();
}

void KTabCtl::show()
{
  if ( isVisible() )
    return;

  d->setSizes();

  for ( int i = 0; i < d->mPages.size(); i++ )
    d->mPages[ i ]->hide();

  QResizeEvent event( size(), size() );
  resizeEvent( &event );

  QWidget::show();
}

bool KTabCtl::isTabEnabled( const QString &name )
{
  for ( int i = 0; i < d->mPages.size(); i++ )
    if ( d->mPages[ i ]->objectName() == name )
      return d->mTabs->isTabEnabled( i );

  return false;
}

void KTabCtl::setTabEnabled( const QString &name, bool state )
{
  if ( name.isEmpty() )
    return;

  for ( int i = 0; i < d->mPages.size(); i++ )
    if ( d->mPages[ i ]->objectName() == name )
      d->mTabs->setTabEnabled( i, state );
}

void KTabCtl::showBorder( bool state )
{
  d->mShowBorder = state;
}

void KTabCtl::setShape( QTabBar::Shape shape )
{
  d->mTabs->setShape( shape );
}

QSize KTabCtl::sizeHint() const
{
  /* desired size of the tabbar */
  QSize hint( d->mTabs->sizeHint() );

  /* overall desired size of all mPages */
  QSize pageHint;
  for ( int i = 0; i < d->mPages.size(); i++ ) {
    QSize sizeI( d->mPages[ i ]->sizeHint() );

    if ( sizeI.isValid() ) {
      /* only mPages with valid size are used */
      if ( sizeI.width() > pageHint.width() )
        pageHint.setWidth( sizeI.width() );

      if ( sizeI.height() > pageHint.height() )
        pageHint.setHeight( sizeI.height() );
    }
  }

  if ( pageHint.isValid() ) {
    /* use maximum of width of tabbar and mPages */
    if ( pageHint.width() > hint.width() )
      hint.setWidth( pageHint.width() );

    /* heights must just be added */
    hint.setHeight( hint.height() + pageHint.height() );

    /* 1999-09-18: Espen Sand
       I cannot get the size to be correct unless the total
       border size is included: ie 2*2 pixels.
    */
    return (hint + QSize( 4, 4 ));
  }

  /*
   * If not at least a one page has a valid sizeHint we have to return
   * an invalid size as well.
   */
  return pageHint;
}

void KTabCtl::paintEvent( QPaintEvent* )
{
  if ( !d->mTabs )
    return;

  if ( !d->mShowBorder )
    return;

  QPainter p;
  p.begin( this );

  int y0 = d->getChildRect().top() - 1;
  int y1 = d->getChildRect().bottom() + 2;
  int x1 = d->getChildRect().right() + 2;
  int x0 = d->getChildRect().left() - 1;

  p.setPen( palette().color( QPalette::Light ) );
  p.drawLine( x0, y0 - 1, x1 - 1, y0 - 1 );      /* 1st top line */

  p.setPen( palette().color( QPalette::Midlight ) );
  p.drawLine( x0, y0, x1 - 1, y0 );              /* 2nd top line */

  p.setPen( palette().color( QPalette::Light ) );
  p.drawLine( x0, y0 + 1, x0, y1 );              /* left line */

  p.setPen( Qt::black );
  p.drawLine( x1, y1, x0, y1 );                  /* bottom line */
  p.drawLine( x1, y1 - 1, x1, y0 );

  p.setPen( palette().color( QPalette::Dark ) );
  p.drawLine( x0 + 1, y1 - 1, x1 - 1, y1 - 1 );  /* bottom */
  p.drawLine( x1 - 1, y1 - 2, x1 - 1, y0 + 1 );
  p.end();
}

/*
 * add a tab to the control. This tab will manage the given Widget w.
 * in most cases, w will be a QWidget and will only act as parent for the
 * actual widgets on this page
 * NOTE: w is not required to be of class QWidget, but expect strange results with
 * other types of widgets
 */
void KTabCtl::addTab( QWidget *widget, const QString& name )
{
  int id = d->mTabs->addTab( name );   /* add the tab itself to the tabbar */
  d->mTabs->setTabEnabled( id, true );

  if ( id == (int)d->mPages.size() ) {
    d->mPages.resize( id + 1 );
    d->mPages[ id ] = widget;          /* remember the widget to manage by this tab */
  }

  // BL: compute sizes
  d->setSizes();
}

#include "ktabctl.moc"
