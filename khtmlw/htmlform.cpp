//----------------------------------------------------------------------------
// khtml widget - forms
//
//

#include <qobject.h>
#include <qlistbox.h>
#include <qcombo.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include <qpushbt.h>
#include <qlined.h>
#include <qmlined.h>
#include "htmlform.h"

#include "htmlform.moc"

//----------------------------------------------------------------------------

void HTMLElement::position( int _x, int _y, int , int _height )
{
	if ( widget == NULL ) // CC: HTMLHidden does not have a widget...
		return;
	
//	if ( _y > absY() + ascent || _y + _height < absY() )
	if ( _y > absY() + ascent + descent || _y + _height < absY() )
	{
		widget->hide();
	}
	else
	{
		widget->move( absX() - _x, absY() - _y );
		widget->show();
	}
}

void HTMLElement::calcAbsolutePos( int _x, int _y )
{
	_absX = _x + x;
	_absY = _y + y - ascent;
}

QString HTMLElement::encodeString( const QString &e )
{
	unsigned pos = 0;
	QString encoded;

	do
	{
		unsigned char c = e[pos];

		if ( c == ' ' )
			encoded += '+';
		else if ( c > 127 || c == '=' || c == '&' || c == '%' || c == '+' )
		{
			char buffer[5];
			sprintf( buffer, "%%%02X", (int)c );
			encoded += buffer;
		}
		else
			encoded += c;
		pos++;
	}
	while ( pos < e.length() );

	return encoded;
}

HTMLElement::~HTMLElement()
{
	if ( widget )
		delete widget;
}

//----------------------------------------------------------------------------

HTMLSelect::HTMLSelect( QWidget *parent, const char *n, int s, bool )
	: HTMLElement( n )
{
	_size = s;
	_defSelected = 0;
	_item = 0;

	_values.setAutoDelete( TRUE );

	QSize size;

	if ( _size > 1 )
	{
		widget = new QListBox( parent );
		size.setWidth( 100 );
		size.setHeight( 20 * _size );
		ascent = 25;
		descent = size.height() - ascent;
	}
	else
	{
		widget = new QComboBox( FALSE, parent );
		size.setWidth( 100 );
		size.setHeight( 25 );
		descent = 3;
		ascent = size.height() - descent;
	}

	connect( widget, SIGNAL( highlighted( int ) ),
			SLOT( slotHighlighted( int ) ) );

	widget->resize( size );

	width = size.width();
}

void HTMLSelect::addOption( const char *v, bool sel )
{
	if ( _size > 1 )
	{
		QListBox *lb = (QListBox *)widget;
		lb->insertItem( "" );
		if ( sel || lb->count() == 1 )
		{
			_defSelected = lb->count() - 1;
			lb->setCurrentItem( _defSelected );
		}
	}
	else
	{
		QComboBox *cb = (QComboBox *)widget;
		cb->insertItem( "" );
		if ( sel || cb->count() == 1 )
		{
			_defSelected = cb->count() - 1;
			cb->setCurrentItem( _defSelected );
		}
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
	char *ptr = t.data() + t.length() - 1;

	// remove trailing spaces
	while ( ptr > t.data() && *ptr == ' ' )
		*ptr-- = '\0';

	if ( _size > 1 )
	{
		QListBox *lb = (QListBox *)widget;
		lb->changeItem( t, lb->count() - 1 );
		item = lb->count() - 1;
	}
	else
	{
		QComboBox *cb = (QComboBox *)widget;
		cb->changeItem( t, cb->count() - 1 );
		item = cb->count() - 1;
	}

	if ( strlen( value( item ) ) == 0 )
		setValue( t, item );
}

QString HTMLSelect::encoding()
{
	QString _encoding = "";

	if ( name().length() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLSelect::reset()
{
	if ( _size > 1 )
		((QListBox *)widget)->setCurrentItem( _defSelected );
	else
		((QComboBox *)widget)->setCurrentItem( _defSelected );
}

void HTMLSelect::slotHighlighted( int indx )
{
	_item = indx;
}

//----------------------------------------------------------------------------

HTMLTextArea::HTMLTextArea( QWidget *parent, const char *n, int r, int c )
	: HTMLElement( n )
{
	_defText = "";

	widget = new QMultiLineEdit( parent );

	QSize size( c * 8, r * 22 );

	widget->resize( size );

	descent = size.height() - 14;
	ascent = 14;
	width = size.width();
}

QString HTMLTextArea::value()
{
	return ((QMultiLineEdit *)widget)->text();
}

void HTMLTextArea::setText( const char *t )
{
	_defText = t;
	((QMultiLineEdit *)widget)->setText( t );
}

QString HTMLTextArea::encoding()
{
	QString _encoding = "";

	if ( name().length() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}
	
	return _encoding;
}

void HTMLTextArea::reset()
{
	((QMultiLineEdit *)widget)->setText( _defText );
}

//----------------------------------------------------------------------------

HTMLInput::HTMLInput( const char *n, const char *v )
	: HTMLElement( n )
{
	_value = v;
}

//----------------------------------------------------------------------------

HTMLCheckBox::HTMLCheckBox( QWidget *parent, const char *n, const char *v,
		bool ch )
	: HTMLInput( n, v )
{
	_defCheck = ch;

	widget = new QCheckBox( parent );

	((QCheckBox *)widget)->setChecked( ch );

	QSize size( 14, 14 );

	widget->resize( size );

	descent = 1;
	ascent = size.height() - descent;
	width = size.width() + 6;
}

QString HTMLCheckBox::encoding()
{
	QString _encoding = "";

	if ( ((QCheckBox *)widget)->isChecked() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLCheckBox::reset()
{
	((QCheckBox *)widget)->setChecked( _defCheck );
}

//----------------------------------------------------------------------------

HTMLHidden::HTMLHidden( const char *n, const char *v )
	: HTMLInput( n, v )
{
}

QString HTMLHidden::encoding()
{
	QString _encoding = "";

	if ( name().length() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}
	
	return _encoding;
}

//----------------------------------------------------------------------------

HTMLRadio::HTMLRadio( QWidget *parent, const char *n, const char *v,
		bool ch )
	: HTMLInput( n, v )
{
	_defCheck = ch;

	widget = new QRadioButton( parent );

	((QRadioButton *)widget)->setChecked( ch );

	QSize size( 14, 14 );

	widget->resize( size );

	descent = 1;
	ascent = size.height() - descent;
	width = size.width() + 6;

	connect( widget, SIGNAL( clicked() ), SLOT( slotClicked() ) );
}

QString HTMLRadio::encoding()
{
	QString _encoding = "";

	if ( ((QRadioButton *)widget)->isChecked() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLRadio::reset()
{
	((QRadioButton *)widget)->setChecked( _defCheck );
}

void HTMLRadio::slotClicked()
{
	emit radioSelected( name(), value() );
}

void HTMLRadio::slotRadioSelected( const char *n, const char *v )
{
	if ( strcasecmp( n, name() ) != 0 )
		return;

	if ( strcasecmp( v, value() ) != 0 )
		((QRadioButton *)widget)->setChecked( false );
}

//----------------------------------------------------------------------------

HTMLReset::HTMLReset( QWidget *parent, const char *v )
	: HTMLInput( "", v )
{
	widget = new QPushButton( parent );

	if ( strlen( value() ) != 0 )
		((QPushButton *)widget)->setText( value() );
	else
		((QPushButton *)widget)->setText( "Reset" );

	QSize size = widget->sizeHint();
	widget->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( widget, SIGNAL( clicked() ), SLOT( slotClicked() ) );
}

void HTMLReset::slotClicked()
{
	emit resetForm();
}

//----------------------------------------------------------------------------

HTMLSubmit::HTMLSubmit( QWidget *parent, const char *v )
	: HTMLInput( "", v )
{
	widget = new QPushButton( parent );

	if ( strlen( value() ) != 0 )
		((QPushButton *)widget)->setText( value() );
	else
		((QPushButton *)widget)->setText( "Submit Query" );

	QSize size = widget->sizeHint();
	widget->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( widget, SIGNAL( clicked() ), SLOT( slotClicked() ) );
}

void HTMLSubmit::slotClicked()
{
	emit submitForm();
}

//----------------------------------------------------------------------------

HTMLTextInput::HTMLTextInput( QWidget *parent, const char *n, const char *v,
		int s )
	: HTMLInput( n, v )
{
	_defText = v;

	widget = new QLineEdit( parent );

	if ( strlen( value() ) != 0 )
		((QLineEdit *)widget)->setText( value() );

	((QLineEdit *)widget)->setMaxLength( s );

	QSize size( s * 8, 25 );
	widget->resize( size );

	descent = 5;
	ascent = size.height() - descent;
	width = size.width();

	connect( widget, SIGNAL( textChanged( const char * ) ),
			SLOT( slotTextChanged( const char * ) ) );
	connect( widget, SIGNAL( returnPressed() ),
			SLOT( slotReturnPressed() ) );
}

QString HTMLTextInput::encoding()
{
	QString _encoding = "";

	if ( name().length() )
	{
		_encoding = encodeString( name() );
		_encoding += '=';
		_encoding += encodeString( value() );
	}

	return _encoding;
}

void HTMLTextInput::reset()
{
	((QLineEdit *)widget)->setText( _defText );
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

HTMLForm::HTMLForm( const char *a, const char *m )
{
	_action = a;
	_method = m;

	elements.setAutoDelete( FALSE );
}

void HTMLForm::addElement( HTMLElement *e )
{
	elements.append( e );
}

void HTMLForm::position( int _x, int _y, int _width, int _height )
{
	HTMLElement *e;

	for ( e = elements.first(); e != NULL; e = elements.next() )
	{
		e->position( _x, _y, _width, _height );
	}
}

void HTMLForm::slotReset()
{
	HTMLElement *e;

	for ( e = elements.first(); e != NULL; e = elements.next() )
	{
		e->reset();
	}
}

void HTMLForm::slotSubmit()
{
	HTMLElement *e;
	QString encoding = "";
	bool first = true;

	for ( e = elements.first(); e != NULL; e = elements.next() )
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
	url += '?';
	url += encoding;

	emit submitted( method(), url );
}

void HTMLForm::slotRadioSelected( const char *n, const char *v )
{
	emit radioSelected( n, v );
}

