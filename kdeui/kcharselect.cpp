/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kcharselect.h"
#include "kcharselect.moc"

#include <qevent.h>
#include <qfont.h>
#include <qpen.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qkeycode.h>
#include <qfontdatabase.h>

#include <klocale.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kapp.h>

QFontDatabase * KCharSelect::fontDataBase = 0;

void KCharSelect::cleanupFontDatabase()
{
    delete fontDataBase;
    fontDataBase = 0;
}

/******************************************************************/
/* Class: KCharSelectTable					  */
/******************************************************************/

//==================================================================
KCharSelectTable::KCharSelectTable( QWidget *parent, const char *name, const QString &_font,
				    const QChar &_chr, int _tableNum )
    : QTableView( parent, name ), vFont( _font ), vChr( _chr ),
      vTableNum( _tableNum ), vPos( 0, 0 ), focusItem( _chr ), focusPos( 0, 0 )
{
    setBackgroundColor( colorGroup().base() );

    setCellWidth( 20 );
    setCellHeight( 25 );

    setNumCols( 32 );
    setNumRows( 8 );

    repaint( true );

    setFocusPolicy( QWidget::StrongFocus );
    setBackgroundMode( QWidget::NoBackground );
}

//==================================================================
void KCharSelectTable::setFont( const QString &_font )
{
    vFont = _font;
    repaint( true );
}

//==================================================================
void KCharSelectTable::setChar( const QChar &_chr )
{
    vChr = _chr;
    repaint( true );
}

//==================================================================
void KCharSelectTable::setTableNum( int _tableNum )
{
    focusItem = QChar( _tableNum * 256 );

    vTableNum = _tableNum;
    repaint( true );
}

//==================================================================
QSize KCharSelectTable::sizeHint() const
{
    int w = cellWidth();
    int h = cellHeight();

    w *= numCols();
    h *= numRows();

    return QSize( w, h );
}

//==================================================================
void KCharSelectTable::paintCell( class QPainter* p, int row, int col )
{
    int w = cellWidth( col );
    int h = cellHeight( row );
    int x2 = w - 1;
    int y2 = h - 1;

    unsigned short c = vTableNum * 256;
    c += row * numCols();
    c += col;

    if ( c == vChr.unicode() ) {
	p->setBrush( QBrush( colorGroup().highlight() ) );
	p->setPen( NoPen );
	p->drawRect( 0, 0, w, h );
	p->setPen( colorGroup().highlightedText() );
	vPos = QPoint( col, row );
    } else {
	QFontMetrics fm = QFontMetrics( QFont(vFont) );
	if( fm.inFont( c ) )
	    p->setBrush( QBrush( colorGroup().base() ) );
	else
	    p->setBrush( QBrush( colorGroup().button() ) );
	p->setPen( NoPen );
	p->drawRect( 0, 0, w, h );
	p->setPen( colorGroup().text() );
    }

    if ( c == focusItem.unicode() && hasFocus() ) {
	style().drawFocusRect( p, QRect( 2, 2, w - 4, h - 4 ), colorGroup() );
	focusPos = QPoint( col, row );
    }

    p->setFont( QFont( vFont ) );

    p->drawText( 0, 0, x2, y2, AlignHCenter | AlignVCenter, QString( QChar( c ) ) );

    p->setPen( colorGroup().text() );

    p->drawLine( x2, 0, x2, y2 );
    p->drawLine( 0, y2, x2, y2 );

    if ( row == 0 )
	p->drawLine( 0, 0, x2, 0 );
    if ( col == 0 )
	p->drawLine( 0, 0, 0, y2 );
}

//==================================================================
void KCharSelectTable::mouseMoveEvent( QMouseEvent *e )
{
    if ( findRow( e->y() ) != -1 && findCol( e->x() ) != -1 ) {
	QPoint oldPos = vPos;

	vPos.setX( findCol( e->x() ) );
	vPos.setY( findRow( e->y() ) );

	vChr = QChar( vTableNum * 256 + numCols() * vPos.y() + vPos.x() );

	QPoint oldFocus = focusPos;

	focusPos = vPos;
	focusItem = vChr;

	updateCell( oldFocus.y(), oldFocus.x(), true );
	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( vPos.y(), vPos.x(), true );

	emit highlighted( vChr );
	emit highlighted();

	emit focusItemChanged( focusItem );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	gotoLeft();
	break;
    case Key_Right:
	gotoRight();
	break;
    case Key_Up:
	gotoUp();
	break;
    case Key_Down:
	gotoDown();
	break;
    case Key_Next:
	emit tableDown();
	break;
    case Key_Prior:
	emit tableUp();
	break;
    case Key_Space:
    case Key_Enter: case Key_Return: {
	QPoint oldPos = vPos;

	vPos = focusPos;
	vChr = focusItem;

	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( vPos.y(), vPos.x(), true );

	emit activated( vChr );
	emit activated();
	emit highlighted( vChr );
	emit highlighted();
    } break;
    }
}

//==================================================================
void KCharSelectTable::gotoLeft()
{
    if ( focusPos.x() > 0 ) {
	QPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() - 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoRight()
{
    if ( focusPos.x() < 31 ) {
	QPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() + 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoUp()
{
    if ( focusPos.y() > 0 ) {
	QPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() - 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoDown()
{
    if ( focusPos.y() < 7 ) {
	QPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() + 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	updateCell( oldPos.y(), oldPos.x(), true );
	updateCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

/******************************************************************/
/* Class: KCharSelect						  */
/******************************************************************/

//==================================================================
KCharSelect::KCharSelect( QWidget *parent, const char *name, const QString &_font, const QChar &_chr, int _tableNum )
    : QVBox( parent, name )
{
    setSpacing( KDialog::spacingHint() );
    setMargin( KDialog::marginHint() );
    QHBox *bar = new QHBox( this );
    bar->setSpacing( KDialog::spacingHint() );

    QLabel *lFont = new QLabel( i18n( "	 Font:	" ), bar );
    lFont->resize( lFont->sizeHint() );
    lFont->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lFont->setMaximumWidth( lFont->sizeHint().width() );

    fontCombo = new QComboBox( true, bar );
    fillFontCombo();
    fontCombo->resize( fontCombo->sizeHint() );

    connect( fontCombo, SIGNAL( activated( const QString & ) ), this, SLOT( fontSelected( const QString & ) ) );

    QLabel *lTable = new QLabel( i18n( "  Table:  " ), bar );
    lTable->resize( lTable->sizeHint() );
    lTable->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lTable->setMaximumWidth( lTable->sizeHint().width() );

    tableSpinBox = new QSpinBox( 0, 255, 1, bar );
    tableSpinBox->resize( tableSpinBox->sizeHint() );

    connect( tableSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( tableChanged( int ) ) );

    charTable = new KCharSelectTable( this, name, _font.isEmpty() ? QVBox::font().family() : _font, _chr, _tableNum );
    charTable->resize( charTable->sizeHint() );
    charTable->setMaximumSize( charTable->size() );
    charTable->setMinimumSize( charTable->size() );

    setFont( _font.isEmpty() ? QVBox::font().family() : _font );
    setTableNum( _tableNum );

    connect( charTable, SIGNAL( highlighted( const QChar & ) ), this, SLOT( charHighlighted( const QChar & ) ) );
    connect( charTable, SIGNAL( highlighted() ), this, SLOT( charHighlighted() ) );
    connect( charTable, SIGNAL( activated( const QChar & ) ), this, SLOT( charActivated( const QChar & ) ) );
    connect( charTable, SIGNAL( activated() ), this, SLOT( charActivated() ) );
    connect( charTable, SIGNAL( focusItemChanged( const QChar & ) ),
	     this, SLOT( charFocusItemChanged( const QChar & ) ) );
    connect( charTable, SIGNAL( focusItemChanged() ), this, SLOT( charFocusItemChanged() ) );
    connect( charTable, SIGNAL( tableUp() ), this, SLOT( charTableUp() ) );
    connect( charTable, SIGNAL( tableDown() ), this, SLOT( charTableDown() ) );

    connect( charTable, SIGNAL(doubleClicked()),this,SLOT(slotDoubleClicked()));

    setFocusPolicy( QWidget::StrongFocus );
    setFocusProxy( charTable );
}

//==================================================================
QSize KCharSelect::sizeHint() const
{
    return QVBox::sizeHint();
}

//==================================================================
void KCharSelect::setFont( const QString &_font )
{
    QValueList<QString>::Iterator it = fontList.find( _font );
    if ( it != fontList.end() ) {
	QValueList<QString>::Iterator it2 = fontList.begin();
	int pos = 0;
	for ( ; it != it2; ++it2, ++pos);
	fontCombo->setCurrentItem( pos );
	charTable->setFont( _font );
    }
    else
	kdWarning() << "Can't find Font: " << _font << endl;
}

//==================================================================
void KCharSelect::setChar( const QChar &_chr )
{
    charTable->setChar( _chr );
}

//==================================================================
void KCharSelect::setTableNum( int _tableNum )
{
    tableSpinBox->setValue( _tableNum );
    charTable->setTableNum( _tableNum );
}

//==================================================================
void KCharSelect::fillFontCombo()
{
    if ( !fontDataBase ) {
	fontDataBase = new QFontDatabase();
	qAddPostRoutine( cleanupFontDatabase );
    }
    fontList=fontDataBase->families();
    fontCombo->insertStringList( fontList );
}

//==================================================================
void KCharSelect::fontSelected( const QString &_font )
{
    charTable->setFont( _font );
    emit fontChanged( _font );
}

//==================================================================
void KCharSelect::tableChanged( int _value )
{
    charTable->setTableNum( _value );
}
