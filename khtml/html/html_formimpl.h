/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
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
#ifndef HTML_FORMIMPL_H
#define HTML_FORMIMPL_H

#include "html/html_elementimpl.h"
#include "html/html_imageimpl.h"
#include "dom/html_element.h"

#include <qvaluelist.h>
#include <qptrlist.h>
#include <qcstring.h>
#include <qmemarray.h>

#include <kdebug.h>


class KHTMLView;
class QTextCodec;

namespace khtml
{
    class RenderFormElement;
    class RenderTextArea;
    class RenderSelect;
    class RenderLineEdit;
    class RenderRadioButton;
    class RenderFileButton;

    typedef QValueList<QCString> encodingList;
}

namespace DOM {

class HTMLFormElement;
class DOMString;
class HTMLGenericFormElementImpl;
class HTMLOptionElementImpl;

// -------------------------------------------------------------------------

class HTMLFormElementImpl : public HTMLElementImpl
{
public:
    HTMLFormElementImpl(DocumentPtr *doc, bool implicit);
    virtual ~HTMLFormElementImpl();

    virtual Id id() const;

    long length() const;

    QByteArray formData(bool& ok);

    DOMString enctype() const { return m_enctype; }
    void setEnctype( const DOMString & );

    bool autoComplete() const { return m_autocomplete; }
    void doAutoFill();

    virtual void parseAttribute(AttributeImpl *attr);

    void radioClicked( HTMLGenericFormElementImpl *caller );

    void registerFormElement(HTMLGenericFormElementImpl *);
    void removeFormElement(HTMLGenericFormElementImpl *);
    void registerImgElement(HTMLImageElementImpl *);
    void removeImgElement(HTMLImageElementImpl *);

    void submitFromKeyboard();
    bool prepareSubmit();
    void submit();
    void reset();

    void setMalformed(bool malformed) { m_malformed = malformed; }
    bool isMalformed() const { return m_malformed; }

    friend class HTMLFormElement;
    friend class HTMLFormCollectionImpl;

private:
    QPtrList<HTMLGenericFormElementImpl> formElements;
    QPtrList<HTMLImageElementImpl> imgElements;
    DOMString m_target;
    DOMString m_enctype;
    QString m_boundary;
    DOMString m_acceptcharset;
    QString m_encCharset;
    bool m_post : 1;
    bool m_multipart : 1;
    bool m_autocomplete : 1;
    bool m_insubmit : 1;
    bool m_doingsubmit : 1;
    bool m_inreset : 1;
    bool m_malformed : 1;
};

// -------------------------------------------------------------------------

class HTMLGenericFormElementImpl : public HTMLElementImpl
{
    friend class HTMLFormElementImpl;
    friend class khtml::RenderFormElement;

public:
    HTMLGenericFormElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    virtual ~HTMLGenericFormElementImpl();

    HTMLFormElementImpl *form() { return m_form; }

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
    virtual void reset() {}

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();

    void onSelect();
    void onChange();

    bool disabled() const { return m_disabled; }
    void setDisabled(bool _disabled);

    virtual bool isSelectable() const;
    virtual bool isEnumeratable() const { return false; }

    bool readOnly() const { return m_readOnly; }
    void setReadOnly(bool _readOnly) { m_readOnly = _readOnly; }

    DOMString name() const;
    void setName(const DOMString& name);

    virtual bool isGenericFormElement() const { return true; }

    /*
     * override in derived classes to get the encoded name=value pair
     * for submitting
     * return true for a successful control (see HTML4-17.13.2)
     */
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool) { return false; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isEditable();

protected:
    HTMLFormElementImpl *getForm() const;

    DOMStringImpl* m_name;
    HTMLFormElementImpl *m_form;
    bool m_disabled, m_readOnly;
};

// -------------------------------------------------------------------------

class HTMLButtonElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLButtonElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);

    virtual ~HTMLButtonElementImpl();

    enum typeEnum {
        SUBMIT,
        RESET,
        BUTTON
    };

    virtual Id id() const;

    DOMString type() const;
    typeEnum buttonType() const { return m_type; }
    virtual void parseAttribute(AttributeImpl *attr);
    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);
    void activate();
    virtual void attach();
    void click();

protected:
    DOMString m_value;
    QString   m_currValue;
    typeEnum  m_type : 2;
    bool      m_dirty : 1;
    bool      m_clicked : 1;
    bool      m_activeSubmit : 1;
};

// -------------------------------------------------------------------------

class HTMLFieldSetElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLFieldSetElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);

    virtual ~HTMLFieldSetElementImpl();

    virtual Id id() const;
    virtual void attach();
    virtual void parseAttribute(AttributeImpl *attr);

};

// -------------------------------------------------------------------------

class HTMLInputElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderLineEdit;
    friend class khtml::RenderRadioButton;
    friend class khtml::RenderFileButton;

public:
    // do not change the order!
    enum typeEnum {
        TEXT,
        PASSWORD,
        ISINDEX,
        CHECKBOX,
        RADIO,
        SUBMIT,
        RESET,
        FILE,
        HIDDEN,
        IMAGE,
        BUTTON
    };

    HTMLInputElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    virtual ~HTMLInputElementImpl();

    virtual Id id() const;

    virtual bool isEnumeratable() const { return inputType() != IMAGE; }

    bool autoComplete() const { return m_autocomplete; }

    bool checked() const { return m_checked; }
    void setChecked(bool);
    long maxLength() const { return m_maxLen; }
    int size() const { return m_size; }
    DOMString type() const;
    void setType(const DOMString& t);

    DOMString value() const;
    void setValue(DOMString val);

    void blur();
    void focus();

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &);

    void select();
    void click();

    virtual void parseAttribute(AttributeImpl *attr);

    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

    typeEnum inputType() const { return m_type; }
    virtual void reset();

    // used in case input type=image was clicked.
    int clickX() const { return xPos; }
    int clickY() const { return yPos; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isEditable();

    DOMString altText() const;
    void activate();

protected:

    DOMString m_value;
    int       xPos;
    short     m_maxLen;
    short     m_size;
    short     yPos;

    typeEnum m_type : 4;
    bool m_clicked : 1 ;
    bool m_checked : 1;
    bool m_activeSubmit : 1;
    bool m_autocomplete : 1;
    bool m_inited : 1;
    bool m_unsubmittedFormChange : 1;
};

// -------------------------------------------------------------------------

class HTMLLabelElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLabelElementImpl(DocumentPtr *doc);
    virtual ~HTMLLabelElementImpl();

    virtual Id id() const;
    virtual void attach();
    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isSelectable() const { return true; }
    NodeImpl* getFormElement();

 private:
    DOMString m_formElementID;
};

// -------------------------------------------------------------------------

class HTMLLegendElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLegendElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    virtual ~HTMLLegendElementImpl();

    virtual Id id() const;
    virtual void attach();
    virtual void parseAttribute(AttributeImpl *attr);
};


// -------------------------------------------------------------------------

class HTMLSelectElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderSelect;

public:
    HTMLSelectElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    ~HTMLSelectElementImpl();

    virtual Id id() const;

    DOMString type() const;

    long selectedIndex() const;
    void setSelectedIndex( long index );

    virtual bool isEnumeratable() const { return true; }

    long length() const;

    long minWidth() const { return m_minwidth; }

    long size() const { return m_size; }

    bool multiple() const { return m_multiple; }

    void add ( const HTMLElement &element, const HTMLElement &before, int& exceptioncode );
    void remove ( long index );
    void blur();
    void focus();

    DOMString value();
    void setValue(DOMStringImpl* value);

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &);

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual NodeImpl *replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *removeChild ( NodeImpl *oldChild, int &exceptioncode );
    virtual NodeImpl *appendChild ( NodeImpl *newChild, int &exceptioncode );
    virtual NodeImpl *addChild( NodeImpl* newChild );

    virtual void childrenChanged();

    virtual void parseAttribute(AttributeImpl *attr);

    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

    // get the actual listbox index of the optionIndexth option
    int optionToListIndex(int optionIndex) const;
    // reverse of optionToListIndex - get optionIndex from listboxIndex
    int listToOptionIndex(int listIndex) const;

    void setRecalcListItems();

    QMemArray<HTMLGenericFormElementImpl*> listItems() const
     {
         if (m_recalcListItems) const_cast<HTMLSelectElementImpl*>(this)->recalcListItems();
         return m_listItems;
     }
    virtual void reset();
    void notifyOptionSelected(HTMLOptionElementImpl *selectedOption, bool selected);

private:
    void recalcListItems();

protected:
    mutable QMemArray<HTMLGenericFormElementImpl*> m_listItems;
    short m_minwidth;
    signed short m_size : 15;
    bool m_multiple : 1;
    bool m_recalcListItems;
};

// -------------------------------------------------------------------------

class HTMLKeygenElementImpl : public HTMLSelectElementImpl
{
public:
    HTMLKeygenElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);

    virtual Id id() const;

    DOMString type() const;

    long selectedIndex() const;
    void setSelectedIndex( long index );

    // ### this is just a rough guess
    virtual bool isEnumeratable() const { return false; }

    virtual void parseAttribute(AttributeImpl *attr);
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

};

// -------------------------------------------------------------------------

class HTMLOptGroupElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLOptGroupElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0)
        : HTMLGenericFormElementImpl(doc, f) {}

    virtual Id id() const;
};


// ---------------------------------------------------------------------------

class HTMLOptionElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderSelect;
    friend class DOM::HTMLSelectElementImpl;

public:
    HTMLOptionElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);

    virtual Id id() const;

    DOMString text() const;

    long index() const;
    void setIndex( long );
    virtual void parseAttribute(AttributeImpl *attr);
    DOMString value() const;
    void setValue(DOMStringImpl* value);

    bool selected() const { return m_selected; }
    void setSelected(bool _selected);

    HTMLSelectElementImpl *getSelect() const;

protected:
    DOMString m_value;
    bool m_selected;
};


// -------------------------------------------------------------------------

class HTMLTextAreaElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderTextArea;

public:
    enum WrapMethod {
        ta_NoWrap,
        ta_Virtual,
        ta_Physical
    };

    HTMLTextAreaElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    ~HTMLTextAreaElementImpl();

    virtual Id id() const;

    long cols() const { return m_cols; }

    long rows() const { return m_rows; }

    WrapMethod wrap() const { return m_wrap; }

    virtual bool isEnumeratable() const { return true; }

    DOMString type() const;

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &);

    void select (  );

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);
    virtual void reset();
    DOMString value();
    void setValue(DOMString _value);
    DOMString defaultValue();
    void setDefaultValue(DOMString _defaultValue);
    void blur();
    void focus();

    virtual bool isEditable();

protected:
    int m_rows;
    int m_cols;
    WrapMethod m_wrap;
    QString m_value;
    bool m_dirtyvalue: 1;
    bool m_unsubmittedFormChange: 1;
    bool m_initialized: 1;
};

// -------------------------------------------------------------------------

class HTMLIsIndexElementImpl : public HTMLInputElementImpl
{
public:
    HTMLIsIndexElementImpl(DocumentPtr *doc, HTMLFormElementImpl *f = 0);
    ~HTMLIsIndexElementImpl();

    virtual Id id() const;
    virtual void parseAttribute(AttributeImpl *attr);

    DOMString prompt() const;
    void setPrompt(const DOMString& _value);
};


} //namespace

#endif
