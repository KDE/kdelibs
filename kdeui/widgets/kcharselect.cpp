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

#include "kcharselect_p.h"
#include "kcharselect_p.moc"

#include <qcolor.h>
#include <qfontcombobox.h>
#include <qevent.h>
#include <qfont.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpainter.h>
#include <qpen.h>
#include <qregexp.h>
#include <qspinbox.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qvalidator.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <qheaderview.h>

class KCharSelectTablePrivate
{
public:
    KCharSelectTablePrivate(KCharSelectTable *q): q(q), m_model(0) {}
  
    void _k_slotCurrentChanged( const QModelIndex & current, const QModelIndex & previous );
  
    KCharSelectTable *q;
  
    /** Current font name. @see setFont() */
    QString vFont;
    /** Currently highlighted character. @see chr() @see setChar() */
    QChar vChr;
    /** Current table number. @see setTable() */
    int vTableNum;
    QPoint vPos;
    QChar focusItem;
    QPoint focusPos;
    int temp;
    KCharSelectItemModel *m_model;
};

class KCharSelect::KCharSelectPrivate
{
public:
    QLineEdit *unicodeLine;
    QFontComboBox *fontCombo;
    QSpinBox *tableSpinBox;
    KCharSelectTable *charTable;

    KCharSelect *q;

    inline void _k_charTableUp() { if ( q->tableNum() < 255 ) q->setTableNum( q->tableNum() + 1 ); }
    inline void _k_charTableDown() { if ( q->tableNum() > 0 ) q->setTableNum( q->tableNum() - 1 ); }
    void _k_fontSelected( const QString &_font );
    void _k_tableChanged( int _value );
    void _k_slotUnicodeEntered();
    void _k_slotUpdateUnicode( const QChar &c );
};

/******************************************************************/
/* Class: KCharSelectTable					  */
/******************************************************************/

//==================================================================
KCharSelectTable::KCharSelectTable( QWidget *parent, const QString &_font,
				    const QChar &_chr, int _tableNum )
    : QTableView( parent), d(new KCharSelectTablePrivate(this))
{
    d->vFont = _font;
    d->vChr = _chr;
    d->vTableNum = _tableNum;
    d->vPos = QPoint( 0, 0 );
    d->focusItem = _chr;
    d->focusPos = QPoint( 0, 0 );
    d->m_model = 0;
      
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    QPalette _palette;
    _palette.setColor( backgroundRole(), palette().color( QPalette::Base ) );
    setPalette( _palette );
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

KCharSelectTable::~KCharSelectTable()
{
    delete d;
}

//==================================================================
void KCharSelectTable::setFont( const QString &_font )
{
    d->vFont = _font;
    if (d->m_model) d->m_model->setFont(_font);
}

QChar KCharSelectTable::chr()
{
    return d->vChr;
}

//==================================================================
void KCharSelectTable::setChar( const QChar &_chr )
{
    //const uint short chr=_chr;
    //if (chr)
    d->vChr = _chr;
#ifdef __GNUC__
    #warning fixme //repaintContents( false );
#endif
}

//==================================================================
void KCharSelectTable::setTableNum( int _tableNum )
{
    d->focusItem = QChar( _tableNum * 256 );

    d->vTableNum = _tableNum;
    
    KCharSelectItemModel *m=d->m_model;
    d->m_model=new KCharSelectItemModel(_tableNum,d->vFont,this);
    setModel(d->m_model);
    QItemSelectionModel *selectionModel=new QItemSelectionModel(d->m_model);
    setSelectionModel(selectionModel);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);
    connect(selectionModel,SIGNAL(currentChanged ( const QModelIndex & ,const QModelIndex &)), this, SLOT(_k_slotCurrentChanged ( const QModelIndex &, const QModelIndex &)));
    delete m; // this should hopefully delete aold selection models too, since it is the parent of them (didn't track, if there are setParent calls somewhere. Check that (jowenn)
#ifdef __GNUC__
    #warning fixme //repaintContents( false );
#endif
}

//==================================================================
void KCharSelectTablePrivate::_k_slotCurrentChanged ( const QModelIndex & current, const QModelIndex & previous ) {
	Q_UNUSED(previous);
	if (!m_model) return;
	focusItem = m_model->data(current,KCharSelectItemModel::CharacterRole).toChar();
	emit q->focusItemChanged( focusItem );
	emit q->focusItemChanged();
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
    	hv->resizeSection(i,new_w);
    }
    hv=verticalHeader();
    for (int i=0;i<rows;i++) {
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
	const QPoint oldPos = d->vPos;

	d->vPos.setX( col );
	d->vPos.setY( row );

	d->vChr = QChar( d->vTableNum * 256 + numCols * d->vPos.y() + d->vPos.x() );

	const QPoint oldFocus = d->focusPos;

	d->focusPos = d->vPos;
	d->focusItem = d->vChr;

#ifdef __GNUC__
#warning fixme
#endif
/*	
	repaintCell( oldFocus.y(), oldFocus.x(), true );
	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( d->vPos.y(), d->vPos.x(), true );
*/
	/*emit highlighted( d->vChr );
	emit highlighted();*/

	emit focusItemChanged( d->focusItem );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::keyPressEvent( QKeyEvent *e )
{
    if (d->m_model)
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
            const QPoint oldPos = d->vPos;

	    d->vPos = d->focusPos;
	    d->vChr = d->focusItem;

	    emit activated( d->m_model->data(currentIndex(),KCharSelectItemModel::CharacterRole).toChar());
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
KCharSelect::KCharSelect( QWidget *parent, const QString &_font, const QChar &_chr, int _tableNum )
  : KVBox( parent ), d(new KCharSelectPrivate)
{
    d->q = this;
    setSpacing( KDialog::spacingHint() );
    KHBox* const bar = new KHBox( this );
    bar->setSpacing( KDialog::spacingHint() );

    QLabel* const lFont = new QLabel( i18n( "Font:" ), bar );
    lFont->resize( lFont->sizeHint() );
    lFont->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lFont->setMaximumWidth( lFont->sizeHint().width() );

    d->fontCombo = new QFontComboBox(bar);
    d->fontCombo->setEditable(true);
    d->fontCombo->resize( d->fontCombo->sizeHint() );

    connect( d->fontCombo, SIGNAL( currentIndexChanged( const QString & ) ), this, SLOT( _k_fontSelected( const QString & ) ) );

    QLabel* const lTable = new QLabel( i18n( "Table:" ), bar );
    lTable->resize( lTable->sizeHint() );
    lTable->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
    lTable->setMaximumWidth( lTable->sizeHint().width() );

    d->tableSpinBox = new QSpinBox( bar );
    d->tableSpinBox->setRange(0,255);
    d->tableSpinBox->setSingleStep(1);
    d->tableSpinBox->resize( d->tableSpinBox->sizeHint() );

    connect( d->tableSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( _k_tableChanged( int ) ) );

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
    d->_k_slotUpdateUnicode(_chr);

    connect( d->unicodeLine, SIGNAL( returnPressed() ), this, SLOT( _k_slotUnicodeEntered() ) );

    d->charTable = new KCharSelectTable( this, _font.isEmpty() ? KVBox::font().family() : _font, _chr, _tableNum );

    const QSize sz( 200,
                    200 );    
#ifdef __GNUC__
    #warning fixme
#endif
    #if 0    
    const QSize sz( charTable->contentsWidth()  +  4 ,
                    charTable->contentsHeight() +  4 );
    #endif
    d->charTable->resize( sz );
    //charTable->setMaximumSize( sz );
    d->charTable->setMinimumSize( sz );

    d->charTable->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    d->charTable->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

    setFont( _font.isEmpty() ? KVBox::font().family() : _font );
    setTableNum( _tableNum );

    connect( d->charTable, SIGNAL( focusItemChanged( const QChar & ) ), this, SLOT( _k_slotUpdateUnicode( const QChar & ) ) );
    connect( d->charTable, SIGNAL( focusItemChanged( const QChar & ) ), this, SIGNAL( highlighted( const QChar & ) ) );
    connect( d->charTable, SIGNAL( focusItemChanged() ), this, SIGNAL( highlighted() ) );
    connect( d->charTable, SIGNAL( activated( const QChar & ) ), this, SIGNAL( activated( const QChar & ) ) );
    connect( d->charTable, SIGNAL( activated() ), this, SIGNAL( activated() ) );
    connect( d->charTable, SIGNAL( focusItemChanged( const QChar & ) ),
	     this, SIGNAL( focusItemChanged( const QChar & ) ) );
    connect( d->charTable, SIGNAL( focusItemChanged() ), this, SIGNAL( focusItemChanged() ) );
    connect( d->charTable, SIGNAL( tableUp() ), this, SLOT( _k_charTableUp() ) );
    connect( d->charTable, SIGNAL( tableDown() ), this, SLOT( _k_charTableDown() ) );

    connect( d->charTable, SIGNAL(doubleClicked()),this,SIGNAL(doubleClicked()));

    setFocusPolicy( Qt::StrongFocus );
    setFocusProxy( d->charTable );
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
    d->fontCombo->setCurrentFont(_font);
    d->charTable->setFont( _font );
}

//==================================================================
void KCharSelect::setChar( const QChar &_chr )
{
    d->charTable->setChar( _chr );
    d->_k_slotUpdateUnicode( _chr );
}

//==================================================================
void KCharSelect::setTableNum( int _tableNum )
{
    d->tableSpinBox->setValue( _tableNum );
    d->charTable->setTableNum( _tableNum );
}

//==================================================================
QChar KCharSelect::chr() const
{
    return d->charTable->chr();
}

//==================================================================
QString KCharSelect::font() const
{
    return d->fontCombo->currentText();
}

//==================================================================
int KCharSelect::tableNum() const
{
    return d->tableSpinBox->value();
}

//==================================================================
void KCharSelect::enableFontCombo( bool e )
{
    d->fontCombo->setEnabled( e );
}

//==================================================================
void KCharSelect::enableTableSpinBox( bool e )
{
    d->tableSpinBox->setEnabled( e );
}

//==================================================================
bool KCharSelect::isFontComboEnabled() const
{
    return d->fontCombo->isEnabled();
}

//==================================================================
bool KCharSelect::isTableSpinBoxEnabled() const
{
    return d->tableSpinBox->isEnabled();
}

//==================================================================
void KCharSelect::KCharSelectPrivate::_k_fontSelected( const QString &_font )
{
    charTable->setFont( _font );
    emit q->fontChanged( _font );
}

//==================================================================
void KCharSelect::KCharSelectPrivate::_k_tableChanged( int _value )
{
    charTable->setTableNum( _value );
}

//==================================================================
void KCharSelect::KCharSelectPrivate::_k_slotUnicodeEntered( )
{
    const QString s = unicodeLine->text();
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
    emit q->activated( ch );
}

void KCharSelect::KCharSelectPrivate::_k_slotUpdateUnicode( const QChar &c )
{
    const int uc = c.unicode();
    QString s;
    s.sprintf("%04X", uc);
    unicodeLine->setText(s);
}


#include "kcharselect.moc"
