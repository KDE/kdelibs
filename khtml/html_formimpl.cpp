/**
 * This file is part of the DOM implementation for KDE.
 *
 * (C) 1999 Lars Knoll (knoll@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id$
 */
// -------------------------------------------------------------------------

#include <stdio.h>

#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qstack.h>

#include "khtml.h"
#include "html_form.h"
#include "dom_string.h"
#include "dom_textimpl.h"
#include "html_documentimpl.h"

#include "khtmlattrs.h"
using namespace DOM;

#include "html_formimpl.moc"

HTMLFormElementImpl::HTMLFormElementImpl(DocumentImpl *doc)
    : HTMLBlockElementImpl(doc)
{
    post = false;
}

HTMLFormElementImpl::~HTMLFormElementImpl()
{
}

const DOMString HTMLFormElementImpl::nodeName() const
{
    return "FORM";
}

ushort HTMLFormElementImpl::id() const
{
    return ID_FORM;
}

long HTMLFormElementImpl::length() const
{
    // ###
    return 0;
}

void HTMLFormElementImpl::submit(  )
{
}

void HTMLFormElementImpl::reset(  )
{
}

void HTMLFormElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ACTION:
	url = attr->value();
	break;
    case ATTR_TARGET:
	target = attr->value();
	break;
    case ATTR_METHOD:
	if ( strcasecmp( attr->value(), "post" ) == 0 )
	    post = true;
	break;
    case ATTR_ENCTYPE:
    case ATTR_ACCEPT_CHARSET:
    case ATTR_ACCEPT:
	// ignore these for the moment...
	break;
    default:
	HTMLBlockElementImpl::parseAttribute(attr);
    }
}

void HTMLFormElementImpl::attach(KHTMLWidget *_view)
{
    view = _view;
}

void HTMLFormElementImpl::detach()
{
    view = 0;
}

void HTMLFormElementImpl::slotSubmit()
{
    printf("submit pressed!\n");
    if(!view) return;

    QStack<NodeImpl> nodeStack;
    QString formData;
    bool first = true;

    NodeImpl *current = _first;
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    switch(current->id())
	    {
	    case ID_INPUT:
	    {
		QString enc =
		    static_cast<HTMLGenericFormElementImpl *>(current)
		    ->encoding();
		if(enc.length())
		{
		    if(!first)
			formData += '&';
		    formData += enc;
		    first = false;
		}
		break;
	    }
	    default:
		break;
	    }

	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
    }

    printf("formdata = %s\n", formData.ascii());

    if(post)
	view->slotFormSubmitted( "post", url.string(), formData.latin1(),
			      target.string() );
    else
	view->slotFormSubmitted( "get", url.string(), formData.latin1(),
			      target.string() );
}

void HTMLFormElementImpl::slotReset()
{
    printf("rest pressed!\n");
}

void HTMLFormElementImpl::radioClicked( NodeImpl *caller, DOMString ident )
{
    printf("radioClicked\n");
    if(!view) return;

    QStack<NodeImpl> nodeStack;
    QString formData;

    NodeImpl *current = _first;
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    switch(current->id())
	    {
	    case ID_INPUT:
	    {
		if(current == caller) break;
		HTMLInputElementImpl *e = static_cast<HTMLInputElementImpl *>(current);
		if(e->_type == HTMLInputElementImpl::RADIO)
		{
		    if(e->_name == ident)
			e->setChecked(false);
		}
		break;
	    }
	    default:
		break;
	    }

	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
    }


}

// -------------------------------------------------------------------------

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentImpl *doc)
    : HTMLPositionedElementImpl(doc)
{
    _form = 0;
    view = 0;
    w = 0;

    badPos = true;
}

HTMLGenericFormElementImpl::~HTMLGenericFormElementImpl()
{
}

QString HTMLGenericFormElementImpl::encodeString( QString e )
{
	static const char *safe = "$-._!*(),"; /* RFC 1738 */
	unsigned pos = 0;
	QString encoded;
	char buffer[5];

	while ( pos < e.length() )
	{
		QChar c = e[pos];

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

QString HTMLGenericFormElementImpl::decodeString( QString e )
{
	unsigned int pos = 0;
	unsigned int len = e.length();
	QString decoded;

	while ( pos < len )
	{
	     if (e[pos] == QChar('%'))
	     {
	         if (pos+2 < len)
	         {
		     DOMString buffer(e.unicode()+pos+1, 2);
		     bool ok;
	             unsigned char val = buffer.string().toInt(&ok, 16);
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

void HTMLGenericFormElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_NAME:
	_name = attr->value();
	break;
    default:
	HTMLPositionedElementImpl::parseAttribute(attr);
    }
}

HTMLFormElementImpl *HTMLGenericFormElementImpl::getForm() const
{
    NodeImpl *p = parentNode();
    while(p)
    {
	if( p->id() == ID_FORM )
	    return static_cast<HTMLFormElementImpl *>(p);
	p = p->parentNode();
    }
    printf("couldn't find form!\n");
    return 0;
}

void HTMLGenericFormElementImpl::setPos( int xPos, int yPos )
{
    x = xPos;
    y = yPos;
    badPos = true;
}

void HTMLGenericFormElementImpl::setXPos( int xPos )
{
    x = xPos;
    badPos = true;
}

void HTMLGenericFormElementImpl::setYPos( int yPos )
{
    y = yPos;
    badPos = true;
}

void HTMLGenericFormElementImpl::print(QPainter *p, int, int,
				       int, int, int tx, int ty)
{
    if(badPos && view && w)
    {
	tx += x;
	ty += y - ascent;
	view->addChild(w, tx, ty);
	// ### we have to set the background somehow...
	w->show();
	badPos = false;
    }
}

void HTMLGenericFormElementImpl::attach(KHTMLWidget *_view)
{
    view = _view;
}

void HTMLGenericFormElementImpl::detach()
{
    if(w) delete w;
    w = 0;
    view = 0;
}

// -------------------------------------------------------------------------

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLButtonElementImpl::~HTMLButtonElementImpl()
{
}

const DOMString HTMLButtonElementImpl::nodeName() const
{
    return "BUTTON";
}

ushort HTMLButtonElementImpl::id() const
{
    return ID_BUTTON;
}

bool HTMLButtonElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLButtonElementImpl::setDisabled( bool )
{
}

long HTMLButtonElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLButtonElementImpl::setTabIndex( long  )
{
}

DOMString HTMLButtonElementImpl::type() const
{
    // ###
    return DOMString();
}

// -------------------------------------------------------------------------

HTMLFieldSetElementImpl::HTMLFieldSetElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLFieldSetElementImpl::~HTMLFieldSetElementImpl()
{
}

const DOMString HTMLFieldSetElementImpl::nodeName() const
{
    return "FIELDSET";
}

ushort HTMLFieldSetElementImpl::id() const
{
    return ID_FIELDSET;
}

// -------------------------------------------------------------------------

HTMLInputElementImpl::HTMLInputElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _type = TEXT;
    _checked = false;
    _disabled = false;
    _maxLen = 0;
    _size = 20;
    _pixmap = 0;
    _clicked = false;

    w = 0;
    view = 0;
    badPos = true;
}

HTMLInputElementImpl::~HTMLInputElementImpl()
{
    if(w) delete w;
}

const DOMString HTMLInputElementImpl::nodeName() const
{
    return "INPUT";
}

ushort HTMLInputElementImpl::id() const
{
    return ID_INPUT;
}

bool HTMLInputElementImpl::defaultChecked() const
{
    // ###
    return false;
}

void HTMLInputElementImpl::setDefaultChecked( bool )
{
}

void HTMLInputElementImpl::setChecked(bool b)
{
    if(_type == RADIO)
    {
	QRadioButton *button = static_cast<QRadioButton *>(w);
	if(button) button->setChecked(b);
    }
}

void HTMLInputElementImpl::setDisabled( bool )
{
}

void HTMLInputElementImpl::setMaxLength( long  )
{
}

bool HTMLInputElementImpl::readOnly() const
{
    // ###
    return false;
}

void HTMLInputElementImpl::setReadOnly( bool )
{
}

long HTMLInputElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLInputElementImpl::setTabIndex( long  )
{
}

DOMString HTMLInputElementImpl::type() const
{
    // ###
    return DOMString();
}

void HTMLInputElementImpl::blur(  )
{
}

void HTMLInputElementImpl::focus(  )
{
}

void HTMLInputElementImpl::select(  )
{
}

void HTMLInputElementImpl::click(  )
{
}

void HTMLInputElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_TYPE:
	if ( strcasecmp( attr->value(), "text" ) == 0 )
	    _type = TEXT;
	else if ( strcasecmp( attr->value(), "password" ) == 0 )
	    _type = PASSWORD;
	else if ( strcasecmp( attr->value(), "checkbox" ) == 0 )
	    _type = CHECKBOX;
	else if ( strcasecmp( attr->value(), "radio" ) == 0 )
	    _type = RADIO;
	else if ( strcasecmp( attr->value(), "submit" ) == 0 )
	    _type = SUBMIT;
	else if ( strcasecmp( attr->value(), "reset" ) == 0 )
	    _type = RESET;
	else if ( strcasecmp( attr->value(), "file" ) == 0 )
	    _type = FILE;
	else if ( strcasecmp( attr->value(), "hidden" ) == 0 )
	    _type = HIDDEN;
	else if ( strcasecmp( attr->value(), "image" ) == 0 )
	    _type = IMAGE;
	else if ( strcasecmp( attr->value(), "button" ) == 0 )
	    _type = BUTTON;
	break;
    case ATTR_VALUE:
	_value = attr->value();
	currValue = _value.string();
	break;
    case ATTR_CHECKED:
	_checked = true;
	break;
    case ATTR_DISABLED:
	_disabled = true;
	break;
    case ATTR_MAXLENGTH:
	_maxLen = attr->val()->toInt();
	break;
    case ATTR_SIZE:
	_size = attr->val()->toInt();
	break;
    case ATTR_SRC:
	_src = attr->value();
	break;
    case ATTR_READONLY:
    case ATTR_ALT:
    case ATTR_USEMAP:
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLInputElementImpl::attach(KHTMLWidget *_view)
{
    printf("inputElement::attach()\n");
    view = _view;
    switch(_type)
    {
    case TEXT:
    case PASSWORD:
    {
	QLineEdit *l = new QLineEdit(view->viewport());
	if ( _type == PASSWORD )
	    l->setEchoMode ( QLineEdit::Password );
	l->setText(_value.string());
	w = l;
	QObject::connect(l, SIGNAL(returnPressed()),
			 this, SLOT(slotReturnPressed()));
	QObject::connect(l, SIGNAL(textChanged(const QString &)),
			 this, SLOT(slotTextChanged(const QString &)));
	break;
    }
    case CHECKBOX:
    {
	QCheckBox *b = new QCheckBox(view->viewport());
	b->setChecked(_checked);
	w = b;
	break;
    }
    case RADIO:
    {
	QRadioButton *b = new QRadioButton(view->viewport());
	b->setChecked(_checked);
	w = b;
	QObject::connect(b, SIGNAL(clicked()), this, SLOT(slotClicked()));
	break;
    }
    case SUBMIT:
    {
	QPushButton *b = new QPushButton(view->viewport());
	w = b;
	if(_value != 0)
	    b->setText(_value.string());
	else
	    b->setText("Submit Query");
	QObject::connect(b, SIGNAL(clicked()), this, SLOT(slotSubmit()));
	view->addChild(w, 0, 0);
	break;
    }
    case RESET:
    {
	QPushButton *b = new QPushButton(view->viewport());
	w = b;
	if ( _value.length() )
	    b->setText( _value.string() );
	else
	    b->setText( "Reset" );
	view->addChild(w, 0, 0);
	QObject::connect(b, SIGNAL(clicked()), form(), SLOT(slotReset()));
	break;
    }
    case FILE:
    case HIDDEN:
	// ###
	break;
    case IMAGE:
    {
	QPushButton *b = new QPushButton(view->viewport());
	w = b;
	b->setText("IMG");
	if(_src != 0) //###
	    /*_src =*/ document->requestImage(this, _src);
	break;
    }
    case BUTTON:
    {
	QPushButton *b = new QPushButton(view->viewport());
	w = b;
	if ( _value.length() )
	    b->setText( _value.string() );
	else
	    b->setText( "" );
	view->addChild(w, 0, 0);
	break;
    }
    }

    if(w && _disabled) w->setEnabled(false);
}

void HTMLInputElementImpl::layout( bool )
{
    width = availableWidth;
    if(!width) return;

    printf("inputElement::layout()\n");
    switch(_type)
    {
    case TEXT:
    case PASSWORD:
    {
	_style->font.family = pSettings->fixedFontFace;
	w->setFont( *getFont() );
	if ( _maxLen > 0 )
	    ((QLineEdit *)w)->setMaxLength( _maxLen );

	QFontMetrics m = w->fontMetrics();
	QSize size( _size * m.maxWidth() + 2, w->sizeHint().height());
	w->resize( size );
	descent = 5;
	ascent = size.height() - descent;
	width = size.width();
	break;
    }
    case CHECKBOX:
    case RADIO:
    case IMAGE:
    case BUTTON:
	w->resize( w->sizeHint() );
	descent = 5;
	ascent = w->height() - descent;
	width = w->width() + 6;
	break;
    case SUBMIT:
    case RESET:
    {
	w->setFont( *getFont() );
	w->resize(w->sizeHint());
	descent = 5;
	ascent = w->height() - descent;
	width = w->width();
	break;
    }
    case FILE:
    case HIDDEN:
	ascent = descent = width = 0;
	break;
    }

    setLayouted();
}

void HTMLInputElementImpl::setPixmap( QPixmap *p )
{
    _pixmap = p;
    if(_type == IMAGE)
	static_cast<QPushButton *>(w)->setPixmap(*p);
}

void HTMLInputElementImpl::pixmapChanged( QPixmap *p )
{
    _pixmap = p;
    if(_type == IMAGE)
	static_cast<QPushButton *>(w)->setPixmap(*p);
}

QString HTMLInputElementImpl::encoding()
{
    QString _encoding;

    if(!_name.length()) return _encoding;

    switch(_type)
    {
    case TEXT:
    case PASSWORD:
	_encoding = encodeString( _name.string() );
	_encoding += '=';
	_encoding += encodeString( currValue ); // FIXME ###
	break;
    case CHECKBOX:
	if ( ((QCheckBox *)w)->isChecked() )
	{
	    _encoding = encodeString( _name.string() );
	    _encoding += '=';
	    _encoding += encodeString( currValue );
	}
	break;
    case RADIO:
	if ( ((QRadioButton *)w)->isChecked() )
	{
	    _encoding = encodeString( _name.string() );
	    _encoding += '=';
	    _encoding += encodeString( currValue );
	}
	break;
    case IMAGE:
    case BUTTON:
	break;
    case SUBMIT:
	if ( _clicked )
	{
	    _encoding = encodeString( _name.string() );
	    _encoding += '=';
	    _encoding += encodeString( currValue );
	}
	break;
    case RESET:
	break;
    case FILE:
	break;
    case HIDDEN:
	_encoding = encodeString( _name.string() );
	_encoding += '=';
	_encoding += encodeString( currValue );
	break;
    }

    printf("this:enc = %s\n", _encoding.ascii());
    return _encoding;
}

void HTMLInputElementImpl::slotTextChanged( const QString &s)
{
    currValue = s;
}

void HTMLInputElementImpl::slotReturnPressed()
{
}

void HTMLInputElementImpl::slotSubmit()
{
    _clicked = true;
    if(form()) _form->slotSubmit();
}

void HTMLInputElementImpl::slotClicked()
{
    if( _type == RADIO && form() )
	_form->radioClicked( this, _name );
}

void HTMLInputElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(InputElement)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;

    if(w)
	minWidth = w->width();
    else
	printf("InputElement: no Widget!!!\n");
    maxWidth = minWidth;

    if(availableWidth && minWidth > availableWidth)
	if(_parent) _parent->updateSize();
}

// -------------------------------------------------------------------------

HTMLLabelElementImpl::HTMLLabelElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLLabelElementImpl::~HTMLLabelElementImpl()
{
}

const DOMString HTMLLabelElementImpl::nodeName() const
{
    return "LABEL";
}

ushort HTMLLabelElementImpl::id() const
{
    return ID_LABEL;
}

// -------------------------------------------------------------------------

HTMLLegendElementImpl::HTMLLegendElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLLegendElementImpl::~HTMLLegendElementImpl()
{
}

const DOMString HTMLLegendElementImpl::nodeName() const
{
    return "LEGEND";
}

ushort HTMLLegendElementImpl::id() const
{
    return ID_LEGEND;
}

// -------------------------------------------------------------------------

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _multiple = false;
    _disabled = false;
    w = 0;
    view = 0;
    _size = 1;
}

HTMLSelectElementImpl::~HTMLSelectElementImpl()
{
}

const DOMString HTMLSelectElementImpl::nodeName() const
{
    return "SELECT";
}

ushort HTMLSelectElementImpl::id() const
{
    return ID_SELECT;
}

DOMString HTMLSelectElementImpl::type() const
{
    // ###
    return DOMString();
}

long HTMLSelectElementImpl::selectedIndex() const
{
    // ###
    return 0;
}

void HTMLSelectElementImpl::setSelectedIndex( long  )
{
}

long HTMLSelectElementImpl::length() const
{
    // ###
    return 0;
}

bool HTMLSelectElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLSelectElementImpl::setDisabled( bool )
{
}


void HTMLSelectElementImpl::setMultiple( bool )
{
}

void HTMLSelectElementImpl::setSize( long  )
{
}

long HTMLSelectElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLSelectElementImpl::setTabIndex( long  )
{
}

void HTMLSelectElementImpl::add( const HTMLElement &/*element*/, const HTMLElement &/*before*/ )
{
}

void HTMLSelectElementImpl::remove( long /*index*/ )
{
}

void HTMLSelectElementImpl::blur(  )
{
}

void HTMLSelectElementImpl::focus(  )
{
}

void HTMLSelectElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_SIZE:
	_size = attr->val()->toInt();
	break;
    case ATTR_DISABLED:
	_disabled = true;
	break;
    case ATTR_MULTIPLE:
	_multiple = true;
	break;
    case ATTR_READONLY:
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// ###
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLSelectElementImpl::attach(KHTMLWidget *_view)
{
    view = _view;

    QSize size;

    if ( _size > 1 || _multiple )
    {
	w = new QListBox( view->viewport() );
	size.setWidth( 150 );
	size.setHeight( 20 * _size );
	ascent = 25;
	descent = size.height() - ascent;
	((QListBox *)w)->setMultiSelection( _multiple );
    }
    else
    {
	w = new QComboBox( FALSE, view->viewport() );
	size.setWidth( 150 );
	size.setHeight( 25 );
	descent = 5;
	ascent = size.height() - descent;
    }

    width = size.width();
    w->resize(size);

    if(w && _disabled) w->setEnabled(false);
}

void HTMLSelectElementImpl::layout( bool )
{
    if(!w) return;

    QStack<NodeImpl> nodeStack;
    NodeImpl *current = _first;
    while(1)
    {
	if(!current)
	{
	    if(nodeStack.isEmpty()) break;
	    current = nodeStack.pop();
	    current = current->nextSibling();
	}
	else
	{
	    switch(current->id())
	    {
	    case ID_OPTGROUP:
	    case ID_OPTION:
		break;
	    case ID_TEXT:
	    {
		DOMStringImpl *text = static_cast<TextImpl *>(current)->string();
		if(_size > 1 || _multiple)
		    static_cast<QListBox *>(w)->insertItem(QConstString(text->s, text->l).string());
		else
		    static_cast<QComboBox *>(w)->insertItem(QConstString(text->s, text->l).string());
		break;
	    }
	    default:
		break;
	    }

	    NodeImpl *child = current->firstChild();
	    if(child)
	    {	
		nodeStack.push(current);
		current = child;
	    }
	    else
	    {
		current = current->nextSibling();
	    }
	}
    }

    setLayouted();
}

void HTMLSelectElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(InputElement)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;

    minWidth = 0;
    if(w)
	minWidth = w->width();
    else
	printf("InputElement: no Widget!!!\n");
    maxWidth = minWidth;

    if(availableWidth && minWidth > availableWidth)
	if(_parent) _parent->updateSize();
}

void HTMLSelectElementImpl::close()
{
    setParsing(false);
    layout();
}

// -------------------------------------------------------------------------

HTMLOptGroupElementImpl::HTMLOptGroupElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLOptGroupElementImpl::~HTMLOptGroupElementImpl()
{
}

const DOMString HTMLOptGroupElementImpl::nodeName() const
{
    return "OPTGROUP";
}

ushort HTMLOptGroupElementImpl::id() const
{
    return ID_OPTGROUP;
}

bool HTMLOptGroupElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLOptGroupElementImpl::setDisabled( bool )
{
}

// -------------------------------------------------------------------------

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLOptionElementImpl::~HTMLOptionElementImpl()
{
}

const DOMString HTMLOptionElementImpl::nodeName() const
{
    return "OPTION";
}

ushort HTMLOptionElementImpl::id() const
{
    return ID_OPTION;
}

bool HTMLOptionElementImpl::defaultSelected() const
{
    // ###
    return false;
}

void HTMLOptionElementImpl::setDefaultSelected( bool )
{
}

DOMString HTMLOptionElementImpl::text() const
{
    // ###
    return DOMString();
}

long HTMLOptionElementImpl::index() const
{
    // ###
    return 0;
}

void HTMLOptionElementImpl::setIndex( long  )
{
}

bool HTMLOptionElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLOptionElementImpl::setDisabled( bool )
{
}

bool HTMLOptionElementImpl::selected() const
{
    // ###
    return false;
}

// -------------------------------------------------------------------------

HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _rows = _cols = 0;
    _disabled = false;

}

HTMLTextAreaElementImpl::~HTMLTextAreaElementImpl()
{
}

const DOMString HTMLTextAreaElementImpl::nodeName() const
{
    return "TEXTAREA";
}

ushort HTMLTextAreaElementImpl::id() const
{
    return ID_TEXTAREA;
}

long HTMLTextAreaElementImpl::cols() const
{
    // ###
    return 0;
}

void HTMLTextAreaElementImpl::setCols( long  )
{
}

bool HTMLTextAreaElementImpl::disabled() const
{
    // ###
    return false;
}

void HTMLTextAreaElementImpl::setDisabled( bool )
{
}

bool HTMLTextAreaElementImpl::readOnly() const
{
    // ###
    return false;
}

void HTMLTextAreaElementImpl::setReadOnly( bool )
{
}

long HTMLTextAreaElementImpl::rows() const
{
    // ###
    return 0;
}

void HTMLTextAreaElementImpl::setRows( long  )
{
}

long HTMLTextAreaElementImpl::tabIndex() const
{
    // ###
    return 0;
}

void HTMLTextAreaElementImpl::setTabIndex( long  )
{
}

DOMString HTMLTextAreaElementImpl::type() const
{
    // ###
    return DOMString();
}

void HTMLTextAreaElementImpl::blur(  )
{
}

void HTMLTextAreaElementImpl::focus(  )
{
}

void HTMLTextAreaElementImpl::select(  )
{
}

void HTMLTextAreaElementImpl::parseAttribute(Attribute *attr)
{
    switch(attr->id)
    {
    case ATTR_ROWS:
	_rows = attr->val()->toInt();
	break;
    case ATTR_COLS:
	_cols = attr->val()->toInt();
	break;
    case ATTR_DISABLED:
	_disabled = true;
	break;
    case ATTR_READONLY:
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// ###
	break;
    case ATTR_NAME:
	// handled by parent...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLTextAreaElementImpl::attach(KHTMLWidget *_view)
{
    view = _view;
    QMultiLineEdit *edit = new QMultiLineEdit(view->viewport());
    w = edit;
    if(_first && _first->id() == ID_TEXT)
    {
	TextImpl *t = static_cast<TextImpl *>(_first);
	edit->setText(QString(t->string()->s, t->string()->l));
    }
    if(edit && _disabled) edit->setEnabled(false);
}

void HTMLTextAreaElementImpl::layout( bool )
{
    width = availableWidth;
    if(!width) return;

    _style->font.family = pSettings->fixedFontFace;
    w->setFont( *getFont() );

    QFontMetrics m = w->fontMetrics();
    QSize size( _cols * m.maxWidth() + 2, m.height()*_rows + 6);
    w->resize( size );
    descent = 5;
    ascent = size.height() - descent;
    width = size.width();

    setLayouted();
}

void HTMLTextAreaElementImpl::calcMinMaxWidth()
{
#ifdef DEBUG_LAYOUT
    printf("%s(InputElement)::calcMinMaxWidth() known=%d\n", nodeName().string().ascii(), minMaxKnown());
#endif

    if(minMaxKnown()) return;

    if(w)
	minWidth = w->width();
    else
	printf("InputElement: no Widget!!!\n");
    maxWidth = minWidth;

    if(availableWidth && minWidth > availableWidth)
	if(_parent) _parent->updateSize();
}
