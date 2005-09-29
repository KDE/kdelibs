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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcharselect.h"
#include "kcharselect.moc"

#include <qbrush.h>
#include <qcolor.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontdatabase.h>
#include <qhbox.h>
#include <qkeycode.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpen.h>
#include <qregexp.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qvalidator.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>

class KCharSelect::KCharSelectPrivate
{
public:
    QLineEdit *unicodeLine;
};

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
    : QGridView( parent, name ), vFont( _font ), vChr( _chr ),
      vTableNum( _tableNum ), vPos( 0, 0 ), focusItem( _chr ), focusPos( 0, 0 ), d(0)
{
    setBackgroundColor( colorGroup().base() );

    setCellWidth( 20 );
    setCellHeight( 25 );

    setNumCols( 32 );
    setNumRows( 8 );

    repaintContents( false );
    
    setToolTips();

    setFocusPolicy( QWidget::StrongFocus );
    setBackgroundMode( QWidget::NoBackground );
}

//==================================================================
void KCharSelectTable::setFont( const QString &_font )
{
    vFont = _font;
    repaintContents( false );

    setToolTips();
}

//==================================================================
void KCharSelectTable::setChar( const QChar &_chr )
{
    vChr = _chr;
    repaintContents( false );
}

//==================================================================
void KCharSelectTable::setTableNum( int _tableNum )
{
    focusItem = QChar( _tableNum * 256 );

    vTableNum = _tableNum;
    repaintContents( false );

    setToolTips();
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
void KCharSelectTable::resizeEvent( QResizeEvent * e )
{
    const int new_w   = (e->size().width()  - 2*(margin()+frameWidth())) / numCols();
    const int new_h   = (e->size().height() - 2*(margin()+frameWidth())) / numRows();

    if( new_w !=  cellWidth())
        setCellWidth( new_w );
    if( new_h !=  cellHeight())
        setCellHeight( new_h );

    setToolTips();
}

//==================================================================
void KCharSelectTable::paintCell( class QPainter* p, int row, int col )
{
    const int w = cellWidth();
    const int h = cellHeight();
    const int x2 = w - 1;
    const int y2 = h - 1;

    //if( row == 0 && col == 0 ) {
    //    printf("Repaint %d\n", temp++);
    //    fflush( stdout );
    //    }

    QFont font = QFont( vFont );
    font.setPixelSize( int(.7 * h) );

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
	QFontMetrics fm = QFontMetrics( font );
	if( fm.inFont( c ) )
		p->setBrush( QBrush( colorGroup().base() ) );
	else
		p->setBrush( QBrush( colorGroup().button() ) );
	p->setPen( NoPen );
	p->drawRect( 0, 0, w, h );
	p->setPen( colorGroup().text() );
    }

    if ( c == focusItem.unicode() && hasFocus() ) {
	style().drawPrimitive( QStyle::PE_FocusRect, p, QRect( 2, 2, w - 4, h - 4 ), 
			       colorGroup() );
	focusPos = QPoint( col, row );
    }

    p->setFont( font );

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
    const int row = rowAt( e->y() );
    const int col = columnAt( e->x() );
    if ( row >= 0 && row < numRows() && col >= 0 && col < numCols() ) {
	const QPoint oldPos = vPos;

	vPos.setX( col );
	vPos.setY( row );

	vChr = QChar( vTableNum * 256 + numCols() * vPos.y() + vPos.x() );

	const QPoint oldFocus = focusPos;

	focusPos = vPos;
	focusItem = vChr;

	repaintCell( oldFocus.y(), oldFocus.x(), true );
	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( vPos.y(), vPos.x(), true );

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
	emit activated( ' ' );
	emit activated();
	emit highlighted( ' ' );
	emit highlighted();
        break;
    case Key_Enter: case Key_Return: {
	const QPoint oldPos = vPos;

	vPos = focusPos;
	vChr = focusItem;

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( vPos.y(), vPos.x(), true );

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
	const QPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() - 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoRight()
{
    if ( focusPos.x() < numCols()-1 ) {
	const QPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() + 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoUp()
{
    if ( focusPos.y() > 0 ) {
	const QPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() - 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoDown()
{
    if ( focusPos.y() < numRows()-1 ) {
	const QPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() + 1 );

	focusItem = QChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::setToolTips()
{
    const int rowCount = numRows();
    const int colCount = numCols();
    for( int i=0 ; i< rowCount; ++i )
    {
	for( int j=0; j< colCount; ++j )
	{
	    const QRect r( cellWidth()*j, cellHeight()*i, cellWidth(), cellHeight() );
	    QToolTip::remove(this,r);
	    const ushort uni = vTableNum * 256 + numCols()*i + j;
	    QString s;
	    s.sprintf( "%04X", uint( uni ) );
	    QString character; // Character (which sometimes need to be escaped)
            switch ( uni )
	    {
                case 0x3c: character = "&lt;"; break;
                case 0x3e: character = "&gt;"; break;
                case 0x26: character = "&amp;"; break;
                case 0x27: character = "&apos;"; break;
                case 0x22: character = "&quot;"; break;
                default: character = QChar( uni ); break;
	    }
	    QToolTip::add(this, r, i18n( "Character","<qt><font size=\"+4\" face=\"%1\">%2</font><br>Unicode code point: U+%3<br>(In decimal: %4)<br>(Character: %5)</qt>" ).arg( vFont ).arg( character ).arg( s ).arg( uni ).arg( character ) );
	}
    }
}

/******************************************************************/
/* Class: KCharSelect						  */
/******************************************************************/

//==================================================================
KCharSelect::KCharSelect( QWidget *parent, const char *name, const QString &_font, const QChar &_chr, int _tableNum )
  : QVBox( parent, name ), d(new KCharSelectPrivate)
{
    setSpacing( KDialog::spacingHint() );
    QHBox* const bar = new QHBox( this );
    bar->setSpacing( KDialog::spacingHint() );

    QLabel* const lFont = new QLabel( i18n( "Font:" ), bar );
    lFont->resize( lFont->sizeHint() );
    lFont->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lFont->setMaximumWidth( lFont->sizeHint().width() );

    fontCombo = new QComboBox( true, bar );
    fillFontCombo();
    fontCombo->resize( fontCombo->sizeHint() );

    connect( fontCombo, SIGNAL( activated( const QString & ) ), this, SLOT( fontSelected( const QString & ) ) );

    QLabel* const lTable = new QLabel( i18n( "Table:" ), bar );
    lTable->resize( lTable->sizeHint() );
    lTable->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lTable->setMaximumWidth( lTable->sizeHint().width() );

    tableSpinBox = new QSpinBox( 0, 255, 1, bar );
    tableSpinBox->resize( tableSpinBox->sizeHint() );

    connect( tableSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( tableChanged( int ) ) );

    QLabel* const lUnicode = new QLabel( i18n( "&Unicode code point:" ), bar );
    lUnicode->resize( lUnicode->sizeHint() );
    lUnicode->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lUnicode->setMaximumWidth( lUnicode->sizeHint().width() );

    const QRegExp rx( "[a-fA-F0-9]{1,4}" );
    QValidator* const validator = new QRegExpValidator( rx, this );

    d->unicodeLine = new QLineEdit( bar );
    d->unicodeLine->setValidator(validator);
    lUnicode->setBuddy(d->unicodeLine);
    d->unicodeLine->resize( d->unicodeLine->sizeHint() );
    slotUpdateUnicode(_chr);

    connect( d->unicodeLine, SIGNAL( returnPressed() ), this, SLOT( slotUnicodeEntered() ) );

    charTable = new KCharSelectTable( this, name, _font.isEmpty() ? QVBox::font().family() : _font, _chr, _tableNum );
    const QSize sz( charTable->contentsWidth()  +  4 ,
                    charTable->contentsHeight() +  4 );
    charTable->resize( sz );
    //charTable->setMaximumSize( sz );
    charTable->setMinimumSize( sz );
    charTable->setHScrollBarMode( QScrollView::AlwaysOff );
    charTable->setVScrollBarMode( QScrollView::AlwaysOff );

    setFont( _font.isEmpty() ? QVBox::font().family() : _font );
    setTableNum( _tableNum );

    connect( charTable, SIGNAL( highlighted( const QChar & ) ), this, SLOT( slotUpdateUnicode( const QChar & ) ) );
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

KCharSelect::~KCharSelect()
{
    delete d;
}

//==================================================================
QSize KCharSelect::sizeHint() const
{
    return QVBox::sizeHint();
}

//==================================================================
void KCharSelect::setFont( const QString &_font )
{
    const QValueList<QString>::Iterator it = fontList.find( _font );
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
    slotUpdateUnicode( _chr );
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

//==================================================================
void KCharSelect::slotUnicodeEntered( )
{
    const QString s = d->unicodeLine->text();
    if (s.isEmpty())
        return;
    
    bool ok;
    const int uc = s.toInt(&ok, 16);
    if (!ok)
        return;
    
    const int table = uc / 256;
    charTable->setTableNum( table );
    tableSpinBox->setValue(table);
    const QChar ch(uc);
    charTable->setChar( ch );
    charActivated( ch );
}

void KCharSelect::slotUpdateUnicode( const QChar &c )
{
    const int uc = c.unicode();
    QString s;
    s.sprintf("%04X", uc);
    d->unicodeLine->setText(s);
}

void KCharSelectTable::virtual_hook( int, void*)
{ /*BASE::virtual_hook( id, data );*/ }

void KCharSelect::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

