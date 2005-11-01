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

#include "kcharselect_p.h"
#include "kcharselect_p.moc"

#include <qbrush.h>
#include <qcolor.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontdatabase.h>

#include <qnamespace.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpen.h>
#include <qregexp.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <q3valuelist.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <kvbox.h>
#include <qheaderview.h>


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
    : QTableView( parent), vFont( _font ), vChr( _chr ),
      vTableNum( _tableNum ), vPos( 0, 0 ), focusItem( _chr ), focusPos( 0, 0 ),m_model(0), d(0)
{
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setBackgroundColor( colorGroup().base() );
    verticalHeader()->setVisible(false);
    verticalHeader()->setResizeMode(QHeaderView::Custom);
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setResizeMode(QHeaderView::Custom);
    setTableNum(_tableNum);
/*
    setCellWidth( 20 );
    setCellHeight( 25 );
*/
    //repaintContents( false );
    
    setFocusPolicy( Qt::StrongFocus );
    //setBackgroundMode( Qt::NoBackground );
}

//==================================================================
void KCharSelectTable::setFont( const QString &_font )
{
    vFont = _font;
    if (m_model) m_model->setFont(_font);
}

//==================================================================
void KCharSelectTable::setChar( const QChar &_chr )
{
    //const uint short chr=_chr;
    //if (chr)
    vChr = _chr;
#ifdef __GNUC__
    #warning fixme //repaintContents( false );
#endif
}

//==================================================================
void KCharSelectTable::setTableNum( int _tableNum )
{
    focusItem = QChar( _tableNum * 256 );

    vTableNum = _tableNum;
    
    KCharSelectItemModel *m=m_model;
    m_model=new KCharSelectItemModel(_tableNum,vFont,this);
    setModel(m_model);
    QItemSelectionModel *selectionModel=new QItemSelectionModel(m_model);
    setSelectionModel(selectionModel);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(selectionModel,SIGNAL(currentChanged ( const QModelIndex & ,const QModelIndex &)), this, SLOT(slotCurrentChanged ( const QModelIndex &, const QModelIndex &)));
    delete m; // this should hopefully delete aold selection models too, since it is the parent of them (didn't track, if there are setParent calls somewhere. Check that (jowenn)
#ifdef __GNUC__
    #warning fixme //repaintContents( false );
#endif
}

//==================================================================
void KCharSelectTable::slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
	if (!m_model) return;
	focusItem = m_model->data(current,KCharSelectItemModel::CharacterRole).toChar();
	emit focusItemChanged( focusItem );
	emit focusItemChanged();
}


//==================================================================
QSize KCharSelectTable::sizeHint() const
{
    if (!model()) return QTableView::sizeHint();
    int w = columnWidth(0);
    int h = rowHeight(0);

    w *= model()->columnCount(QModelIndex());
    h *= model()->rowCount(QModelIndex());

    return QSize( w, h );
}

//==================================================================
void KCharSelectTable::resizeEvent( QResizeEvent * e )
{
    if (!model()) return;
    const int new_w   = (e->size().width()  /*- 2*(margin()+frameWidth())*/) / model()->columnCount(QModelIndex());
    const int new_h   = (e->size().height() /*- 2*(margin()+frameWidth())*/) / model()->rowCount(QModelIndex());
    const int columns=model()->columnCount(QModelIndex());
    const int rows=model()->rowCount(QModelIndex());
    setUpdatesEnabled(false);
    QHeaderView* hv=horizontalHeader();
    for (int i=0;i<columns;i++) {
    	qDebug("Setting new width");
    	hv->resizeSection(i,new_w);
    }
    hv=verticalHeader();
    for (int i=0;i<rows;i++) {
    	qDebug("Setting new height");
    	hv->resizeSection(i,new_h);
    }

    setUpdatesEnabled(true);
    QTableView::resizeEvent(e);
}

#ifdef __GNUC__
#warning fix all below
#endif
//==================================================================
void KCharSelectTable::mouseMoveEvent( QMouseEvent *e )
{
    if (!model()) return;
    const int numRows=model()->rowCount(QModelIndex());
    const int numCols=model()->columnCount(QModelIndex());
    const int row = rowAt( e->y() );
    const int col = columnAt( e->x() );
    if ( row >= 0 && row < numRows && col >= 0 && col < numCols ) {
	const QPoint oldPos = vPos;

	vPos.setX( col );
	vPos.setY( row );

	vChr = QChar( vTableNum * 256 + numCols * vPos.y() + vPos.x() );

	const QPoint oldFocus = focusPos;

	focusPos = vPos;
	focusItem = vChr;

#ifdef __GNUC__
#warning fixme
#endif
/*	
	repaintCell( oldFocus.y(), oldFocus.x(), true );
	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( vPos.y(), vPos.x(), true );
*/
	/*emit highlighted( vChr );
	emit highlighted();*/

	emit focusItemChanged( focusItem );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::keyPressEvent( QKeyEvent *e )
{
    if (m_model)
    switch ( e->key() ) {
    case Qt::Key_PageDown:
        emit tableDown();
	return;
        break;
    case Qt::Key_PageUp:
        emit tableUp();
	return;
        break;
    case Qt::Key_Space:
	emit activated( ' ' );
	emit activated();
	return;
        break;
    case Qt::Key_Enter: case Qt::Key_Return: {
    	if (!currentIndex().isValid()) return;
            const QPoint oldPos = vPos;

	    vPos = focusPos;
	    vChr = focusItem;

	    emit activated( m_model->data(currentIndex(),KCharSelectItemModel::CharacterRole).toChar());
	    emit activated();
        }
	return;
	break;
    }
    QTableView::keyPressEvent(e);
}


/******************************************************************/
/* Class: KCharSelect						  */
/******************************************************************/

//==================================================================
KCharSelect::KCharSelect( QWidget *parent, const char *name, const QString &_font, const QChar &_chr, int _tableNum )
  : KVBox( parent/*, name*/ ), d(new KCharSelectPrivate)
{
    setSpacing( KDialog::spacingHint() );
    KHBox* const bar = new KHBox( this );
    bar->setSpacing( KDialog::spacingHint() );

    QLabel* const lFont = new QLabel( i18n( "Font:" ), bar );
    lFont->resize( lFont->sizeHint() );
    lFont->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lFont->setMaximumWidth( lFont->sizeHint().width() );

    fontCombo = new QComboBox(bar );
    fontCombo->setEditable(true);
    fillFontCombo();
    fontCombo->resize( fontCombo->sizeHint() );

    connect( fontCombo, SIGNAL( activated( const QString & ) ), this, SLOT( fontSelected( const QString & ) ) );

    QLabel* const lTable = new QLabel( i18n( "Table:" ), bar );
    lTable->resize( lTable->sizeHint() );
    lTable->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lTable->setMaximumWidth( lTable->sizeHint().width() );

    tableSpinBox = new QSpinBox( bar );
    tableSpinBox->setRange(0,255);
    tableSpinBox->setSingleStep(1);
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

    charTable = new KCharSelectTable( this, name, _font.isEmpty() ? KVBox::font().family() : _font, _chr, _tableNum );

    const QSize sz( 200,
                    200 );    
#ifdef __GNUC__
    #warning fixme
#endif
    #if 0    
    const QSize sz( charTable->contentsWidth()  +  4 ,
                    charTable->contentsHeight() +  4 );
    #endif
    charTable->resize( sz );
    //charTable->setMaximumSize( sz );
    charTable->setMinimumSize( sz );
#ifdef __GNUC__
    #warning fixme
#endif
/*    charTable->setHScrollBarMode( Q3ScrollView::AlwaysOff );
    charTable->setVScrollBarMode( Q3ScrollView::AlwaysOff );*/

    setFont( _font.isEmpty() ? KVBox::font().family() : _font );
    setTableNum( _tableNum );

    connect( charTable, SIGNAL( focusItemChanged( const QChar & ) ), this, SLOT( slotUpdateUnicode( const QChar & ) ) );
    connect( charTable, SIGNAL( focusItemChanged( const QChar & ) ), this, SLOT( charHighlighted( const QChar & ) ) );
    connect( charTable, SIGNAL( focusItemChanged() ), this, SLOT( charHighlighted() ) );
    connect( charTable, SIGNAL( activated( const QChar & ) ), this, SLOT( charActivated( const QChar & ) ) );
    connect( charTable, SIGNAL( activated() ), this, SLOT( charActivated() ) );
    connect( charTable, SIGNAL( focusItemChanged( const QChar & ) ),
	     this, SLOT( charFocusItemChanged( const QChar & ) ) );
    connect( charTable, SIGNAL( focusItemChanged() ), this, SLOT( charFocusItemChanged() ) );
    connect( charTable, SIGNAL( tableUp() ), this, SLOT( charTableUp() ) );
    connect( charTable, SIGNAL( tableDown() ), this, SLOT( charTableDown() ) );

    connect( charTable, SIGNAL(doubleClicked()),this,SLOT(slotDoubleClicked()));

    setFocusPolicy( Qt::StrongFocus );
    setFocusProxy( charTable );
}

KCharSelect::~KCharSelect()
{
    delete d;
}

//==================================================================
QSize KCharSelect::sizeHint() const
{
    return KVBox::sizeHint();
}

//==================================================================
void KCharSelect::setFont( const QString &_font )
{
    int pos = fontList.indexOf ( _font );
    if ( pos != 1 ) {
	fontCombo->setCurrentIndex( pos );
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
    fontCombo->addItems( fontList );
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

