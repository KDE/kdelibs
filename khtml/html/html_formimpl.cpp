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
#include "xml/dom_textimpl.h"

#include "rendering/render_form.h"

#include <kdebug.h>
#include <kmimetype.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <netaccess.h>
#include <qfile.h>

using namespace DOM;
using namespace khtml;

//template class QList<khtml::RenderFormElement>;

HTMLFormElementImpl::HTMLFormElementImpl(DocumentImpl *doc)
    : HTMLElementImpl(doc)
{
    m_post = false;
    m_multipart = false;
    m_enctype = "application/x-www-form-urlencoded";
    m_boundary = "----------0xKhTmLbOuNdArY";
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
    return formElements.count();
}

QCString HTMLFormElementImpl::encodeByteArray(const QByteArray& e)
{
    // http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.1
    // safe characters like NS handles them for compatibility
    static const char *safe = "-._*";
    unsigned pos = 0;
    QCString encoded;

    while ( pos < e.size() )
    {
        unsigned char c = e[pos];

        if ( (( c >= 'A') && ( c <= 'Z')) ||
             (( c >= 'a') && ( c <= 'z')) ||
             (( c >= '0') && ( c <= '9')) ||
             (strchr(safe, c))
            )
            encoded += c;
        else if ( c == ' ' )
            encoded += '+';
        else if ( c == '\n' )
            encoded += "%0D%0A";
        else if ( c != '\r' )
        {
            encoded += '%';
            unsigned int h = c / 16;
            h += (h > 9) ? ('A' - 10) : '0';
            encoded += h;

            unsigned int l = c % 16;
            l += (l > 9) ? ('A' - 10) : '0';
            encoded += l;
        }
        pos++;
    }

    return encoded;
}


QByteArray HTMLFormElementImpl::formData()
{
    kdDebug( 6030 ) << "form: formData()" << endl;

    QByteArray form_data(0);
    bool first = true;
    QCString enc_string = ""; // used for non-multipart data

    HTMLGenericFormElementImpl *current = formElements.first();
    for( ; current; current = formElements.next() )
    {
        khtml::encodingList lst;

        kdDebug(6030) << "checking " << current->name().string() << endl;

        if (!current->disabled() && current->encoding(lst)) {

            kdDebug(6030) << "adding name " << current->name().string() << endl;

            khtml::encodingList::Iterator it;
            for( it = lst.begin(); it != lst.end(); ++it )
            {
                QByteArray enc(*it);
                kdDebug(6030) << "found data!" << endl;

                if (!m_multipart)
                {
                    if(!first)
                        enc_string += '&';

                    // HACK HACK HACK HACK
                    // ### encoding functions need to be able to submit more than
                    // one name part as well for this case
                    if(current->id() == ID_INPUT &&
                       static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::IMAGE)
                    {
                        HTMLInputElementImpl* i = static_cast<HTMLInputElementImpl*>(current);
                        if(i->clickX() != -1)
                        {
                            QCString aStr;
                            enc_string += HTMLFormElementImpl::encodeByteArray(QString(current->name().string() + ".x").local8Bit());
                            aStr.setNum(i->clickX());
                            enc_string += "=";
                            enc_string += aStr;
                            enc_string += "&";
                            enc_string += HTMLFormElementImpl::encodeByteArray(QString(current->name().string() + ".y").local8Bit());
                            enc_string += "=";
                            aStr.setNum(i->clickY());
                            enc_string += aStr;
                        }
                    }
                    else
                    {
                        enc_string += HTMLFormElementImpl::encodeByteArray(current->name().string().local8Bit());
                        enc_string += "=";
                        enc_string += HTMLFormElementImpl::encodeByteArray(enc);
                    }

                    first = false;
                }
                else
                {
                    QCString str(("--" + m_boundary.string() + "\r\n").ascii());
                    str += "Content-Disposition: form-data; ";
                    str += ("name=\"" + current->name().string() + "\"").ascii();

                    // if the current type is FILE, then we also need to
                    // include the filename *and* the file type

                    if (current->nodeType() == Node::ELEMENT_NODE && current->id() == ID_INPUT &&
                        static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::FILE)
                    {
                        QString path = static_cast<HTMLInputElementImpl*>(current)->filename().string();
                        QString onlyfilename = path.mid(path.findRev('/')+1);

                        str += ("; filename=\"" + onlyfilename + "\"\r\n").ascii();
                        if(!static_cast<HTMLInputElementImpl*>(current)->filename().isEmpty())
                        {
                            str += "Content-Type: ";
                            KMimeType::Ptr ptr = KMimeType::findByURL(KURL(path));
                            str += ptr->name().ascii();
                        }
                    }

                    str += "\r\n\r\n";

                    // this is where it gets ugly.. we have to memcpy the
                    // text part to the form.. then memcpy the (possibly
                    // binary) data.  yuck!
                    unsigned int old_size = form_data.size();
                    form_data.resize( old_size + str.size() + enc.size());
                    memcpy(form_data.data() + old_size, str.data(), str.length());
                    memcpy(form_data.data() + old_size + str.length(), enc, enc.size());
                    form_data[form_data.size()-2] = '\r';
                    form_data[form_data.size()-1] = '\n';
                }
            }
        }
    }
    if (m_multipart)
        enc_string = ("--" + m_boundary.string() + "--\r\n").ascii();

    int old_size = form_data.size();
    form_data.resize( form_data.size() + enc_string.length() );
    memcpy(form_data.data() + old_size, enc_string.data(), enc_string.length() );

    return form_data;
}

void HTMLFormElementImpl::setEnctype( const DOMString& type )
{
    m_enctype = type;
    if ( strcasecmp( type, "multipart/form-data" ) == 0 )
        m_multipart = true;
}

void HTMLFormElementImpl::setBoundary( const DOMString& bound )
{
    m_boundary = bound;
}

void HTMLFormElementImpl::submit(  )
{
    kdDebug( 6030 ) << "submit pressed!" << endl;
    if(!view) return;

    DOMString script = getAttribute(ATTR_ONSUBMIT);
    if (!script.isNull() && view->part()->jScriptEnabled())
        if(!view->part()->executeScript(Node(this), script.string()))
            return; // don't submit if script returns false

    QByteArray form_data = formData();

    // formdata is not null-terminated, so this will cause Insure++ to scream
    // kdDebug( 6030 ) << "formdata = " << form_data.data() << endl << "m_post = " << m_post << endl << "multipart = " << m_multipart << endl;

    if(m_post)
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
    if (!script.isNull() && view->part()->jScriptEnabled())
        if(!view->part()->executeScript(Node(this), script.string()))
            return;

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
            m_post = true;
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
                (static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::TEXT ||
                 static_cast<HTMLInputElementImpl*>(current)->inputType() == HTMLInputElementImpl::PASSWORD))
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

void HTMLGenericFormElementImpl::onBlur()
{
    DOMString script = getAttribute(ATTR_ONBLUR);
    if (!script.isEmpty() && view->part()->jScriptEnabled())
        view->part()->executeScript(Node(this), script.string());
}

void HTMLGenericFormElementImpl::onFocus()
{
    DOMString script = getAttribute(ATTR_ONFOCUS);
    if (!script.isEmpty() && view->part()->jScriptEnabled())
        view->part()->executeScript(Node(this), script.string());
}

void HTMLGenericFormElementImpl::onSelect()
{
    DOMString script = getAttribute(ATTR_ONSELECT);
    if (!script.isEmpty() && view->part()->jScriptEnabled())
        view->part()->executeScript(Node(this), script.string());
}

void HTMLGenericFormElementImpl::onChange()
{
    DOMString script = getAttribute(ATTR_ONCHANGE);
    if (!script.isEmpty() && view->part()->jScriptEnabled())
        view->part()->executeScript(Node(this), script.string());
}


void HTMLGenericFormElementImpl::blur()
{
    static_cast<RenderFormElement*>(m_render)->blur();
//    onBlur(); // ### enable this - but kjs needs to support re-entry
}

void HTMLGenericFormElementImpl::focus()
{
    static_cast<RenderFormElement*>(m_render)->focus();
//    onFocus(); // ### enable this - but kjs needs to support re-entry
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
    return getAttribute(ATTR_TYPE);
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
    m_value = "";
    _maxLen = -1;
    _size = 20;
    _clicked = false;
    m_filename = "";
    m_haveType = false;

    view = 0;
}

HTMLInputElementImpl::HTMLInputElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    _type = TEXT;
    m_checked = false;
    m_value = "";
    _maxLen = -1;
    _size = 20;
    _clicked = false;
    m_filename = "";
    m_haveType = false;

    view = 0;
}

HTMLInputElementImpl::~HTMLInputElementImpl()
{
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
    // needs to be lowercase according to DOM spec
    switch (_type) {
    case TEXT: return "text";
    case PASSWORD: return "password";
    case CHECKBOX: return "checkbox";
    case RADIO: return "radio";
    case SUBMIT: return "submit";
    case RESET: return "reset";
    case FILE: return "file";
    case HIDDEN: return "hidden";
    case IMAGE: return "image";
    case BUTTON: return "button";
    default: return "";
    }
}

QString HTMLInputElementImpl::state( )
{
    switch (_type) {
    case TEXT:
    case PASSWORD:
        return m_value.string()+'.'; // Make sure the string is not empty!
    case CHECKBOX:
        return QString::fromLatin1(m_checked ? "on" : "off");
    case RADIO:
        return QString::fromLatin1(m_checked ? "on" : "off");
    case FILE:
        return m_filename.string()+'.';
    default:
        return QString::null;
    }
}



void HTMLInputElementImpl::restoreState(const QString &state)
{
    switch (_type) {
    case TEXT:
    case PASSWORD:
        m_value = DOMString(state.left(state.length()-1));
        break;
    case CHECKBOX:
    case RADIO:
        m_checked = state == QString::fromLatin1("on");
        break;
    case FILE:
        m_value = m_filename = DOMString(state.left(state.length()-1));
        break;
    default:
        break;
    }
    setChanged(true);
}


void HTMLInputElementImpl::blur(  )
{
    if (_type != IMAGE)
        HTMLGenericFormElementImpl::blur();
}

void HTMLInputElementImpl::focus(  )
{
    if (_type != IMAGE)
        HTMLGenericFormElementImpl::focus();
}

void HTMLInputElementImpl::select(  )
{
    if (_type == TEXT || _type == PASSWORD)
        static_cast<RenderLineEdit*>(m_render)->select();
    else if (_type == FILE)
        static_cast<RenderFileButton*>(m_render)->select();
}

void HTMLInputElementImpl::click(  )
{
    // ###
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
    case ATTR_TYPE: {
            typeEnum newType;

            if ( strcasecmp( attr->value(), "text" ) == 0 )
                newType = TEXT;
            else if ( strcasecmp( attr->value(), "password" ) == 0 )
                newType = PASSWORD;
            else if ( strcasecmp( attr->value(), "checkbox" ) == 0 )
                newType = CHECKBOX;
            else if ( strcasecmp( attr->value(), "radio" ) == 0 )
                newType = RADIO;
            else if ( strcasecmp( attr->value(), "submit" ) == 0 )
                newType = SUBMIT;
            else if ( strcasecmp( attr->value(), "reset" ) == 0 )
                newType = RESET;
            else if ( strcasecmp( attr->value(), "file" ) == 0 )
                newType = FILE;
            else if ( strcasecmp( attr->value(), "hidden" ) == 0 )
                newType = HIDDEN;
            else if ( strcasecmp( attr->value(), "image" ) == 0 )
                newType = IMAGE;
            else if ( strcasecmp( attr->value(), "button" ) == 0 )
                newType = BUTTON;
            else
                newType = TEXT;

            if (!m_haveType) {
                _type = newType;
                m_haveType = true;
            }
            else if (_type != newType) {
                setAttribute(ATTR_TYPE,type());
            }
        }
        break;
    case ATTR_VALUE:
        m_value = attr->value();
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

    khtml::RenderObject *r = _parent ? _parent->renderer() : 0;
    if(r)
    {
        switch(_type)
        {
        case TEXT:
            m_render = new RenderLineEdit(view, this);
            break;
        case PASSWORD:
            m_render = new RenderLineEdit(view, this);
            break;
        case CHECKBOX:
            m_render = new RenderCheckBox(view, this);
            break;
        case RADIO:
            m_render = new RenderRadioButton(view, this);
            break;
        case SUBMIT:
            m_render = new RenderSubmitButton(view, this);
            break;
        case IMAGE:
        {
            m_render = new RenderImageButton(this);
            setHasEvents();
            break;
        }
        case RESET:
            m_render = new RenderResetButton(view, this);
            break;
        case FILE:
            m_render = new RenderFileButton(view, this);
            break;
        case HIDDEN:
            m_render = 0;
            break;
        case BUTTON:
            m_render = new RenderPushButton(view, this);
            break;
        }

        if (m_render)
        {
            m_render->setStyle(m_style);
            kdDebug( 6030 ) << "adding " << m_render->renderName() << " as child of " << r->renderName() << endl;
            QString state = document->registerElement(this);
            if ( !state.isEmpty())
            {
                kdDebug( 6030 ) << "Restoring InputElem name=" << _name.string() <<
                    " state=" << state << endl;
                restoreState( state );
            }

            r->addChild(m_render, _next ? _next->renderer() : 0);
        }
    }
    NodeBaseImpl::attach(_view);

    if (m_render && _type == IMAGE) {
        static_cast<RenderImageButton*>
            (m_render)->setImageUrl(_src,
                                    static_cast<HTMLDocumentImpl *>(document)->URL(),
                                    static_cast<HTMLDocumentImpl *>(document)->docLoader());

    }
}


bool HTMLInputElementImpl::encoding(khtml::encodingList& encoding)
{
    if (_name.isEmpty()) return false;

    // ### local8Bit() is probably wrong here
    switch (_type) {
        case HIDDEN:
        case TEXT:
        case PASSWORD:
            // always successful
            encoding += m_value.string().local8Bit();
            return true;
        case CHECKBOX:

            if( checked() )
            {
                encoding += ( m_value.isEmpty() ? QCString("on") : m_value.string().local8Bit() );
                return true;
            }
            break;

        case RADIO:

            if( checked() )
            {
                encoding += m_value.string().local8Bit();
                return true;
            }
            break;

        case BUTTON:
        case RESET:
            // those buttons are never successful
            return false;

        case IMAGE:

            if(_clicked)
            {
                _clicked = false;
                encoding += m_value.string().local8Bit();
                return true; // coordinate submit are currently a special case in formData
            }
            break;

        case SUBMIT:

            if (m_render && static_cast<RenderSubmitButton*>(m_render)->clicked())
            {
                QCString enc_str = m_value.isNull() ?
                                   static_cast<RenderSubmitButton*>(m_render)->defaultLabel().local8Bit() : value().string().local8Bit();

                if (m_render)
                    static_cast<RenderSubmitButton*>(m_render)->setClicked(false);

                if(!enc_str.isEmpty())
                {
                    encoding += enc_str;
                    return true;
                }
            }
            break;

        case FILE:
        {
            QString local;

            // if no filename at all is entered, return successful, however empty
            if(m_filename.isEmpty())
                return true;

            if ( KIO::NetAccess::download(KURL(m_filename.string()), local) )
            {
                QFile file(local);
                if (file.open(IO_ReadOnly))
                {
                    QByteArray filearray(file.size()+1);
                    file.readBlock( filearray.data(), file.size());
                    // not really necessary, but makes it easier
                    filearray[filearray.size()-1] = '\0';
                    file.close();

                    encoding += filearray;
                    KIO::NetAccess::removeTempFile( local );

                    return true;
                }
                else
                {
                    KMessageBox::error(0L, i18n("Cannot open downloaded file.\nSubmit a bugreport"));
                    KIO::NetAccess::removeTempFile( local );
                    return false;
                }
            }
            else {
                KMessageBox::sorry(0L, i18n("Error downloading file:\n%1").arg(KIO::NetAccess::lastErrorString()));
                return false;
            }
            break;
        }
    }
    return false;
}

void HTMLInputElementImpl::reset()
{
    setValue(getAttribute(ATTR_VALUE));
    setChecked(getAttribute(ATTR_CHECKED) != 0);
    if ((_type == SUBMIT || _type == RESET || _type == BUTTON || _type == IMAGE) && m_render)
        static_cast<RenderSubmitButton*>(m_render)->setClicked(false);
}

void HTMLInputElementImpl::setChecked(bool _checked)
{
    qDebug("setchecked %d", _checked);

    m_checked = _checked;
    if (_type == RADIO && _form && m_checked)
        _form->radioClicked(this);
    setChanged(true);
}

void HTMLInputElementImpl::setValue(DOMString val)
{
    switch (_type) {
    case TEXT:
    case PASSWORD:
        m_value = (val.isNull() ? DOMString("") : val);
        setChanged(true);
        break;
    case FILE:
        // sorry, can't change this!
        m_value = m_filename;
    default:
        setAttribute(ATTR_VALUE,val);
    }
}

bool HTMLInputElementImpl::mouseEvent( int _x, int _y, int button, MouseEventType type,
                                       int _tx, int _ty, DOMString &url,
                                       NodeImpl *&innerNode, long &offset )
{
    bool wasPressed = pressed();
    bool ret = HTMLGenericFormElementImpl::mouseEvent(_x,_y,button,type,_tx,_ty,url,innerNode,offset);
    if (_type == IMAGE && (type == MouseClick || ((type == MouseRelease) && wasPressed))) {
        xPos = _x - _tx - m_render->xPos();
        yPos = _y - _ty - m_render->yPos();
        _clicked = true;
        _form->submit();
        return true;
    }
    return ret;
}

void HTMLInputElementImpl::setOwnerDocument(DocumentImpl *_document)
{
    if (ownerDocument())
	ownerDocument()->removeElement(this);
    HTMLGenericFormElementImpl::setOwnerDocument(_document);
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
}

HTMLSelectElementImpl::HTMLSelectElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    m_multiple = false;
    view = 0;
    // 0 means invalid (i.e. not set)
    m_size = 0;
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
    uint i;
    for (i = 0; i < m_listItems.size(); i++) {
        if (m_listItems[i]->id() == ID_OPTION
            && static_cast<HTMLOptionElementImpl*>(m_listItems[i])->selected())
            return listToOptionIndex(int(i)); // selectedIndex is the *first* selected item; there may be others
    }
    return -1;
}

void HTMLSelectElementImpl::setSelectedIndex( long  index )
{
    // deselect all other options and select only the new one
    int listIndex;
    for (listIndex = 0; listIndex < int(m_listItems.size()); listIndex++) {
        if (m_listItems[listIndex]->id() == ID_OPTION)
            static_cast<HTMLOptionElementImpl*>(m_listItems[listIndex])->setSelected(false);
    }
    listIndex = optionToListIndex(index);
    if (listIndex >= 0)
        static_cast<HTMLOptionElementImpl*>(m_listItems[listIndex])->setSelected(true);

    setChanged(true);
}

long HTMLSelectElementImpl::length() const
{
    int len = 0;
    uint i;
    for (i = 0; i < m_listItems.size(); i++) {
        if (m_listItems[i]->id() == ID_OPTION)
            len++;
    }
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
    return m_render ? static_cast<RenderSelect*>(m_render)->state() : QString::null;
}

void HTMLSelectElementImpl::restoreState(const QString &state)
{
    if (m_render)
        static_cast<RenderSelect*>(m_render)->restoreState(state);
    setChanged(true);
}

NodeImpl *HTMLSelectElementImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::insertBefore(newChild,refChild);
    recalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::replaceChild(newChild,oldChild);
    recalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::removeChild ( NodeImpl *oldChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::removeChild(oldChild);
    recalcListItems();
    return result;
}

NodeImpl *HTMLSelectElementImpl::appendChild ( NodeImpl *newChild )
{
    NodeImpl *result = HTMLGenericFormElementImpl::appendChild(newChild);
    recalcListItems();
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
            m_render = f;
            m_render->setStyle(m_style);
            r->addChild(m_render, _next ? _next->renderer() : 0);
        }
    }
    NodeBaseImpl::attach(_view);
}


bool HTMLSelectElementImpl::encoding(khtml::encodingList& encoded_values)
{
    bool successful = false;

    uint i;
    for (i = 0; i < m_listItems.size(); i++) {
        if (m_listItems[i]->id() == ID_OPTION) {
            HTMLOptionElementImpl *option = static_cast<HTMLOptionElementImpl*>(m_listItems[i]);
            if (option->selected()) {
                if (option->value().isNull() || option->value() == "")
                    encoded_values += option->text().string().stripWhiteSpace().local8Bit();
                else
                    encoded_values += option->value().string().local8Bit();
                successful = true;
            }
        }
    }

    // ### this case should not happen. make sure that we select the first option
    // in any case. otherwise we have no consistency with the DOM interface. FIXME!
    // we return the first one if it was a combobox select
    if (!successful && !m_multiple && m_size <= 1 && (m_listItems[0]->id() == ID_OPTION) ) {
        HTMLOptionElementImpl *option = static_cast<HTMLOptionElementImpl*>(m_listItems[0]);
        if (option->value().isNull() || option->value() == "")
            encoded_values += option->text().string().stripWhiteSpace().local8Bit();
        else
            encoded_values += option->value().string().local8Bit();
        successful = true;
    }

    return successful;
}

int HTMLSelectElementImpl::optionToListIndex(int optionIndex) const
{
    if (optionIndex < 0 || optionIndex >= int(m_listItems.size()))
        return -1;

    int listIndex = 0;
    int optionIndex2 = 0;
    for (;
         optionIndex2 < int(m_listItems.size()) && optionIndex2 <= optionIndex;
         listIndex++) { // not a typo!
        if (m_listItems[listIndex]->id() == ID_OPTION)
            optionIndex2++;
    }
    listIndex--;
    return listIndex;
}

int HTMLSelectElementImpl::listToOptionIndex(int listIndex) const
{
    if (listIndex < 0 || listIndex >= int(m_listItems.size()) ||
        m_listItems[listIndex]->id() != ID_OPTION)
        return -1;

    int optionIndex = 0; // actual index of option not counting OPTGROUP entries that may be in list
    int i;
    for (i = 0; i < listIndex; i++)
        if (m_listItems[i]->id() == ID_OPTION)
            optionIndex++;
    return optionIndex;
}

void HTMLSelectElementImpl::recalcListItems()
{
    NodeImpl* current = firstChild();
    bool inOptGroup = false;
    m_listItems.resize(0);
    bool foundSelected = false;
    while(current) {
        if (!inOptGroup && current->id() == ID_OPTGROUP && current->firstChild()) {
            // ### what if optgroup contains just comments? don't want one of no options in it...
            m_listItems.resize(m_listItems.size()+1);
            m_listItems[m_listItems.size()-1] = static_cast<HTMLGenericFormElementImpl*>(current);
            current = current->firstChild();
            inOptGroup = true;
        }
        if (current->id() == ID_OPTION) {
            m_listItems.resize(m_listItems.size()+1);
            m_listItems[m_listItems.size()-1] = static_cast<HTMLGenericFormElementImpl*>(current);
            if (foundSelected && !m_multiple && static_cast<HTMLOptionElementImpl*>(current)->selected())
                static_cast<HTMLOptionElementImpl*>(current)->setSelected(false);
            foundSelected = static_cast<HTMLOptionElementImpl*>(current)->selected();
        }
        NodeImpl *parent = current->parentNode();
        current = current->nextSibling();
        if (!current) {
            if (inOptGroup) {
                current = parent->nextSibling();
                inOptGroup = false;
            }
        }
    }
    setChanged(true);
}

void HTMLSelectElementImpl::reset()
{
    uint i;
    for (i = 0; i < m_listItems.size(); i++) {
        if (m_listItems[i]->id() == ID_OPTION) {
            HTMLOptionElementImpl *option = static_cast<HTMLOptionElementImpl*>(m_listItems[i]);
            bool selected = (!option->getAttribute(ATTR_SELECTED).isNull());
            option->setSelected(selected);
            if (!m_multiple && selected)
                return;
        }
    }
}

void HTMLSelectElementImpl::notifyOptionSelected(HTMLOptionElementImpl *selectedOption, bool selected)
{
    if (selected && !m_multiple) {
        // deselect all other options
        uint i;
        for (i = 0; i < m_listItems.size(); i++) {
            if (m_listItems[i]->id() == ID_OPTION && m_listItems[i] != selectedOption)
                static_cast<HTMLOptionElementImpl*>(m_listItems[i])->setSelected(false);
        }
    }
    if (m_render)
        static_cast<RenderSelect*>(m_render)->updateSelection();
    setChanged(true);


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
    if (select)
        static_cast<HTMLSelectElementImpl*>(select)->recalcListItems();
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

DOMString HTMLOptionElementImpl::text()
{
    DOMString label = getAttribute(ATTR_LABEL);
    if (label.isEmpty() && firstChild() && firstChild()->nodeType() == Node::TEXT_NODE) {
        // ### allow for comments and multiple textnodes in our children
        return firstChild()->nodeValue();
    }
    else
        return label;
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
        m_selected = (attr->val() != 0);
        break;
    case ATTR_VALUE:
        m_value = attr->value();
        break;
    default:
        HTMLGenericFormElementImpl::parseAttribute(attr);
    }
}

void HTMLOptionElementImpl::setSelected(bool _selected)
{
    m_selected = _selected;
    HTMLSelectElementImpl *select = getSelect();
    if (select)
        select->notifyOptionSelected(this,_selected);
}

HTMLSelectElementImpl *HTMLOptionElementImpl::getSelect()
{
    NodeImpl *select = parentNode();
    while (select && select->id() != ID_SELECT)
        select = select->parentNode();
    return static_cast<HTMLSelectElementImpl*>(select);
}


// -------------------------------------------------------------------------

HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc)
    : HTMLGenericFormElementImpl(doc)
{
    // DTD requires rows & cols be specified, but we will provide reasonable defaults
    m_rows = 3;
    m_cols = 60;
    m_wrap = ta_Virtual;
    m_value = 0;
}


HTMLTextAreaElementImpl::HTMLTextAreaElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f)
    : HTMLGenericFormElementImpl(doc, f)
{
    // DTD requires rows & cols be specified, but we will provide reasonable defaults
    m_rows = 3;
    m_cols = 60;
    m_wrap = ta_Virtual;
    m_value = 0;
}

ushort HTMLTextAreaElementImpl::id() const
{
    return ID_TEXTAREA;
}

DOMString HTMLTextAreaElementImpl::type() const
{
    return "textarea";
}

QString HTMLTextAreaElementImpl::state( )
{
    // Make sure the string is not empty!
    return value().string()+'.';
}

void HTMLTextAreaElementImpl::restoreState(const QString &state)
{
    m_value = DOMString(state.left(state.length()-1));
    setChanged(true);
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
        // virtual / physical is Netscape extension of HTML 3.0, now deprecated
        // soft/ hard / off is recommendation for HTML 4 extension by IE and NS 4
        if ( strcasecmp( attr->value(), "virtual" ) == 0  || strcasecmp( attr->value(), "soft") == 0)
            m_wrap = ta_Virtual;
        else if ( strcasecmp ( attr->value(), "physical" ) == 0 || strcasecmp( attr->value(), "hard") == 0)
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
            m_render = f;
            m_render->setStyle(m_style);
            r->addChild(m_render, _next ? _next->renderer() : 0);

            QString state = document->registerElement(this);
            if (!state.isEmpty())
                restoreState( state );
        }
    }
    NodeBaseImpl::attach(_view);
}

bool HTMLTextAreaElementImpl::encoding(khtml::encodingList& encoding)
{
    if (_name.isEmpty() || !m_render) return false;

    // ### make this work independent of render
    encoding += value().string().local8Bit();

    return true;
}

void HTMLTextAreaElementImpl::reset()
{
    setValue(defaultValue());
}

DOMString HTMLTextAreaElementImpl::value()
{
    if (m_value.isNull())
        m_value = defaultValue();
    return m_value;
}

void HTMLTextAreaElementImpl::setValue(DOMString _value)
{
    m_value = _value;
    setChanged(true);
}


DOMString HTMLTextAreaElementImpl::defaultValue()
{
    DOMString val = "";
    // there may be comments - just grab the text nodes
    NodeImpl *n;
    for (n = firstChild(); n; n = n->nextSibling())
        if (n->isTextNode())
            val += static_cast<TextImpl*>(n)->data();
    return val;
}

void HTMLTextAreaElementImpl::setDefaultValue(DOMString _defaultValue)
{
    // there may be comments - remove all the text nodes and replace them with one
    QList<NodeImpl> toRemove;
    NodeImpl *n;
    for (n = firstChild(); n; n = n->nextSibling())
        if (n->isTextNode())
            toRemove.append(n);
    QListIterator<NodeImpl> it(toRemove);
    for (; it.current(); ++it) {
        removeChild(it.current());
    }
    insertBefore(document->createTextNode(_defaultValue),firstChild());
    setValue(_defaultValue);
}

// -------------------------------------------------------------------------




