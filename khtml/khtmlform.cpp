/* 
    This file is part of the KDE libraries

    Copyright (C) 1997 Martin Jones (mjones@kde.org)
              (C) 1997 Torben Weis (weis@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE HTML Widget -- Forms
// $Id$

#include <stdlib.h>

#include <qobject.h>
#include <qobjectlist.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmultilinedit.h>
#include <qfontmetrics.h>
#include <qstrlist.h>

#include "khtmlform.h"

#include "khtmljscript.h"
#include "khtml.h"
#include "khtmlfont.h"

//----------------------------------------------------------------------------


QString HTMLElement::encodeString( const QString &e )
{
	static char *safe = "$-._!*(),"; /* RFC 1738 */
	unsigned pos = 0;
	QString encoded;
	char buffer[5];

	while ( pos < e.length() )
	{
		unsigned char c = (unsigned char) e[pos];

		if ( (( c >= 'A') && ( c <= 'Z')) ||
		     (( c >= 'a') && ( c <= 'z')) ||
		     (( c >= '0') && ( c <= '9')) ||
		     (strchr(safe, c))
		   )
		{
			encoded += c;
		}
		else if ( c == ' ' )
		{
			encoded += '+';
		}
		else if ( c == '\n' )
		{
			encoded += "%0D%0A";
		}
		else if ( c != '\r' )
		{
			sprintf( buffer, "%%%02X", (int)c );
			encoded += buffer;
		}
		pos++;
	}

	return encoded;
}

QString HTMLElement::decodeString( const char *e )
{
	unsigned int pos = 0;
	unsigned int len = strlen(e);
	char buffer[3];
	QString decoded;

	while ( pos < len )
	{
	     if (e[pos] == '%')
	     {
	         if (pos+2 < len)
	         {
	             buffer[0] = e[pos+1];
	             buffer[1] = e[pos+2];
	             buffer[2] = '\0';
	             unsigned char val = strtol(buffer, NULL, 16);
	             if (((char) val) != '\r')
	             {
	                 decoded += (char) val;     
	             }
	         }
	         else
	         {
	             decoded += e[pos];
	         }
	     }
	     else if (e[pos] == '+')
	     {
	       decoded += ' ';
	     }
	     else
	     {
	       decoded += e[pos];
	     }
	     pos++;
	}
	return decoded;
}

HTMLElement::~HTMLElement()
{
    if ( form )
	form->removeElement( this );
}

//----------------------------------------------------------------------------

HTMLWidgetElement *HTMLWidgetElement::currentFormFocusWidget = 0L;

HTMLWidgetElement::HTMLWidgetElement( const char *n, const HTMLFont *f = 0 ) 
    : HTMLElement( n )
{ 
    _absX = 0; 
    _absY = 0; 
    w = 0; 
    p = 0;
    font = f;
    showAsWidget = false;
}

HTMLWidgetElement::~HTMLWidgetElement()
{
    if ( w )
	delete w;
    if ( p )
	delete p;
}

void HTMLWidgetElement::position( int _x, int _y, int , int _height )
{
	if ( w == 0L ) // CC: HTMLHidden does not have a widget...
		return;

        _relX = absX() - _x;
	_relY = absY() - _y;

	if (!showAsWidget) 
	  return;
	  
	if ( _y > absY() + ascent + descent || _y + _height < absY() )
	{
		w->hide();
	}
	else
	{
		w->move( absX() - _x, absY() - _y );
		w->show();
	}
}

bool HTMLWidgetElement::print( QPainter *_painter, int, int _y, int, int _height, int _tx, int _ty, bool toPrinter )
{
    if ( _y + _height < y - ascent || _y > y + descent )
	return false;

    if ( toPrinter )
    {
	if ( _y + _height < y + descent )
	    return true;
	if ( isPrinted() )
	    return false;
	setPrinted( true );
    }

    print( _painter, _tx, _ty );

    return false;
}

void HTMLWidgetElement::print( QPainter *_painter, int _tx, int _ty )
{
//  if ( w == 0 || p == 0 || p->isNull() || w->isVisible())
  if ( w == 0 || p == 0 || p->isNull() || showAsWidget)
    {
//      printf("hidden %s\n", className());
      return;
    }  

  if (p->size() != w->size() )
     p->resize( w->width(), w->height() );
    
  paintWidget( w );

  _painter->drawPixmap( QPoint( x + _tx, y - ascent + _ty ), *p );
}

void HTMLWidgetElement::paintWidget( QWidget *widget )
{
  QPainter::redirect( widget, p );
  QPaintEvent pe( QRect(widget->pos().x(), widget->pos().y(), widget->width(), widget->height()) );
  QApplication::sendEvent( widget, &pe );
  QPainter::redirect( widget, 0 );
  
  const QObjectList *childrenList = w->children();
  if (childrenList)
     {
       QObjectListIt it( *childrenList );
       for (; it.current(); ++it)
         if (it.current()->isWidgetType())
	    {
	      QWidget *w = (QWidget *)it.current();
	      if (w->parentWidget() == widget && w->isVisible())
	        paintWidget( w );
	    }
     }  
}

HTMLObject *HTMLWidgetElement::mouseEvent( int _x, int _y, int button, int state )
{
//    printf("currentFormFocusWidget = %p ; this = %p \n", currentFormFocusWidget, this );

    if ( (!w->isVisible()) && currentFormFocusWidget != this )
     {
       if (currentFormFocusWidget)
          {
//	    printf("hiding active widget %s = %p\n", currentFormFocusWidget->className(), currentFormFocusWidget);
	    currentFormFocusWidget->setPixmapMode( true );
	  }    

       printf("activating %s = %p\n", className(), this);
	       
       currentFormFocusWidget = this;
       w->move( _relX, _relY );
       w->show();
       showAsWidget = true;
       
       w->setFocus();
     }
//  else w->hide();
  
  return 0L;
}

void HTMLWidgetElement::calcAbsolutePos( int _x, int _y )
{
	_absX = _x + x;
	_absY = _y + y - ascent;
}

void HTMLWidgetElement::setWidget( QWidget *_w )
{ 
    w = _w; 
    p = new QPixmap( w->width(), w->height() );

    p->fill( w->backgroundColor() );
}
//----------------------------------------------------------------------------

HTMLSelect::HTMLSelect( QWidget *parent, const char *n, int s, bool m,
			const HTMLFont *f )
	: HTMLWidgetElement( n, f )
{
    QWidget *w;
	_size = s;
	_defSelected = 0;
	_item = 0;

	_values.setAutoDelete( TRUE );

	QSize size;

	if ( _size > 1 )
	{
		w = new QListBox( parent );
		size.setWidth( 150 );
		size.setHeight( 20 * _size );
		ascent = 25;
		descent = size.height() - ascent;
		((QListBox *)w)->setMultiSelection( m );
	}
	else
	{
		w = new QComboBox( FALSE, parent );
		size.setWidth( 150 );
		size.setHeight( 25 );
		descent = 5;
		ascent = size.height() - descent;
	}
	if( font )
	    w->setFont( *font );

	connect( w, SIGNAL( highlighted( int ) ),
		 SLOT( slotHighlighted( int ) ) );

	w->resize( size );

	width = size.width();

	setWidget( w );
}

void HTMLSelect::addOption( const char *v, bool sel )
{
	if ( _size > 1 )
	{
		QListBox *lb = (QListBox *)widget();
		lb->insertItem( "" );
		if ( sel || lb->count() == 1 )
		{
			_defSelected = lb->count() - 1;
			lb->setSelected( _defSelected, true );
		}
		width = lb->maxItemWidth()+20;
		widget()->resize( width, widget()->height() );
	}
	else
	{
		QComboBox *cb = (QComboBox *)widget();
		cb->insertItem( "" );
		if ( sel || cb->count() == 1 )
		{
			_defSelected = cb->count() - 1;
			cb->setCurrentItem( _defSelected );
			_item = _defSelected;
		}
		QSize size = widget()->sizeHint();
		widget()->resize( size );
		ascent = size.height() - descent;
		width = size.width();
	}

	if ( v )
		_values.append( new QString( v ) );
	else
		_values.append( new QString( "" ) );
}

const QString &HTMLSelect::value( int item )
{
	return *_values.at( item );
}

void HTMLSelect::setValue( const char *v, int indx )
{
	*_values.at( indx ) = v;
}

void HTMLSelect::setText( const char *text )
{
	int item;
	QString t = text;
	t = t.stripWhiteSpace();

	if ( _size > 1 )
	{
		QListBox *lb = (QListBox *)widget();
		lb->changeItem( t, lb->count() - 1 );
		item = lb->count() - 1;
	}
	else
	{
		QComboBox *cb = (QComboBox *)widget();
		cb->changeItem( t, cb->count() - 1 );
		item = cb->count() - 1;
	}

	if ( strlen( value( item ) ) == 0 )
		setValue( t, item );
}

QString HTMLSelect::encoding()
{
    QString _encoding = "";

    if ( elementName().length() )
    {
	if ( _size > 1 && ((QListBox *)widget())->isMultiSelection() )
	{ // multiple
	    QListBox* lb = (QListBox *) widget();

	    for ( unsigned i = 0; i < lb->count(); i++ )
	    {
		if ( lb->isSelected( i ) )
		{
		    if ( !_encoding.isEmpty() )
			_encoding += '&';
		    _encoding += encodeString( elementName() );
		    _encoding += '=';
		    _encoding += encodeString( value( i ) );
		}
	    }
	}
	else
	{
	    _encoding = encodeString( elementName() );
	    _encoding += '=';
	    _encoding += encodeString( value() );
	}
    }

    return _encoding;
}

void HTMLSelect::resetElement(const char *data)
{
	if ( _size > 1 )
		((QListBox *)widget())->setCurrentItem( _defSelected );
	else
		((QComboBox *)widget())->setCurrentItem( _defSelected );
}

void HTMLSelect::slotHighlighted( int indx )
{
	_item = indx;
}

//----------------------------------------------------------------------------

HTMLTextArea::HTMLTextArea( QWidget *parent, const char *n, int r, int c,
			    const HTMLFont *f )
	: HTMLWidgetElement( n, f )
{
    QWidget *w;
	_defText = "";

printf("New HTMLTextArea element! Text = \"%s\"\n", n);

        w = new HTMLMultiLineEditWidget( this, parent );
	if( font )
	    w->setFont( *font );

	QFontMetrics fm( w->font() );

	QSize size( c * fm.width('a'), r * (fm.height()+1) );

	w->resize( size );

	descent = size.height() - 14;
	ascent = 14;
	width = size.width();
	setWidget( w );
}

QString HTMLTextArea::value()
{
	return ((QMultiLineEdit *)widget())->text();
}

void HTMLTextArea::setText( const char *t )
{
	_defText = t;
	((QMultiLineEdit *)widget())->setText( t );
}

QString HTMLTextArea::encoding()
{
	QString _encoding = "";

	if ( elementName().length() )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}
	
	return _encoding;
}

void HTMLTextArea::resetElement(const char *data)
{
	if (!data)
	{
		((QMultiLineEdit *)widget())->setText( _defText );
	}
	else
	{
		((QMultiLineEdit *)widget())->setText( decodeString(data) );
	}
}

//----------------------------------------------------------------------------

HTMLInput::HTMLInput( const char *n, const char *v, const HTMLFont *f )
	: HTMLWidgetElement( n, f )
{
	_value = v;
}

//----------------------------------------------------------------------------

HTMLButton::HTMLButton( KHTMLWidget *_parent, const char *_name, const char *v, QList<JSEventHandler> *_events, const HTMLFont *f )
	: HTMLInput( "", v, f )
{
    QWidget *w;
    view = _parent;
//    w = new QPushButton( _parent );
    w = new HTMLPushButtonWidget( this, _parent );
    if( font )
	w->setFont( *font );

    if ( strlen( value() ) != 0 )
	((QPushButton *)w)->setText( value() );
    else if ( strlen( _name ) != 0 )
	((QPushButton *)w)->setText( _name );
    else
	((QPushButton *)w)->setText( "" );

    QSize size = w->sizeHint();
    w->resize( size );
    
    descent = 5;
    ascent = size.height() - descent;
    width = size.width();
    
    connect( w, SIGNAL( clicked() ), SLOT( slotClicked() ) );
    
    eventHandlers = _events;
    setWidget( w );
}

void HTMLButton::slotClicked()
{
    if ( eventHandlers == 0L )
	return;
    
    JSEventHandler* ev;
    for ( ev = eventHandlers->first(); ev != 0L; ev = eventHandlers->next() )
    {
	if ( strcmp( ev->getName(), "onClick" ) == 0L )
	{
	    ev->exec( 0L );
	    return;
	}
    }
}

HTMLButton::~HTMLButton()
{
    if ( eventHandlers )
	delete eventHandlers;
}

//----------------------------------------------------------------------------

HTMLCheckBox::HTMLCheckBox( QWidget *parent, const char *n, const char *v,
		bool ch, const HTMLFont *f )
	: HTMLInput( n, v, f )
{
    QWidget *w;
	_defCheck = ch;

//	w = new QCheckBox( parent );
        w = new HTMLCheckBoxWidget( this, parent );
	if( font )
	    w->setFont( *font );

	((QCheckBox *)w)->setChecked( ch );

	QSize size( 14, 14 );

	w->resize( size );

	descent = 1;
	ascent = size.height() - descent;
	width = size.width() + 6;

	setWidget( w );
}

QString HTMLCheckBox::encoding()
{
	QString _encoding = "";

	if ( ((QCheckBox *)widget())->isChecked() )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLCheckBox::resetElement(const char *data)
{
	((QCheckBox *)widget())->setChecked( _defCheck );
}

//----------------------------------------------------------------------------

HTMLHidden::HTMLHidden( const char *n, const char *v )
	: HTMLInput( n, v )
{
}

QString HTMLHidden::encoding()
{
	QString _encoding;

	if ( elementName().length() )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}
	
	return _encoding;
}

//----------------------------------------------------------------------------

HTMLRadio::HTMLRadio( QWidget *parent, const char *n, const char *v,
		bool ch, const HTMLFont *f )
	: HTMLInput( n, v, f )
{
    QWidget *w;
	_defCheck = ch;

//	w = new QRadioButton( parent );
        w = new HTMLRadioButtonWidget( this, parent );
	if( font )
	    w->setFont( *font );

	((QRadioButton *)w)->setChecked( ch );

	QSize size( 14, 14 );

	w->resize( size );

	descent = 1;
	ascent = size.height() - descent;
	width = size.width() + 6;

	connect( w, SIGNAL( clicked() ), SLOT( slotClicked() ) );
	setWidget( w );
}

QString HTMLRadio::encoding()
{
	QString _encoding = "";

	if ( ((QRadioButton *)widget())->isChecked() )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLRadio::resetElement(const char *data)
{
	((QRadioButton *)widget())->setChecked( _defCheck );
}

void HTMLRadio::slotClicked()
{
	emit radioSelected( elementName(), value() );
}

void HTMLRadio::slotRadioSelected( const char *n, const char *v )
{
	if ( strcasecmp( n, elementName().data() ) != 0 )
		return;

	if ( strcasecmp( v, value().data() ) != 0 )
		((QRadioButton *)widget())->setChecked( false );
}

//----------------------------------------------------------------------------

HTMLReset::HTMLReset( QWidget *parent, const char *v, const HTMLFont *f )
	: HTMLInput( "", v, f )
{
    QWidget *w;
    w = new HTMLPushButtonWidget( this, parent );
	if( font )
	    w->setFont( *font );

	if ( strlen( value() ) != 0 )
		((QPushButton *)w)->setText( value() );
	else
		((QPushButton *)w)->setText( "Reset" );

	QSize size = w->sizeHint();
	w->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( w, SIGNAL( clicked() ), SLOT( slotClicked() ) );

	setWidget( w );
}

void HTMLReset::slotClicked()
{
	emit resetForm();
}

//----------------------------------------------------------------------------

HTMLSubmit::HTMLSubmit( QWidget *parent, const char *n, const char *v,
			const HTMLFont *f )
	: HTMLInput( n, v, f )
{
    QWidget *w;
        w = new HTMLPushButtonWidget( this, parent );
	if( font )
	    w->setFont( *font );

	if ( strlen( value() ) != 0 )
		((QPushButton *)w)->setText( value() );
	else
		((QPushButton *)w)->setText( "Submit Query" );

	QSize size = w->sizeHint();
	w->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( w, SIGNAL( clicked() ), SLOT( slotClicked() ) );

	activated = false;
	
	setWidget( w );
}

QString HTMLSubmit::encoding()
{
	QString _encoding = "";

	if ( elementName().length() && activated )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLSubmit::slotClicked()
{
	activated = true;
	emit submitForm();
}

//----------------------------------------------------------------------------

HTMLTextInput::HTMLTextInput( QWidget *parent, const char *n, const char *v,
		int s, int ml, bool password, const HTMLFont *f )
	: HTMLInput( n, v, f )
{
    QWidget *w;
	_defText = v;

	w = new QLineEdit( parent );
	if( font )
	    w->setFont( *font );

	if ( strlen( value() ) != 0 )
		((QLineEdit *)w)->setText( value() );
	if ( password )
	    ((QLineEdit *)w)->setEchoMode ( QLineEdit::Password );

	if ( ml > 0 )
	    ((QLineEdit *)w)->setMaxLength( ml );

	QSize size( s * 8, 25 );
	w->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( w, SIGNAL( textChanged( const char * ) ),
			SLOT( slotTextChanged( const char * ) ) );
	connect( w, SIGNAL( returnPressed() ),
			SLOT( slotReturnPressed() ) );

	setWidget( w );
}

QString HTMLTextInput::encoding()
{
	QString _encoding = "";

	if ( elementName().length() )
	{
		_encoding = encodeString( elementName() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLTextInput::resetElement(const char *data)
{
	if (!data)
	{
		((QLineEdit *)widget())->setText( _defText );
	}
	else
	{
		((QLineEdit *)widget())->setText( decodeString(data) );
	}
}

void HTMLTextInput::slotTextChanged( const char *t )
{
	setValue( t );
}

void HTMLTextInput::slotReturnPressed()
{
	emit submitForm();
}

//----------------------------------------------------------------------------

HTMLImageInput::HTMLImageInput( KHTMLWidget *widget, const char *f,
	const char *n )
    : QObject(), HTMLImage( widget, f, 0, 0), HTMLElement( n )
{
    _xp = _yp = 0;
    pressed = false;
    activated = false;
}

QString HTMLImageInput::encoding()
{
    QString _encoding = "";

    if ( elementName().length() && activated )
    {
	QString num;

	num.setNum( _xp );
	_encoding = encodeString( elementName() );
	_encoding += ".x=";
	_encoding += num;

	_encoding += "&";

	num.setNum( _yp );
	_encoding += encodeString( elementName() );
	_encoding += ".y=";
	_encoding += num;
    }

    return _encoding;
}

HTMLObject *HTMLImageInput::mouseEvent( int _x, int _y, int button, int state )
{
    if ( _x < x || _x > x + width )
	return 0;

    if ( _y <= y - ascent || _y > y + descent )
	return 0;

    if ( button == LeftButton )
    {
	if ( ! ( state & LeftButton ) )
	{
	    pressed = true;
	}
	else if ( pressed )
	{
	    _xp = _x - x;
	    _yp = _y - ( y - ascent );
	    activated = true;
	    emit submitForm();
	    pressed = false;
	}

	return this;
    }

    return 0;
}

//----------------------------------------------------------------------------

HTMLForm::HTMLForm( const char *a, const char *m, const char *t )
{
    _action = a;
    _method = m;
    _target = t;

    elements.setAutoDelete( false );
    hidden.setAutoDelete( true );
}

void HTMLForm::addElement( HTMLElement *e )
{
    elements.append( e );
}

void HTMLForm::addHidden( HTMLHidden *e )
{
    elements.append( e );
    hidden.append( e );
}

void HTMLForm::removeElement( HTMLElement *e )
{
    elements.removeRef( e );
}

void HTMLForm::position( int _x, int _y, int _width, int _height )
{
    HTMLElement *e;

    for ( e = elements.first(); e != 0; e = elements.next() )
    {
	e->position( _x, _y, _width, _height );
    }
}

void HTMLForm::slotReset()
{
    HTMLElement *e;

    for ( e = elements.first(); e != 0; e = elements.next() )
    {
	e->resetElement();
    }
}

void HTMLForm::slotSubmit()
{
	HTMLElement *e;
	QString encoding = "";
	bool first = true;

	for ( e = elements.first(); e != 0; e = elements.next() )
	{
		QString enc = e->encoding();
		if ( enc.length() )
		{
			if ( !first )
				encoding += '&';
			encoding += enc;
			first = false;
		}
	}

	QString url = action();

	emit submitted( method(), url, encoding, _target );
}

void HTMLForm::slotRadioSelected( const char *n, const char *v )
{
    emit radioSelected( n, v );
}

void HTMLForm::saveForm(QStrList *saveList)
{
    for (HTMLElement *e = elements.first(); e != 0; e = elements.next() )
    {
	saveList->append( e->encoding() );
    }
}

void HTMLForm::restoreForm(QStrList *saveList)
{
    for (HTMLElement *e = elements.first(); e != 0; e = elements.next() )
    {
        if(!saveList->current())
        {
            break;
        }
	e->resetElement( saveList->current() );
	(void) saveList->next();
    }
}

HTMLForm::~HTMLForm()
{
    for (HTMLElement *e = elements.first(); e != 0; e = elements.next() )
    {
	e->setForm( 0 );
    }
}

//----------------------------------------------------------------------------

HTMLMultiLineEditWidget::HTMLMultiLineEditWidget( HTMLWidgetElement *htmlParent, QWidget *parent, const char *name )
: QMultiLineEdit( parent, name )
{
  widgetElement = htmlParent;
}

HTMLMultiLineEditWidget::~HTMLMultiLineEditWidget()
{
}

void HTMLMultiLineEditWidget::paintEvent( QPaintEvent *pe )
{
  if ( widgetElement->isInPixmapMode() )
     {
       bool f = testWFlags( WState_Visible );
       setWFlags( WState_Visible );
       QTableView::paintEvent( pe );
       if (!f) clearWFlags( WState_Visible );
     }
   else
       QTableView::paintEvent( pe );  
}

//----------------------------------------------------------------------------

HTMLPushButtonWidget::HTMLPushButtonWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0)
: QPushButton( parent )
{
  widgetElement = htmlParent;
}

HTMLPushButtonWidget::~HTMLPushButtonWidget()
{
}

void HTMLPushButtonWidget::paintEvent( QPaintEvent *pe )
{
  QPainter painter;
  
  painter.begin( this );
  
  drawButton( &painter );
  
  painter.end();
}

//----------------------------------------------------------------------------

HTMLRadioButtonWidget::HTMLRadioButtonWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0)
: QRadioButton( parent )
{
  widgetElement = htmlParent;
}

HTMLRadioButtonWidget::~HTMLRadioButtonWidget()
{
}

void HTMLRadioButtonWidget::paintEvent( QPaintEvent *pe )
{
  QPainter painter;
  
  painter.begin( this );
  
  drawButton( &painter );
  
  painter.end();
}

//----------------------------------------------------------------------------

HTMLCheckBoxWidget::HTMLCheckBoxWidget( HTMLWidgetElement *htmlParent, QWidget *parent = 0, const char *name = 0)
: QCheckBox( parent )
{
  widgetElement = htmlParent;
}

HTMLCheckBoxWidget::~HTMLCheckBoxWidget()
{
}

void HTMLCheckBoxWidget::paintEvent( QPaintEvent *pe )
{
  QPainter painter;
  
  painter.begin( this );
  
  drawButton( &painter );
  
  painter.end();
}

#include "khtmlform.moc"
