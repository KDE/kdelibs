/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
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
#include "html_formimpl.h"

#include "khtmlview.h"
#include "khtml_part.h"
#include "html_documentimpl.h"

#include "htmlhashes.h"

#include "css/cssstyleselector.h"
#include "css/cssproperties.h"

#include "rendering/render_form.h"

#include <kdebug.h>
#include <kmimetype.h>
#include <netaccess.h>
#include <qfile.h>

#include <iostream.h>

using namespace DOM;
using namespace khtml;

template class QList<khtml::RenderFormElement>;

HTMLFormElementImpl::HTMLFormElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    post = false;
    multipart = false;
    _enctype = "application/x-www-form-urlencoded";
    _boundary = "----------0xKhTmLbOuNdArY";
}

HTMLFormElementImpl::~HTMLFormElementImpl()
{
    QListIterator<HTMLGenericFormElementImpl> it(formElements);
    for (; it.current(); ++it)
	it.current()->setForm(0);
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


QByteArray HTMLFormElementImpl::formData()
{
    kdDebug( 6030 ) << "form: formData()" << endl;

    // This entire function is a big hack now... but it seems to be
    // necessary.  The core problem is that in multipart forms, it's
    // possible (likely) that binary formats will be included as a
    // part of the form.  Unfortunately, this means that we cannot
    // store the result in QString as that will hit the first NULL
    // character and screw everything up.  QCString works better as
    // it's just a glorified QByteArray anyway.  However, it also
    // doesn't like NULL characters.
    //
    // So, to actually include binary data inside a normal string, we
    // need to do a bunch of ugly memcpy's.... it does work, though.
    // - Kurt Granroth

    QByteArray form_data(0);
    bool first = true;
    QCString enc_string = ""; // used for non-multipart data

    HTMLGenericFormElementImpl *current = formElements.first();
    for( ; current; current = formElements.next() )
    {
	kdDebug( 6030 ) << "getting data from " << current << " name = " << current->nodeName().string() << endl;

        if (!current->disabled()) {

            QCString enc(current->encoding());
            kdDebug( 6030 ) << "current encoding = " << enc.data() << endl;

            if (!multipart)
            {
                if(!enc.length())
                    continue;

                if(!first)
                    enc_string += '&';

                enc_string += enc.data();

                first = false;
            }
            else
            {
                // if there is no name to this part, we skip it
                if ( current->name() == 0 )
                    continue;

                // hack to see if the enc data is really a cstring
                int enc_size;
                QCString hack(enc);
                if (hack.length() == (hack.size() - 1))
                    enc_size = hack.length();
                else
                    enc_size = hack.size();
                int old_size = form_data.size();


                QCString str(("--" + _boundary.string() + "\r\n").ascii());
                str += "Content-Disposition: form-data; ";
                str += ("name=\"" + current->name().string() + "\"").ascii();

                // if the current type is FILE, then we also need to
                // include the filename *and* the file type

               if (current->nodeType() == Node::ELEMENT_NODE && current->id() == ID_INPUT &&
                   static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::FILE)
                {
                    str += ("; filename=\"" + static_cast<HTMLInputElementImpl*>(current)->value().string() + "\"\r\n").ascii();
                    str += "Content-Type: ";
                    KMimeType::Ptr ptr = KMimeType::findByURL(KURL(static_cast<HTMLInputElementImpl*>(current)->value().string()), 0, false);
                    str += ptr->name().ascii();
                }


                str += "\r\n\r\n";

                // this is where it gets ugly.. we have to memcpy the
                // text part to the form.. then memcpy the (possibly
                // binary) data.  yuck!
                form_data.resize( old_size + str.size() + enc_size + 1);
                memcpy(form_data.data() + old_size, str.data(), str.length());
                memcpy(form_data.data() + old_size + str.length(), enc, enc_size);
                form_data[form_data.size()-2] = '\r';
                form_data[form_data.size()-1] = '\n';
            }
        }
    }
    if (multipart)
        enc_string = ("--" + _boundary.string() + "--\r\n").ascii();

    int old_size = form_data.size();
    form_data.resize( form_data.size() + enc_string.length() );
    memcpy(form_data.data() + old_size, enc_string.data(), enc_string.length() );

    kdDebug( 6030 ) << "End encoding size = " << form_data.size() << endl;

    return form_data;
}

void HTMLFormElementImpl::setEnctype( const DOMString& type )
{
    _enctype = type;
    if ( strcasecmp( type, "multipart/form-data" ) == 0 )
        multipart = true;
}

void HTMLFormElementImpl::setBoundary( const DOMString& bound )
{
    _boundary = bound;
}

void HTMLFormElementImpl::submit(  )
{
    kdDebug( 6030 ) << "submit pressed!" << endl;
    if(!view) return;

    DOMString script = getAttribute(ATTR_ONSUBMIT);
    if (!script.isNull())
	view->part()->executeScript(script.string());

    QByteArray form_data = formData();

    kdDebug( 6030 ) << "formdata = " << form_data.data() << endl << "post = " << post << endl << "multipart = " << multipart << endl;
    if(post)
    {
        view->part()->submitForm( "post", url.string(), form_data,
                                  target.string(),
                                  enctype().string(),
                                  boundary().string() );
    }
    else
        view->part()->submitForm( "get", url.string(), form_data,
                                  target.string() );
}

void HTMLFormElementImpl::reset(  )
{
    kdDebug( 6030 ) << "reset pressed!" << endl;

    DOMString script = getAttribute(ATTR_ONRESET);
    if (!script.isNull())
	view->part()->executeScript(script.string());

    HTMLGenericFormElementImpl *current = formElements.first();
    while(current)
    {
	current->reset();
	current = formElements.next();
    }
    if (document->isHTMLDocument())
	static_cast<HTMLDocumentImpl*>(document)->updateRendering();
}

void HTMLFormElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
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
        setEnctype( attr->value() );
	break;
    case ATTR_ACCEPT_CHARSET:
    case ATTR_ACCEPT:
	// ignore these for the moment...
	break;
    default:
	HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLFormElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

void HTMLFormElementImpl::detach()
{
    view = 0;
    HTMLElementImpl::detach();
}

void HTMLFormElementImpl::radioClicked( HTMLGenericFormElementImpl *caller )
{
    // ### make this work for form elements that don't have renders
    if(!view) return;

    HTMLGenericFormElementImpl *current;
    for (current = formElements.first(); current; current = formElements.next())
    {
	if (current->id() == ID_INPUT &&
	    static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::RADIO &&
	    current != caller && current->name() == caller->name()) {
	    static_cast<HTMLInputElementImpl*>(current)->setChecked(false);
	}
    }
}

void HTMLFormElementImpl::maybeSubmit()
{
    if(!view) return;

    int le = 0;
    int total = 0;

    // count number of LineEdits / total number

    HTMLGenericFormElementImpl *current;
    for (current = formElements.first(); current; current = formElements.next()) {
	if (!current->disabled() && !current->readOnly()) {
	    if (current->id() == ID_INPUT &&
	        (static_cast<HTMLInputElementImpl*>(current)->type() == HTMLInputElementImpl::TEXT ||
	         static_cast<HTMLInputElementImpl*>(current)->type() == HTMLInputElementImpl::PASSWORD))
		le++;

            // we're not counting hidden input's here, as they're not enabled (### check this)
	    total++;
	}
    }

    // if there's only one lineedit or only one possibly successful one, submit
    if (le < 2 || total < 2)
        submit();
}


void HTMLFormElementImpl::registerFormElement(HTMLGenericFormElementImpl *e)
{
    formElements.append(e);
}

void HTMLFormElementImpl::removeFormElement(HTMLGenericFormElementImpl *e)
{
  // ### make sure this get's called, when formElements get removed from
  // the document tree
    formElements.remove(e);
}


// -------------------------------------------------------------------------

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLElementImpl(doc)
{
    _form = f;
    if (_form)
	_form->registerFormElement(this);

    view = 0;
    m_disabled = m_readOnly = false;

}

HTMLGenericFormElementImpl::HTMLGenericFormElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    _form = getForm();
    if (_form)
	_form->registerFormElement(this);

    view = 0;
    m_disabled = m_readOnly = false;
}

HTMLGenericFormElementImpl::~HTMLGenericFormElementImpl()
{
    if (_form)
	_form->removeFormElement(this);
}

void HTMLGenericFormElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_NAME:
	_name = attr->value();
	break;
    case ATTR_DISABLED:
	m_disabled = attr->val() != 0;
	break;
    case ATTR_READONLY:
        m_readOnly = attr->val() != 0;
    default:
	HTMLElementImpl::parseAttribute(attr);
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
    kdDebug( 6030 ) << "couldn't find form!" << endl;
    return 0;
}


void HTMLGenericFormElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

void HTMLGenericFormElementImpl::detach()
{
    view = 0;
    HTMLElementImpl::detach();
}

QCString HTMLGenericFormElementImpl::encodeString( QString e )
{
    static const char *safe = "$-._!*(),"; /* RFC 1738 */
    unsigned pos = 0;
    QString encoded;

    while ( pos < e.length() )
    {
        QChar c = e[pos];

        if ( (( c >= 'A') && ( c <= 'Z')) ||
             (( c >= 'a') && ( c <= 'z')) ||
             (( c >= '0') && ( c <= '9')) ||
             (strchr(safe, c.latin1()))
            )
        {
            encoded += c;
        }
        else if ( c.latin1() == ' ' )
        {
            encoded += '+';
        }
        else if ( c.latin1() == '\n' )
        {
            encoded += QString::fromLatin1("%0D%0A");
        }
        else if ( c.latin1() != '\r' )
        {
            // HACK! sprintf( buffer, "%%%02X", (int)c );
            //       encoded += buffer;

            // Instead of this hack, use KURL's method.
            // For non-latin1 characters, the (int) cast can lead to wrong values.
            // (even negative ones!) (David)
            encoded += KURL::encode_string( QString(c) );
        }
        pos++;
    }

    return encoded.latin1();
}

// -------------------------------------------------------------------------

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    _clicked = false;
    _type = SUBMIT;
    dirty = true;
}

HTMLButtonElementImpl::HTMLButtonElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    _clicked = false;
    _type = SUBMIT;
    dirty = true;
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

void HTMLButtonElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_TYPE:
	if ( strcasecmp( attr->value(), "submit" ) == 0 )
	    _type = SUBMIT;
	else if ( strcasecmp( attr->value(), "reset" ) == 0 )
	    _type = RESET;
	else if ( strcasecmp( attr->value(), "button" ) == 0 )
	    _type = BUTTON;
	break;
    case ATTR_VALUE:
	_value = attr->value();
	currValue = _value.string();
	break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
	// ignore for the moment
	break;
//    case ATTR_NAME:
	// handled by parent class...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLButtonElementImpl::attach(KHTMLView *_view)
{
    view = _view;
    HTMLElementImpl::attach(_view);
}

// -------------------------------------------------------------------------

HTMLFieldSetElementImpl::HTMLFieldSetElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

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
    m_checked = false;
    _maxLen = -1;
    _size = 20;
    _clicked = false;
    m_filename = "";

    view = 0;
}

HTMLInputElementImpl::HTMLInputElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    _type = TEXT;
    m_checked = false;
    _maxLen = -1;
    _size = 20;
    _clicked = false;
    m_filename = "";

    view = 0;
}

HTMLInputElementImpl::~HTMLInputElementImpl()
{
    ownerDocument()->removeElement(this);
}

const DOMString HTMLInputElementImpl::nodeName() const
{
    return "INPUT";
}

ushort HTMLInputElementImpl::id() const
{
    return ID_INPUT;
}

long HTMLInputElementImpl::tabIndex() const
{
    // ###
    return 0;
}

DOMString HTMLInputElementImpl::type() const
{
    // HTML DTD suggests this is supposed to be uppercase
    switch (_type) {
	case TEXT: return "TEXT";
	case PASSWORD: return "PASSWORD";
	case CHECKBOX: return "CHECKBOX";
	case RADIO: return "RADIO";
	case SUBMIT: return "SUBMIT";
	case RESET: return "RESET";
	case FILE: return "FILE";
	case HIDDEN: return "HIDDEN";
	case IMAGE: return "IMAGE";
	case BUTTON: return "BUTTON";
	default: return "";
    }
}

QString HTMLInputElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLInputElementImpl::blur(  )
{
    kdDebug( 6030 ) << "HTMLInputElementImpl::blur(  )" << endl;

}

void HTMLInputElementImpl::focus(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::focus(  )" << endl;

}

void HTMLInputElementImpl::select(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::select(  )" << endl;
}

void HTMLInputElementImpl::click(  )
{
    kdDebug( 6030 ) << " HTMLInputElementImpl::click(  )" << endl;
}

void HTMLInputElementImpl::parseAttribute(AttrImpl *attr)
{
    // ### IMPORTANT: check that the type can't be changed after the first time
    // otherwise a javascript programmer may be able to set a text field's value
    // to something like /etc/passwd and then change it to a file field
    // ### also check that input defaults to something - text perhaps?
    switch(attr->attrId)
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
	break;
    case ATTR_CHECKED:
	setChecked(attr->val() != 0);
	break;
    case ATTR_MAXLENGTH:
	_maxLen = attr->val() ? attr->val()->toInt() : -1;
	break;
    case ATTR_SIZE:
	_size = attr->val() ? attr->val()->toInt() : 20;
	break;
    case ATTR_SRC:
	_src = attr->value();
	break;
    case ATTR_ALT:
    case ATTR_USEMAP:
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ### ignore for the moment
	break;
    case ATTR_ALIGN:
	addCSSProperty(CSS_PROP_TEXT_ALIGN, attr->value(), false);
	break;
//    case ATTR_NAME:
	// handled by parent class...
    case ATTR_WIDTH:
	addCSSProperty(CSS_PROP_WIDTH, attr->value(), false);
	break;
    case ATTR_HEIGHT:
	addCSSProperty(CSS_PROP_HEIGHT, attr->value(), false);
	break;
	
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLInputElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	RenderFormElement *f = 0;
	
	switch(_type)
	{
	case TEXT:
	{
	    f = new RenderLineEdit(view, this);
	    break;
	}
	case PASSWORD:
	{
	    f = new RenderLineEdit(view, this);
	    break;
	}
	case CHECKBOX:
	{
	    f = new RenderCheckBox(view, this);
	    f->setChecked(m_checked);
	    break;
	}
	case RADIO:
	{
	    f = new RenderRadioButton(view, this);
	    f->setChecked(m_checked);
	    break;
	}
	case SUBMIT:
	{
	    f = new RenderSubmitButton(view, this);
	    break;
	}
	case IMAGE:
	{
	    RenderImageButton *i = new RenderImageButton(view, this);
	    i->setImageUrl(_src, static_cast<HTMLDocumentImpl *>(document)->URL(),
	                   static_cast<HTMLDocumentImpl *>(document)->docLoader());
	    f = i;
	    break;
	}
	case RESET:
	{
	    f = new RenderResetButton(view, this);
	    break;
	}
	case FILE:
        {
            f = new RenderFileButton(view, this);
            break;
        }
	case HIDDEN:
        {
            f = new RenderHiddenButton(view, this);
            break;
        }
	case BUTTON:
	{
	    f = new RenderPushButton(view, this);
	    break;
	}
	}
	if (f)
	{
	    f->setValue(_value); // ### remove
	    f->setEnabled(!m_disabled); // ### remove render's knowledge of this
            f->setReadonly(m_readOnly); // ### remove render's knowledge of this
	
    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
            kdDebug( 6030 ) << "adding " << m_render->renderName() << " as child of " << r->renderName() << endl;
            QString state = document->registerElement(this);
            if ( !state.isEmpty())
            {
               kdDebug( 6030 ) << "Restoring InputElem name=" << _name.string() <<
                            " state=" << state << endl;
               f->restoreState( state );
            }

	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
    NodeBaseImpl::attach(_view);
}


QCString HTMLInputElementImpl::encoding()
{
    cerr << "HTMLInputElementImpl::encoding()\n";
    QCString encoding = "";
    if (_name.isEmpty()) return encoding;
    switch (_type) {
	case TEXT:
	case PASSWORD:
	case HIDDEN:
	{
	    if ( _form->enctype() == "application/x-www-form-urlencoded" )
		encoding = encodeString( _name.string() ) + '=' + encodeString( _value.string() );
	    else
		encoding = _value.string().latin1();
	    break;
	}
	case CHECKBOX:
	{
	    if (checked())
	    {
		if ( _form->enctype() == "application/x-www-form-urlencoded" )
		    encoding = encodeString( _name.string() ) + '=' +
		    		( _value.isEmpty() ? QCString("on") : encodeString( _value.string() ) );
		else
		    encoding = ( _value.isEmpty() ? QCString("on") : QCString(_value.string().latin1()) );
	    }
	    break;
	}
	case RADIO:
	{
	    if (checked())
	    {
		if ( _form->enctype() == "application/x-www-form-urlencoded" )
		    encoding = encodeString( _name.string() ) + '=' + encodeString( _value.string() );
		else
		    encoding = _value.string().latin1();
	    }
	    break;
	}
	case SUBMIT:
	{
	    if (m_render && static_cast<RenderSubmitButton*>(m_render)->clicked())
	    {
		QString val = _value.isNull() ? static_cast<RenderSubmitButton*>(m_render)->defaultLabel() :
		              value().string();
		if ( _form->enctype() == "application/x-www-form-urlencoded" )
		    encoding = encodeString( _name.string() ) + '=' + encodeString( val );
		else
		    encoding = val.latin1();
	    }
	    if (m_render)
		static_cast<RenderSubmitButton*>(m_render)->setClicked(false);
	    break;
	}
	case FILE: {
//	    m_value = m_edit->text();

	    if ( _form->enctype() == "application/x-www-form-urlencoded" )
		encoding = encodeString( _name.string() ) + '=' + encodeString( m_filename.string() );
	    else
	    {
		QString local;
		if ( !KIO::NetAccess::download(KURL(m_filename.string()), local) );
		{
		    QFile file(local);
		    if (file.open(IO_ReadOnly))
		    {
			uint size = file.size();
			char *buf = new char[ size ];
			file.readBlock( buf, size );
			file.close();

			encoding.duplicate(buf, size);

			delete[] buf;
		    }
		    KIO::NetAccess::removeTempFile( local );
		}
	    }
	    break;
	}	
	default:
	break;
    }
    cerr << "HTMLInputElementImpl::encoding(): returning \"" << encoding << "\"\n";
    return encoding;
}

void HTMLInputElementImpl::saveDefaultAttrs()
{
    _defaultValue = getAttribute(ATTR_VALUE);
    _defaultChecked = (getAttribute(ATTR_CHECKED) != 0);
}

void HTMLInputElementImpl::reset()
{
    cerr << "HTMLInputElementImpl::reset()\n";
    setAttribute(ATTR_VALUE,_defaultValue);
    setAttribute(ATTR_CHECKED,_defaultChecked);
    if ((_type == SUBMIT || _type == RESET || _type == BUTTON || _type == IMAGE) && m_render)
	static_cast<RenderSubmitButton*>(m_render)->setClicked(false);
}

void HTMLInputElementImpl::setChecked(bool _checked)
{
    m_checked = _checked;
    if (_type == RADIO && _form && m_render && m_checked)
	_form->radioClicked(this);
    setChanged(true);
}

void HTMLInputElementImpl::setValue(DOMString val)
{
    switch (_type) {
	case TEXT:
	case PASSWORD:
	    _value = val;
	    setChanged(true);
	    break;
	case FILE:
	    // sorry, can't change this!
	    _value = m_filename;
	default:
	    setAttribute(ATTR_VALUE,val);
    }
}


// -------------------------------------------------------------------------

HTMLLabelElementImpl::HTMLLabelElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

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

HTMLLegendElementImpl::HTMLLegendElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

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
    m_multiple = false;
    view = 0;
    // 0 means invalid (i.e. not set)
    m_size = 0;
    m_selectedIndex = -1;
}

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_multiple = false;
    view = 0;
    // 0 means invalid (i.e. not set)
    m_size = 0;
    m_selectedIndex = -1;
}

ushort HTMLSelectElementImpl::id() const
{
    return ID_SELECT;
}

DOMString HTMLSelectElementImpl::type() const
{
    return (m_multiple ? "select-multiple" : "select-one");
}

long HTMLSelectElementImpl::selectedIndex() const
{
    return m_selectedIndex;
}

void HTMLSelectElementImpl::setSelectedIndex( long  index )
{
    m_selectedIndex = index;
    setChanged(true);
}

long HTMLSelectElementImpl::length() const
{
    int len = 0;
    NodeImpl *child;
    for (child = firstChild(); child; child = child->nextSibling())
	len++;
    return len;
}

void HTMLSelectElementImpl::add( const HTMLElement &/*element*/, const HTMLElement &/*before*/ )
{
    // ###
}

void HTMLSelectElementImpl::remove( long /*index*/ )
{
    // ###
}

QString HTMLSelectElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLSelectElementImpl::blur(  )
{
    // ###
}

void HTMLSelectElementImpl::focus(  )
{
    // ###
}

NodeImpl *HTMLSelectElementImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::insertBefore(newChild,refChild);
    if (m_render)
	static_cast<RenderSelect*>(m_render)->setOptionsChanged(true);
    return result;
}

NodeImpl *HTMLSelectElementImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::replaceChild(newChild,oldChild);
    if (m_render)
	static_cast<RenderSelect*>(m_render)->setOptionsChanged(true);
    return result;
}

NodeImpl *HTMLSelectElementImpl::removeChild ( NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::removeChild(oldChild);
    if (m_render)
	static_cast<RenderSelect*>(m_render)->setOptionsChanged(true);
    return result;
}

NodeImpl *HTMLSelectElementImpl::appendChild ( NodeImpl *newChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::appendChild(newChild);
    if (m_render)
	static_cast<RenderSelect*>(m_render)->setOptionsChanged(true);
    setChanged(true);
    return result;
}

void HTMLSelectElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_SIZE:
	m_size = attr->val()->toInt();
	break;
    case ATTR_MULTIPLE:
	m_multiple = (attr->val() != 0);
	break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ### ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// ###
	break;
//    case ATTR_NAME:
	// handled by parent class...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLSelectElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
        RenderSelect *f = new RenderSelect(view, this);
	if (f)
	{
            f->setReadonly(m_readOnly);
            f->setEnabled(!m_disabled);

    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
    NodeBaseImpl::attach(_view);
}


QCString HTMLSelectElementImpl::encoding()
{
    // ### move encoding stuff for selects to here
    if (!m_render || _name.isEmpty()) return "";
    return static_cast<RenderSelect*>(m_render)->encoding();
}

// -------------------------------------------------------------------------

HTMLOptGroupElementImpl::HTMLOptGroupElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
}

HTMLOptGroupElementImpl::HTMLOptGroupElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
}

HTMLOptGroupElementImpl::~HTMLOptGroupElementImpl()
{
}

ushort HTMLOptGroupElementImpl::id() const
{
    return ID_OPTGROUP;
}

NodeImpl *HTMLOptGroupElementImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::insertBefore(newChild,refChild);
    recalcSelectOptions();
    return result;
}

NodeImpl *HTMLOptGroupElementImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::replaceChild(newChild,oldChild);
    recalcSelectOptions();
    return result;
}

NodeImpl *HTMLOptGroupElementImpl::removeChild ( NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::removeChild(oldChild);
    recalcSelectOptions();
    return result;
}

NodeImpl *HTMLOptGroupElementImpl::appendChild ( NodeImpl *newChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::appendChild(newChild);
    recalcSelectOptions();
    return result;
}

void HTMLOptGroupElementImpl::parseAttribute(AttrImpl *attr)
{
    HTMLGenericFormElementImpl::parseAttribute(attr);
    recalcSelectOptions();
}

void HTMLOptGroupElementImpl::recalcSelectOptions()
{
    NodeImpl *select = parentNode();
    while (select && select->id() != ID_SELECT)
	select = select->parentNode();
    if (select) {
	// ### also do this for other changes to optgroup and options
	select->setChanged(true);
	if (select->renderer())
	    static_cast<RenderSelect*>(select->renderer())->setOptionsChanged(true);
    }
}

// -------------------------------------------------------------------------

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_selected = false;
}

HTMLOptionElementImpl::HTMLOptionElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    m_selected = false;
}

const DOMString HTMLOptionElementImpl::nodeName() const
{
    return "OPTION";
}

ushort HTMLOptionElementImpl::id() const
{
    return ID_OPTION;
}

DOMString HTMLOptionElementImpl::text() const
{
    if(firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
	return firstChild()->nodeValue();
    }
    else
	return DOMString();
}

long HTMLOptionElementImpl::index() const
{
    // ###
    return 0;
}

void HTMLOptionElementImpl::setIndex( long  )
{
    // ###
}

void HTMLOptionElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_SELECTED:
        m_selected = true;
        break;
    case ATTR_VALUE:
        m_value = attr->value();
        break;
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

bool HTMLOptionElementImpl::selected() const
{
    return m_selected;
}

// -------------------------------------------------------------------------

HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    // DTD requires rows & cols be specified, but we will provide reasonable defaults
    m_rows = 3;
    m_cols = 60;
    m_wrap = ta_Virtual;
}


HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    // DTD requires rows & cols be specified, but we will provide reasonable defaults
    m_rows = 3;
    m_cols = 60;
    m_wrap = ta_Virtual;
}

ushort HTMLTextAreaElementImpl::id() const
{
    return ID_TEXTAREA;
}

DOMString HTMLTextAreaElementImpl::type() const
{
    // ###
    return DOMString();
}

QString HTMLTextAreaElementImpl::state( )
{
   RenderFormElement *formElement = dynamic_cast<RenderFormElement *>(m_render);
   if (formElement)
      return formElement->state();
   return QString::null;
}

void HTMLTextAreaElementImpl::blur(  )
{
    if (m_render)
	static_cast<RenderTextArea*>(m_render)->blur();
//    onBlur(); // ### enable this - but kjs needs to support re-entry
}

void HTMLTextAreaElementImpl::focus(  )
{
    // ### make sure this can't cause an infinite loop when called from onFocus/onBlur
    if (m_render)
	static_cast<RenderTextArea*>(m_render)->focus();
//    onFocus(); // ### enable this - but kjs needs to support re-entry
}

void HTMLTextAreaElementImpl::select(  )
{
    if (m_render)
	static_cast<RenderTextArea*>(m_render)->select();
//    onSelect(); // ### enable this - but kjs needs to support re-entry
}

void HTMLTextAreaElementImpl::parseAttribute(AttrImpl *attr)
{
    switch(attr->attrId)
    {
    case ATTR_ROWS:
        m_rows = attr->val() ? attr->val()->toInt() : 3;
        break;
    case ATTR_COLS:
        m_cols = attr->val() ? attr->val()->toInt() : 60;
        break;
    case ATTR_WRAP:
        if ( strcasecmp( attr->value(), "virtual" ) == 0 )
            m_wrap = ta_Virtual;
        else if ( strcasecmp ( attr->value(), "physical" ) == 0)
            m_wrap = ta_Physical;
        else
            m_wrap = ta_NoWrap;
        break;
    case ATTR_TABINDEX:
    case ATTR_ACCESSKEY:
	// ignore for the moment
	break;
    case ATTR_ONFOCUS:
    case ATTR_ONBLUR:
    case ATTR_ONSELECT:
    case ATTR_ONCHANGE:
	// no need to parse
	break;
//    case ATTR_NAME:
	// handled by parent class...
    default:
	HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLTextAreaElementImpl::attach(KHTMLView *_view)
{
    m_style = document->styleSelector()->styleForElement(this);
    view = _view;

    khtml::RenderObject *r = _parent->renderer();
    if(r)
    {
	RenderTextArea *f = new RenderTextArea(view, this);

	if (f)
	{
            f->setReadonly(m_readOnly);
	    f->setEnabled(!m_disabled);

    	    m_render = f;
	    m_render->setStyle(m_style);
	    m_render->ref();
	    r->addChild(m_render, _next ? _next->renderer() : 0);
	}
    }
    NodeBaseImpl::attach(_view);
}

QCString HTMLTextAreaElementImpl::encoding()
{
    QCString encoding;
    if (_name.isEmpty()) return encoding;
    if (!m_render) return ""; // ### make this work independent of render

    if ( _form->enctype() == "application/x-www-form-urlencoded" )
        encoding = encodeString( _name.string() ) + '=' +
		   encodeString( static_cast<RenderTextArea*>(m_render)->text() );
    else
        encoding += static_cast<RenderTextArea*>(m_render)->text().latin1();

    return encoding;
}

void HTMLTextAreaElementImpl::reset()
{
    // ### use default value as specified in HTML file
    setValue("");
}

void HTMLTextAreaElementImpl::setValue(DOMString _value)
{
    m_value = _value;
    setChanged(true);
}

void HTMLTextAreaElementImpl::onBlur()
{
    DOMString script = getAttribute(ATTR_ONBLUR);
    if (!script.isEmpty())
	view->part()->executeScript(script.string());
}

void HTMLTextAreaElementImpl::onFocus()
{
    DOMString script = getAttribute(ATTR_ONFOCUS);
    if (!script.isEmpty())
	view->part()->executeScript(script.string());
}

void HTMLTextAreaElementImpl::onSelect()
{
    DOMString script = getAttribute(ATTR_ONSELECT);
    if (!script.isEmpty())
	view->part()->executeScript(script.string());
}

void HTMLTextAreaElementImpl::onChange()
{
    DOMString script = getAttribute(ATTR_ONFOCUS);
    if (!script.isEmpty())
	view->part()->executeScript(script.string());
}



// -------------------------------------------------------------------------




