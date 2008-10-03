/*
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 *           (C) 2004, 2005, 2006 Apple Computer, Inc.
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
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */
#ifndef HTML_FORMIMPL_H
#define HTML_FORMIMPL_H

#include "html/html_elementimpl.h"
#include "html/html_imageimpl.h"
#include "dom/html_element.h"

class QTextCodec;

namespace khtml
{
    class RenderFormElement;
    class RenderTextArea;
    class RenderSelect;
    class RenderLineEdit;
    class RenderRadioButton;
    class RenderFileButton;

    typedef QList<QByteArray> encodingList;
}

namespace KWallet {
    class Wallet;
}

namespace DOM {

class HTMLFormElement;
class DOMString;
class HTMLGenericFormElementImpl;
class HTMLOptionElementImpl;
class HTMLCollectionImpl;

// -------------------------------------------------------------------------

class HTMLFormElementImpl : public HTMLElementImpl
{
public:
    HTMLFormElementImpl(DocumentImpl *doc, bool implicit);
    virtual ~HTMLFormElementImpl();

    virtual Id id() const;

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();
    virtual void addId(const QString& id);
    virtual void removeId(const QString& id);


    long length() const;

    QByteArray formData(bool& ok);

    DOMString enctype() const { return m_enctype; }
    void setEnctype( const DOMString & );

    DOMString target() const;
    DOMString action() const;
    HTMLCollectionImpl* elements();

    bool autoComplete() const { return m_autocomplete; }
    void doAutoFill();
    void walletOpened(KWallet::Wallet *w);

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
    void gatherWalletData();
    QList<HTMLGenericFormElementImpl*> formElements;
    QList<HTMLImageElementImpl*> imgElements;
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
    bool m_haveTextarea : 1; // for wallet storage
    bool m_havePassword : 1; // for wallet storage
    DOMString m_name;        // our name
    QMap<QString, QString> m_walletMap; // for wallet storage
};

// -------------------------------------------------------------------------

class HTMLGenericFormElementImpl : public HTMLElementImpl
{
    friend class HTMLFormElementImpl;
    friend class khtml::RenderFormElement;

public:
    HTMLGenericFormElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    virtual ~HTMLGenericFormElementImpl();

    HTMLFormElementImpl *form() const { return m_form; }

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
    virtual void reset() {}

    virtual void insertedIntoDocument();
    virtual void removedFromDocument();

    void onSelect();
    void onChange();

    bool disabled() const { return m_disabled; }
    void setDisabled(bool _disabled);

    virtual bool isFocusable() const;
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
    HTMLButtonElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);

    virtual ~HTMLButtonElementImpl();

    enum typeEnum {
        SUBMIT,
        RESET,
        BUTTON
    };

    virtual Id id() const;
    virtual bool isEnumeratable() const { return true; }

    DOMString type() const;
    typeEnum buttonType() const { return KDE_CAST_BF_ENUM(typeEnum, m_type); }
    virtual void parseAttribute(AttributeImpl *attr);
    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);
    void activate();
    virtual void attach();
    void click();

protected:
    DOMString             m_value;
    QString               m_currValue;
    KDE_BF_ENUM(typeEnum) m_type : 2;
    bool                  m_dirty : 1;
    bool                  m_clicked : 1;
    bool                  m_activeSubmit : 1;
};

// -------------------------------------------------------------------------

class HTMLFieldSetElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLFieldSetElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);

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

    HTMLInputElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    virtual ~HTMLInputElementImpl();

    virtual Id id() const;

    virtual bool isEnumeratable() const { return inputType() != IMAGE; }

    bool autoComplete() const { return m_autocomplete; }

    bool checked() const { return m_useDefaultChecked ? m_defaultChecked : m_checked; }
    void setChecked(bool);
    bool indeterminate() const { return m_indeterminate; }
    void setIndeterminate(bool);
    long maxLength() const { return m_maxLen; }
    int size() const { return m_size; }
    DOMString type() const;
    void setType(const DOMString& t);

    DOMString value() const;
    void setValue(DOMString val);

    DOMString valueWithDefault() const;

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &state);

    void select();
    void click();

    virtual void parseAttribute(AttributeImpl *attr);

    virtual void copyNonAttributeProperties(const ElementImpl* source);

    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);

    typeEnum inputType() const { return KDE_CAST_BF_ENUM(typeEnum, m_type); }
    virtual void reset();

    // used in case input type=image was clicked.
    int clickX() const { return xPos; }
    int clickY() const { return yPos; }

    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isEditable();

    DOMString altText() const;
    void activate();

    void setUnsubmittedFormChange(bool unsubmitted) { m_unsubmittedFormChange = unsubmitted; }

    //Mozilla extensions.
    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd  (long pos);
    void setSelectionRange(long start, long end);
protected:
    void parseType(const DOMString& t);

    DOMString m_value;
    int       xPos;
    short     m_maxLen;
    short     m_size;
    short     yPos;

    KDE_BF_ENUM(typeEnum) m_type : 4;
    bool m_clicked : 1 ;
    bool m_checked : 1;
    bool m_defaultChecked : 1; // could do without by checking ATTR_CHECKED
    bool m_useDefaultChecked : 1;
    bool m_indeterminate : 1;
    bool m_haveType : 1;
    bool m_activeSubmit : 1;
    bool m_autocomplete : 1;
    bool m_inited : 1;
    bool m_unsubmittedFormChange : 1;
};

// -------------------------------------------------------------------------

class HTMLLabelElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLabelElementImpl(DocumentImpl *doc);
    virtual ~HTMLLabelElementImpl();

    virtual Id id() const;
    virtual void attach();
    virtual void defaultEventHandler(EventImpl *evt);
    virtual bool isFocusable() const { return true; }
    virtual bool isTabFocusable() const { return false; }
    NodeImpl* getFormElement();

 private:
    DOMString m_formElementID;
};

// -------------------------------------------------------------------------

class HTMLLegendElementImpl : public HTMLGenericFormElementImpl
{
public:
    HTMLLegendElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
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
    HTMLSelectElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    ~HTMLSelectElementImpl();

    virtual Id id() const;

    DOMString type() const;

    HTMLCollectionImpl* options();

    long selectedIndex() const;
    void setSelectedIndex( long index );

    virtual bool isEnumeratable() const { return true; }

    long length() const;

    long minWidth() const { return m_minwidth; }

    long size() const { return m_size; }

    bool multiple() const { return m_multiple; }

    void add ( HTMLElementImpl* element, HTMLElementImpl* before, int& exceptioncode );
    void remove ( long index );

    DOMString value() const;
    void setValue(DOMStringImpl* value);

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &state);

    virtual NodeImpl *insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode );
    virtual void      replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode );
    virtual void      removeChild ( NodeImpl *oldChild, int &exceptioncode );
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

    QVector<HTMLGenericFormElementImpl*> listItems() const
     {
         if (m_recalcListItems) const_cast<HTMLSelectElementImpl*>(this)->recalcListItems();
         return m_listItems;
     }
    virtual void reset();
    void notifyOptionSelected(HTMLOptionElementImpl *selectedOption, bool selected);

private:
    void recalcListItems() const;

protected:
    mutable QVector<HTMLGenericFormElementImpl*> m_listItems;
    short m_minwidth;
    signed short m_size : 15;
    bool m_multiple : 1;
    mutable bool m_recalcListItems :1;
    mutable unsigned int   m_length:31;
};

// -------------------------------------------------------------------------

class HTMLKeygenElementImpl : public HTMLSelectElementImpl
{
public:
    HTMLKeygenElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);

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
    HTMLOptGroupElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0)
		: HTMLGenericFormElementImpl(doc, f) {}

    virtual Id id() const;
};


// ---------------------------------------------------------------------------

class HTMLOptionElementImpl : public HTMLGenericFormElementImpl
{
    friend class khtml::RenderSelect;
    friend class DOM::HTMLSelectElementImpl;

public:
    HTMLOptionElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);

    virtual Id id() const;

    DOMString text() const;

    long index() const;
    void setIndex( long );
    virtual void parseAttribute(AttributeImpl *attr);
    DOMString value() const;
    void setValue(DOMStringImpl* value);

    bool selected() const { return m_selected; }
    void setSelected(bool _selected);
    void setDefaultSelected( bool _defaultSelected );

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

    HTMLTextAreaElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    ~HTMLTextAreaElementImpl();

    virtual Id id() const;
    virtual void childrenChanged();

    long cols() const { return m_cols; }

    long rows() const { return m_rows; }

    WrapMethod wrap() const { return m_wrap; }

    virtual bool isEnumeratable() const { return true; }

    DOMString type() const;

    virtual bool maintainsState() { return true; }
    virtual QString state();
    virtual void restoreState(const QString &state);

    void select (  );

    virtual void parseAttribute(AttributeImpl *attr);
    virtual void attach();
    virtual bool encoding(const QTextCodec*, khtml::encodingList&, bool);
    virtual void reset();
    DOMString value();
    void setValue(DOMString _value);
    DOMString defaultValue();
    void setDefaultValue(DOMString _defaultValue);

    virtual bool isEditable();
    void setUnsubmittedFormChange(bool unsubmitted) { m_unsubmittedFormChange = unsubmitted; }

    //Mozilla extensions.
    long selectionStart();
    long selectionEnd();
    void setSelectionStart(long pos);
    void setSelectionEnd  (long pos);
    void setSelectionRange(long start, long end);
    long textLength();
protected:
    int m_rows;
    int m_cols;
    WrapMethod m_wrap;
    QString m_value;
    bool m_changed: 1;    //States whether the contents has been editted
    bool m_unsubmittedFormChange: 1;
    bool m_initialized: 1;
};

// -------------------------------------------------------------------------

class HTMLIsIndexElementImpl : public HTMLInputElementImpl
{
public:
    HTMLIsIndexElementImpl(DocumentImpl *doc, HTMLFormElementImpl *f = 0);
    ~HTMLIsIndexElementImpl();

    virtual Id id() const;
    virtual void parseAttribute(AttributeImpl *attr);

    DOMString prompt() const;
    void setPrompt(const DOMString& _value);
};


} //namespace

#endif
